/*
 * PocketGeiger.h
 *
 *  Created on: Sep 4, 2017
 *	  Author: pete
 */

#ifndef GEIGER_POCKETGEIGER_H_
#define GEIGER_POCKETGEIGER_H_

#include <cstddef>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include <fastgpioomega2.h>

#ifndef HISTORY_LENGTH
  #define HISTORY_LENGTH 200
#endif
// Duration of each history array cell (seconds).
#define HISTORY_UNIT 6
#define PROCESS_PERIOD 160

#define OMEGA2_INPUT 	0
#define OMEGA2_OUTPUT 	1
#define SIG_GPIO_IRQ	42

class PocketGeiger {
public:

	void setup(int, int);
	void loop();
	/* Integration time of traced radiation count (in milliseconds),
	 * grows gradually to HISTORY_LENGTH * HISTORY_UNIT * 1000. */
	unsigned long integrationTime();
	/* Return the current radiation count, that is the number of Gamma ray
	 * since the last call to loop(), which reset the current count to 0. */
	int currentRadiationCount();
	/* Return the radiation count, that is the number of Gamma ray occured
	 * during the integration time. */
	unsigned long radiationCount();
	// Return the number of radiation count by minute.
	double cpm();
	// Return the radiation dose, exprimed in Sievert (uSv/h).
	double uSvh();
	/* Return the error of the measurement (uSv/h).
	 * The range of precision of the measurement is:
	 * [ uSvh-uSvhError, uSvh+uSvhError ]. */
	double uSvhError();
	// Dose coefficient (cpm = uSv x alpha)
	static constexpr double kAlpha = 53.032;
	/* Register a function that will be called when a radiation pulse
	 * is detected. */
	void registerRadiationCallback(void (*callback)(void));
	/* Register a function that will be called when a noise pulse
	 * is detected. */
	void registerNoiseCallback(void (*callback)(void));
	// radiation count used in interrupt routine
	static int volatile _radiationCount;
	// noise count used in interrupt routine
	static int volatile _noiseCount;
	void (*_radiationCallback)(void);
	void (*_noiseCallback)(void);

	static PocketGeiger& instance() {
		static PocketGeiger m_instance;
		return m_instance;
	}

private:
	PocketGeiger();
        PocketGeiger(PocketGeiger const&);              // Don't Implement.
        void operator=(PocketGeiger const&); // Don't implement

	virtual ~PocketGeiger();
	unsigned long millis();

	// History of count rates.
	unsigned int _countHistory[HISTORY_LENGTH];
	unsigned long previousTime;
	unsigned long previousHistoryTime;
	// Current count (sum of count in _countHistory).
	unsigned long _count;
	// Position of current count rate on _countHistory[].
	int historyIndex;
	// Current length of count history
	int historyLength;
	// Pin settings.
	int _signPin;
	int _noisePin;
	unsigned long m_millis;
	FastGpioOmega2 m_omega2;

	// User callbacks.
	// function to attach the interrupt handler
	bool setupInterrupt();
	static void _onRadiationHandler();
	static void _onNoiseHandler();
};

#endif /* GEIGER_POCKETGEIGER_H_ */
