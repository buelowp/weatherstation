/*
 * PocketGeiger.cpp
 *
 *  Created on: Sep 4, 2017
 *      Author: pete
 */

#include "PocketGeiger.h"

PocketGeiger::PocketGeiger(int signal, int noise)
{
    m_previousTime = 0;
    m_previousHistoryTime = 0;
    m_count = 0;
    m_historyIndex = 0;
    m_historyLength = 0;
    m_signalPin = signal;
    m_noisePin = noise;
    m_millis = millis();

    m_signal = new GPIOPin(m_signalPin);
    m_signalHandler = new Geiger_Pin_Handler_Object(GPIO_IRQ_RISING);
    m_signal->setDirection(GPIO_INPUT);
    m_signal->setIrq(GPIO_IRQ_RISING, m_signalHandler, 0);

    m_noise = new GPIOPin(m_noisePin);
    m_noiseHandler = new Geiger_Pin_Handler_Object(GPIO_IRQ_RISING);
    m_noise->setDirection(GPIO_INPUT);
    m_noise->setIrq(GPIO_IRQ_RISING, m_noiseHandler, 0);

	for(int i = 0; i < HISTORY_LENGTH; i++)
		m_countHistory[i] = 0;

	m_count = 0;
	m_historyIndex = 0;
	m_historyLength = 0;
	// Init measurement time.
	m_previousTime = millis();
	m_previousHistoryTime = millis();
}

PocketGeiger::~PocketGeiger()
{
}

/**
 * \func unsigned long PocketGeiger::millis()
 * \return Returns an unsigned long pertaining to the total number of elapsed millis since we started
 * \details This is a rough replacement for arduino millis(). We set an initial millis value when the class
 * is instantiated, and then we simply calculate the delta between start and now and return it.
 */
unsigned long PocketGeiger::millis()
{
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    unsigned long milliseconds = te.tv_sec*1000UL + te.tv_usec/1000; // caculate milliseconds
    // printf("milliseconds: %lld\n", milliseconds);
    return milliseconds - m_millis;
}

/**
 * \func void PocketGeiger::loop()
 * \details This is the calculation loop for determining our radiation data. It should be called
 * about everr second or so. Note that there is a 2-3 minute warmup for this device, so expect it
 * to take a while to get data accurately.
 */
void PocketGeiger::loop()
{
	// Process radiation dose if the process period has elapsed.
	unsigned long currentTime = millis();

	if (currentTime - m_previousTime >= PROCESS_PERIOD) {
		int currentCount = m_signalHandler->count();
		int currentNoiseCount = m_noiseHandler->count();

		m_signalHandler->reset();
		m_noiseHandler->reset();

		if (currentNoiseCount == 0) {
			// Store count log.
			m_countHistory[m_historyIndex] += currentCount;
			// Add number of counts.
			m_count += currentCount;
		}

		// Shift an array for counting log for each 6 seconds.
		if (currentTime - m_previousHistoryTime >= HISTORY_UNIT * 1000) {
			m_previousHistoryTime += (unsigned long)(HISTORY_UNIT * 1000);
			m_historyIndex = (m_historyIndex + 1) % HISTORY_LENGTH;
			if (m_historyLength < (HISTORY_LENGTH - 1)) {
				// Since, we overwrite the oldest value in the history,
				// the effective maximum length is HISTORY_LENGTH-1
				m_historyLength++;
			}
			m_count -= m_countHistory[m_historyIndex];
			m_countHistory[m_historyIndex] = 0;
		}

		// Save time of current process period
		m_previousTime = currentTime;
	}
}

unsigned long PocketGeiger::integrationTime()
{
	return (m_historyLength * HISTORY_UNIT * 1000UL + m_previousTime - m_previousHistoryTime);
}

int PocketGeiger::currentRadiationCount()
{
	return m_signalHandler->count();
}

unsigned long PocketGeiger::radiationCount()
{
	return m_count;
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

