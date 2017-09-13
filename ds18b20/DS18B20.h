/*
 * DS18B20.h
 *
 *  Created on: Sep 4, 2017
 *      Author: pete
 */

#ifndef DS18B20_DS18B20_H_
#define DS18B20_DS18B20_H_

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <map>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

#define ONE_WIRE_LOCATION	"/sys/bus/w1/devices/"

class DS18B20 {
public:
	DS18B20();
	virtual ~DS18B20();

	int findDevices();
	int deviceCount();
	float tempCByName(std::string&);
	float tempFByName(std::string&);
	float averageTempFForAllDevices();
	float averageTempCForAllDevices();

private:
	bool read(std::string, std::string);
	float averageTempForAllDevices();

	std::map<std::string, std::string> m_devices;
	std::map<std::string, float> m_lastResult;
	std::string m_busMasterPath;
	std::string m_oneWireFullPath;
	int m_deviceCount;
};

#endif /* DS18B20_DS18B20_H_ */
