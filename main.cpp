/*
 * main.cpp
 *
 *  Created on: Sep 4, 2017
 *      Author: pete
 */

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <wiringPi.h>
#include "ds18b20/DS18B20.h"
#include "geiger/PocketGeiger.h"
#include "tsl2561/tsl2561.h"

void *tsl;
DS18B20 ds18b20_1;
DS18B20 ds18b20_2;

void onRadiation()
{
	std::cout << "Wild Gamma Rays are a happenin: " << PocketGeiger::instance().uSvh();
	std::cout << "uSv/h +/- " << PocketGeiger::instance().uSvhError();
}

void onNoise()
{
	std::cout << "too much noise";
}

PI_THREAD (luxThread)
{
	int c;
	long lux;

	if (tsl != NULL) {
		while (1) {
			lux = tsl2561_lux(tsl);
			std::cout << "lux: " << lux;
			usleep(3 * 1000 * 1000);
		}
	}
}

PI_THREAD (tempThread)
{
	ds18b20_1.open("sdlk");
	ds18b20_2.open("lksd");

	while (1) {
		ds18b20_1.read();
		ds18b20_2.read();
		std::cout << __PRETTY_FUNCTION__ << ": temp=" << ds18b20_1.tempF();
		std::cout << __PRETTY_FUNCTION__ << ": temp=" << ds18b20_2.tempF();
		usleep(1000 * 1000);
	}
}

void runtime()
{

	piThreadCreate(luxThread);
	piThreadCreate(tempThread);

	while (1) {
		sleep(1000);
	}
}

int main(int argc, char *argv[])
{
	pid_t pid;

	if ((tsl = tsl2561_init(0x39, "/dev/i2c-1")) == NULL) {
		std::cerr << "Unable to open TSL device";
	}

	PocketGeiger::instance().registerRadiationCallback(&onRadiation);
	PocketGeiger::instance().registerNoiseCallback(&onRadiation);

	tsl2561_enable_autogain(tsl);
	tsl2561_set_integration_time(tsl, TSL2561_INTEGRATION_TIME_13MS);

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
	exit(0);
}
