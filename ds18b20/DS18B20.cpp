/*
 * DS18B20.cpp
 *
 *  Created on: Sep 4, 2017
 *      Author: pete
 */

#include "DS18B20.h"

DS18B20::DS18B20()
{
	m_oneWireFullPath(ONE_WIRE_LOCATION);
}

DS18B20::~DS18B20()
{
}

int DS18B20::findDevices()
{
	DIR *dirp = opendir(ONE_WIRE_LOCATION);
	struct dirent *dp;

	while ((dp = readdir(dirp)) != NULL) {
		std::string file(dp->d_name);
		if (file.compare(".") == 0 || file.compare("..") == 0)
			continue;

		if (file.compare("w1_bus_master1") == 0) {
			m_busMasterPath = m_oneWireFullPath + file;
			std::cout << __PRETTY_FUNCTION__ << ": found master at " << m_busMasterPath;
			continue;
		}

		m_devices.insert(std::make_pair(file, m_oneWireFullPath + file));
		std::cout << __PRETTY_FUNCTION__ << ": added " << file;
	}

	return m_devices.size();
}

float DS18B20::tempCByName(std::string &device)
{
	auto path = m_devices.find(device);

	if (!path == m_devices.end()) {
		if (read(path->first, path->second)) {
			return (m_lastResult[path->first] / 1000);
		}
	}

	std::cerr << __PRETTY_FUNCTION__ << ": Unable to find " << device;
	return 0;
}

float DS18B20::tempFByName(std::string &device)
{
	auto path = m_devices.find(device);

	if (!path == m_devices.end()) {
		if (read(path->first, path->second)) {
			return ((m_lastResult[path->first] / 1000) * 9 / 5 + 32);
		}
	}

	std::cerr << __PRETTY_FUNCTION__ << ": Unable to find " << device;
	return 0;
}

float DS18B20::averageTempFForAllDevices()
{
	return ((averageTempForAllDevices() / 1000) * 9 / 5 + 32);
}

float DS18B20::averageTempFForAllDevices()
{

	return (averageTempForAllDevices() / 1000);
}

float DS18B20::averageTempForAllDevices()
{
	auto it = m_devices.begin();
	float average = 0.0;
	int count = 0;

	while (it != m_devices.end()) {
		if (read(it->first, it->second)) {
			average += m_lastResult[it->first];
			count++;
		}
	}

	if (count != 0)
		average = average / count;

	return average;
}

bool DS18B20::read(std::string &device, std::string &path)
{
	return true;
}
