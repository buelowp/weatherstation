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

#include <GPIOPin.h>
#include <GPIOAccess.h>
#include "GeigerPinHandler.h"

#ifndef HISTORY_LENGTH
  #define HISTORY_LENGTH 200
#endif
// Duration of each history array cell (seconds).
#define HISTORY_UNIT 6
#define PROCESS_PERIOD 160

/**
 * \class PocketGeiger
 *
 * \details Provides access to a 2 pin interrupt driven
 * geiger counter. It gives access to variations on the
 * data to provide different ways of looking at what the
 * sensor is telling us
 */
class PocketGeiger {
public:
	PocketGeiger(int, int);
	virtual ~PocketGeiger();


	void loop();
	unsigned long integrationTime();
	int currentRadiationCount();
	unsigned long radiationCount();
	double cpm();
	double uSvh();
	double uSvhError();

private:
	static constexpr double kAlpha = 53.032;

	unsigned long millis();

	unsigned int m_countHistory[HISTORY_LENGTH];
	unsigned long m_previousTime;
	unsigned long m_previousHistoryTime;
	unsigned long m_count;
	unsigned long m_millis;
	int m_historyIndex;
	int m_historyLength;
	int m_signalPin;
	int m_noisePin;

	Geiger_Pin_Handler_Object *m_signalHandler;
	Geiger_Pin_Handler_Object *m_noiseHandler;
	GPIOPin *m_signal;
	GPIOPin *m_noise;
};

#endif /* GEIGER_POCKETGEIGER_H_ */
