/*
 * PocketGeiger.cpp
 *
 *  Created on: Sep 4, 2017
 *      Author: pete
 */

#include "PocketGeiger.h"

int volatile PocketGeiger::_radiationCount = 0;
int volatile PocketGeiger::_noiseCount = 0;

/*
void PocketGeiger::_onRadiationHandler()
{
	PocketGeiger::instance()._radiationCount++;
}

void PocketGeiger::_onNoiseHandler()
{
	PocketGeiger::instance()._noiseCount++;
}
*/

extern "C" {
	void irq_handler(int n, siginfo_t *info, void *unused)
	{
		printf("%s: Received value 0x%X\n", __PRETTY_FUNCTION__, info->si_int);
	}
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
	m_millis = millis();
}

PocketGeiger::~PocketGeiger()
{
}

unsigned long PocketGeiger::millis()
{
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    unsigned long milliseconds = te.tv_sec*1000UL + te.tv_usec/1000; // caculate milliseconds
    // printf("milliseconds: %lld\n", milliseconds);
    return milliseconds - m_millis;
}

void PocketGeiger::setup(int sig, int noise)
{
	_signPin = sig;
	_noisePin = noise;
	/*
	 * The Omega2 does not have internal resistors
	 * These two pins MUST have 4.7k PullUps externally
	 * to work with this chip.
	 *
	 * Preferred pins for Omega2 would be
	 * GPIO 18
	 * GPIO 19
	 * GPIO 4
	 * GPIO 5
	 */
	m_omega2.SetDirection(_signPin, OMEGA2_INPUT);
	m_omega2.SetDirection(_noisePin, OMEGA2_INPUT);

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

bool PocketGeiger::setupInterrupt()
{
	int fd;
	char buf[100];

	struct sigaction sig;
	sig.sa_sigaction = irq_handler;
	sig.sa_flags = SA_SIGINFO | SA_NODEFER;
	sigaction(SIG_GPIO_IRQ, &sig, NULL);

	fd=open("/sys/kernel/debug/gpio-irq", O_WRONLY);
	if(fd < 0)
	{
		perror("open");
		return false;
	}

	sprintf(buf, "+ %d %i", _signPin, getpid());

	if(write(fd, buf, strlen(buf) + 1) < 0)
	{
		perror("write");
		close(fd);
		return false;
	}

	memset(buf, '\0', 100);
	sprintf(buf, "+ %d %i", _noisePin, getpid());

	if(write(fd, buf, strlen(buf) + 1) < 0)
	{
		perror("write");
		close(fd);
		return false;
	}

	close(fd);
	return true;
}
