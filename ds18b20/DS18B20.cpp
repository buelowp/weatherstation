/*
 * DS18B20.cpp
 *
 *  Created on: Sep 4, 2017
 *      Author: pete
 */

#include "DS18B20.h"

DS18B20::DS18B20()
{
	m_oneWireFullPath = ONE_WIRE_LOCATION;
}

DS18B20::~DS18B20()
{
}

int DS18B20::findDevices()
{
	DIR *dirp;
	struct dirent *dp;

	if ((dirp = opendir(ONE_WIRE_LOCATION)) == NULL) {
		std::cerr << __PRETTY_FUNCTION__ << ": unable to open " << ONE_WIRE_LOCATION << std::endl;
		std::cerr << __PRETTY_FUNCTION__ << ": " << strerror(errno) << std::endl;
	}
	while ((dp = readdir(dirp)) != NULL) {
		std::string file(dp->d_name);
		if (file.compare(".") == 0 || file.compare("..") == 0)
			continue;

		if (file.compare("w1_bus_master1") == 0) {
			m_busMasterPath = m_oneWireFullPath + file;
			std::cout << __PRETTY_FUNCTION__ << ": found master at " << m_busMasterPath << std::endl;
			continue;
		}

		m_devices.insert(std::make_pair(file, m_oneWireFullPath + file));
		std::cout << __PRETTY_FUNCTION__ << ": added " << file << std::endl;
	}

	return m_devices.size();
}

float DS18B20::tempCByName(std::string &device)
{
	auto path = m_devices.find(device);

	if (path != m_devices.end()) {
		if (read(path->first, path->second)) {
			return m_lastResult[path->first];
		}
	}

	std::cerr << __PRETTY_FUNCTION__ << ": Unable to find " << device << std::endl;
	return 0;
}

float DS18B20::tempFByName(std::string &device)
{
	auto path = m_devices.find(device);

	if (path != m_devices.end()) {
		if (read(path->first, path->second)) {
			return (m_lastResult[path->first] * 9 / 5 + 32);
		}
	}

	std::cerr << __PRETTY_FUNCTION__ << ": Unable to find " << device << std::endl;
	return 0;
}

float DS18B20::averageTempFForAllDevices()
{
	std::cout << __PRETTY_FUNCTION__ << std::endl;
	return (averageTempForAllDevices() * 9 / 5 + 32);
}

float DS18B20::averageTempCForAllDevices()
{
	std::cout << __PRETTY_FUNCTION__ << std::endl;
	return averageTempForAllDevices();
}

float DS18B20::averageTempForAllDevices()
{
	float average = 0.0;
	int count = 0;

	std::cout << __PRETTY_FUNCTION__ << ": Looping " << m_devices.size() << " times" << std::endl;
	for (auto it: m_devices) {
		std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << std::endl;
		if (read(it.first, it.second)) {
			average += m_lastResult[it.first];
			std::cout << __PRETTY_FUNCTION__ << ": total = " << average << std::endl;
			count++;
		}
	}

	if (count != 0) {
		average = average / count;
		std::cout << __PRETTY_FUNCTION__ << ": calculated average = " << average << std::endl;
	}

	return average;
}

int DS18B20::deviceCount()
{
	return m_devices.size();
}

bool DS18B20::read(std::string device, std::string path)
{
	std::string w1 = path + "/w1_slave";
	std::ifstream file(w1.c_str());
	bool crcOk = false;
	float rawValue = 0;
	size_t pos;

	std::cout << __PRETTY_FUNCTION__ << ": opened path " << w1 << std::endl;
	while (!file.eof()) {
		char buff[128];
		file.getline(buff, 128);
		std::string line(buff);
		if (line.find("CRC=") != std::string::npos) {
			if (line.find("YES") != std::string::npos) {
				std::cout << __PRETTY_FUNCTION__ << ": CRC is good" << std::endl;
				crcOk = true;
			}
			continue;
		}
		if ((pos = line.find("t=")) != std::string::npos) {
			size_t chars = (line.size() - (pos + 2));
			std::cout << __PRETTY_FUNCTION__ << ": extracting " << chars << " characters" << std::endl;
			std::cout << __PRETTY_FUNCTION__ << ": line size " << line.size() << std::endl;
			std::cout << __PRETTY_FUNCTION__ << ": pos " << (pos + 2) << std::endl;
			std::cout << __PRETTY_FUNCTION__ << ": line: " << line << std::endl;
			rawValue = std::stof(line.substr(pos + 2, chars));
			continue;
		}
	}

	m_lastResult[device] = (float)(rawValue / 1000);
	std::cout << __PRETTY_FUNCTION__ << ": Got C temp of " << m_lastResult[device] << " for device " << device << std::endl;
	return true;
}
