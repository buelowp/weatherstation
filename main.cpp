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
#include <vector>
#include <string>
#include <sstream>

#include "geiger/PocketGeiger.h"

PocketGeiger *g_pg;

void usage(const char *name)
{
    std::cerr << "usage: " << name << " -l <pin:pin> -d" << std::endl;
    std::cerr << "\t-l <pin:pin> Enable Pocket Geiger counter with signal pin:noise pin" << std::endl;
    std::cerr << "\tNote, pins must be numeric and colon seperated to work" << std::endl;
    std::cerr << "\t-d Daemonize the application to run in the background" << std::endl;
    exit(-1);
}

#if 0
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
		if (t != 0) {
			RGBLed rgb;
			rgb.green();
		}
		std::cout << __PRETTY_FUNCTION__ << ": temp = " << t << std::endl;
		usleep(1000 * 1000 * 60);
	}
}

void environThread()
{
	CCS811 ccs811(7);

}
#endif

void runtime()
{
    while (1) {
        if(g_pg)
            g_pg->loop();

        sleep(1);
    }
}

int split_args(const char *name, const char *input, std::vector<int> &result)
{
    std::istringstream f(input);
    std::string s;    
    while (getline(f, s, ':')) {
        try {
        	result.push_back(std::stoi(s));
        }
        catch (std::exception &e) {
        	std::cerr << e.what();
        	usage(name);
        }
    }
    return result.size();
}

int main(int argc, char *argv[])
{
	pid_t pid;
    std::vector<int> args;
    g_pg = nullptr;
    bool daemonize = false;
    int c;

    while ((c = getopt (argc, argv, "dl:")) != -1) {
        switch (c) {
        case 'l':
            if (split_args(argv[0], optarg, args) == 2)
                g_pg = new PocketGeiger(args.at(0), args.at(1));     
            else
                usage(argv[0]);

            break;
        case 'd':
            daemonize = true;
            break;
        default:
            usage(argv[0]);
        }
    }

    if (daemonize) {
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
    }
    else {
        runtime();
    }

	exit(0);
}
