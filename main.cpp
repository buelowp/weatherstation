/*
 * main.cpp
 *
 *  Created on: Sep 4, 2017
 *      Author: pete
 */

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <thread>
#include <unistd.h>
#include "ds18b20/DS18B20.h"
#include "geiger/PocketGeiger.h"
#include "tsl2561/TSL2561.h"

void onRadiation()
{
	std::cout << "Wild Gamma Rays are a happening: " << PocketGeiger::instance().uSvh();
	std::cout << "uSv/h +/- " << PocketGeiger::instance().uSvhError();
}

void onNoise()
{
	std::cout << "too much noise";
}

void luxThread()
{
	TSL2561 tsl2561(0, TSL2561_INTEGRATION_TIME_13MS);
	int c;
	unsigned long lux;

	tsl2561.enableAutoGain();

	while (1) {
		lux = tsl2561.lux();
		usleep(1000 * 1000 * 60);
	}
}

void tempThread()
{
	DS18B20 probes;

	std::cout << __PRETTY_FUNCTION__ << ": Staring temperature monitor thread" << std::endl;
	if (probes.findDevices() == 0) {
		std::cerr << __PRETTY_FUNCTION__ << ": No DS18B20 devices found" << std::endl;
		return;
	}
	std::cout << __PRETTY_FUNCTION__ << ": Found " << probes.deviceCount() << " devices" << std::endl;

	while (1) {
		float t = probes.averageTempFForAllDevices();
		std::cout << __PRETTY_FUNCTION__ << ": temp = " << t << std::endl;
		usleep(1000 * 1000 * 60);
	}
}

void runtime()
{
//	std::thread lt(&luxThread);
	std::thread tt(&tempThread);

	while (1) {
		sleep(1000);
	}
}

int main(int argc, char *argv[])
{
	pid_t pid;

	runtime();
//	PocketGeiger::instance().registerRadiationCallback(&onRadiation);
//	PocketGeiger::instance().registerNoiseCallback(&onRadiation);
/*
	pid = fork();
	if (pid == 0) {
		runtime();
	}
	else if (pid < 0) {
		std::cerr << "fork failed";
	}
	else {
		fclose(stdout);
		fclose(stdin);
		fclose(stderr);
	}
	*/
	exit(0);
}
