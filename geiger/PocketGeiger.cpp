/*
 * PocketGeiger.cpp
 *
 *  Created on: Sep 4, 2017
 *      Author: pete
 */

#include "PocketGeiger.h"

int volatile PocketGeiger::_radiationCount = 0;
int volatile PocketGeiger::_noiseCount = 0;

void PocketGeiger::_onRadiationHandler()
{
	PocketGeiger::instance()._radiationCount++;
}

void PocketGeiger::_onNoiseHandler()
{
	PocketGeiger::instance()._noiseCount++;
}

PocketGeiger::PocketGeiger()
{
	previousTime = 0;
	previousHistoryTime = 0;
	_count = 0;
	historyIndex = 0;
	historyLength = 0;
	_signPin = 0;
	_noisePin = 0;
	_noiseCallback = NULL;
	_radiationCallback = NULL;

	wiringPiSetupGpio();
}

PocketGeiger::~PocketGeiger()
{
}

void PocketGeiger::setup(int sig, int noise)
{
	_signPin = sig;
	_noisePin = noise;
	pinMode(_signPin, INPUT);
	pinMode(_noisePin, INPUT);
	pullUpDnControl(_signPin, PUD_UP);
	pullUpDnControl(_noisePin, PUD_UP);

	// Initialize _countHistory[].
	for(int i = 0; i < HISTORY_LENGTH; i++)
		_countHistory[i] = 0;
	_count = 0;
	historyIndex = 0;
	historyLength = 0;
	// Init measurement time.
	previousTime = millis();
	previousHistoryTime = millis();
	// Attach interrupt handlers.
	setupInterrupt();
}

void PocketGeiger::loop()
{
	// Process radiation dose if the process period has elapsed.
	unsigned long currentTime = millis();
	if (currentTime - previousTime >= PROCESS_PERIOD) {
		int currentCount = _radiationCount;
		int currentNoiseCount = _noiseCount;
		_radiationCount = 0;
		_noiseCount = 0;

		if (currentNoiseCount == 0) {
			// Store count log.
			_countHistory[historyIndex] += currentCount;
			// Add number of counts.
			_count += currentCount;
		}

		// Shift an array for counting log for each 6 seconds.
		if (currentTime - previousHistoryTime >= HISTORY_UNIT * 1000) {
			previousHistoryTime += (unsigned long)(HISTORY_UNIT * 1000);
			historyIndex = (historyIndex + 1) % HISTORY_LENGTH;
			if (historyLength < (HISTORY_LENGTH-1)) {
				// Since, we overwrite the oldest value in the history,
				// the effective maximum length is HISTORY_LENGTH-1
				historyLength++;
			}
			_count -= _countHistory[historyIndex];
			_countHistory[historyIndex] = 0;
		}

		// Save time of current process period
		previousTime = currentTime;
		// Enable the callbacks.
		if (_noiseCallback && currentNoiseCount > 0) {
			_noiseCallback();
		}

		if (_radiationCallback && currentCount > 0) {
			_radiationCallback();
		}
	}
}

void PocketGeiger::registerRadiationCallback(void (*callback)(void))
{
	_radiationCallback = callback;
}

void PocketGeiger::registerNoiseCallback(void (*callback)(void))
{
	_noiseCallback = callback;
}

unsigned long PocketGeiger::integrationTime()
{
	return (historyLength * HISTORY_UNIT * 1000UL + previousTime - previousHistoryTime);
}

int PocketGeiger::currentRadiationCount()
{
	int currentCount = _radiationCount;
	return currentCount;
}

unsigned long PocketGeiger::radiationCount()
{
	return _count;
}

double PocketGeiger::cpm()
{
	// cpm = uSv x alpha
	double min = integrationTime() / 60000.0;
	return (min > 0) ? radiationCount() / min : 0;
}

double PocketGeiger::uSvh()
{
	return cpm() / kAlpha;
}

double PocketGeiger::uSvhError()
{
	double min = integrationTime() / 60000.0;
	return (min > 0) ? std::sqrt(radiationCount()) / min / kAlpha : 0;
}

void PocketGeiger::setupInterrupt()
{
	wiringPiISR(_signPin, INT_EDGE_FALLING, _onRadiationHandler);
	wiringPiISR(_noisePin, INT_EDGE_FALLING, _onNoiseHandler);
}
