/*
 * CCS811.cpp
 *
 *  Created on: Sep 13, 2017
 *      Author: buelowp
 */

#include "CCS811.h"

CCS811::CCS811(int pin, int addr)
{
	m_omega2.SetDirection(pin, OMEGA2_OUTPUT);
	m_gpio = pin;
	m_tvoc = 0;
	m_co2 = 0;
	m_deviceNumber = 0;
	m_deviceAddress = addr;
}

CCS811::~CCS811()
{
}

void CCS811::beginBurnIn()
{
	setMeasurementMode(CCS811_MEAS_MODE_1);
	sensorSleep(20 * 60, 0);
}

void CCS811::setMeasurementMode(int mode)
{
	i2c_writeBytes(m_deviceNumber, m_deviceAddress, CCS811_MEAS_MODE, 1, 1);
}

void CCS811::prepare()
{
	int hwid = 0;

//	m_omega2.Set(m_gpio, 0);
//	sensorSleep(0, 70);
	i2c_writeBytes(m_deviceNumber, m_deviceAddress, CCS811_HW_ID, 1, 1);
	i2c_readByte(m_deviceNumber, m_deviceAddress, CCS811_HW_ID, &hwid);
//	m_omega2.Set(m_gpio, 1);
}

void CCS811::compensate(float t, float r)
{
	int temp, rh;
	uint8_t envdata[4];

//	m_omega2.Set(m_gpio, 0);
//	sensorSleep(0, 50);

	if (t > 0) {
		temp = (int)t + 0.5;
	}
	else if (t < 0) {
		temp = (int)t - 0.5;
	}
	temp = temp + 25;
	rh = (int)r + 0.5;

	envdata[0] = rh << 1;
	envdata[1] = 0;
	envdata[2] = temp << 1;
	envdata[3] = 0;

	i2c_writeBuffer(m_deviceNumber, m_deviceAddress, CCS811_ENV_DATA, envdata, 4);
//	m_omega2.Set(m_gpio, 1);
}

void CCS811::getData(int *voc, int *co2)
{
	uint8_t buffer[8];

	//CCS811::compensate(t, rh);
//	m_omega2.Set(m_gpio, 0);
//	sensorSleep(0, 50);
	i2c_writeBytes(m_deviceNumber, m_deviceAddress, CCS811_ALG_RESULT_DATA, 0, 1);
	i2c_read(m_deviceNumber, m_deviceAddress, CCS811_ALG_RESULT_DATA, buffer, 8);
	m_co2 = ((uint16_t)buffer[0] << 8) | buffer[1];
	m_tvoc = ((uint16_t)buffer[2] << 8) | buffer[3];
//	m_omega2.Set(m_gpio, 1);

	*voc = m_tvoc;
	*co2 = m_co2;
}

uint8_t CCS811::readErrorID(uint8_t status)
{
	int error_id = 0;

//	m_omega2.Set(m_gpio, 0);
//	sensorSleep(0, 50);
	i2c_writeBytes(m_deviceNumber, m_deviceAddress, CCS811_ERROR_ID, 1, 1);
	i2c_readByte(m_deviceNumber, m_deviceAddress, CCS811_ERROR_ID, &error_id);
//	m_omega2.Set(m_gpio, 1);

	uint8_t bit = (status & (1 << 1 - 1)) != 0;

	if (bit == 1) {
		std::cerr << __PRETTY_FUNCTION__ << ":" << __LINE__ << ": Error ID: " << error_id;
	}
	return (uint8_t)error_id;
}

bool CCS811::isDeviceReady()
{
	uint8_t bitmask = readStatus();
	std::bitset<8> status(bitmask);

	if (status.test(CCS811_READY) && status.test(CCS811_APP_VALID)) {
		if (status.test(CCS811_ERROR)) {
			std::cerr << __PRETTY_FUNCTION__ << ": error bit set" << std::endl;
			return false;
		}
		if (status.test(CCS811_DATA_READY))
			return true;
	}

	if (!status.test(CCS811_READY))
		std::cerr << __PRETTY_FUNCTION__ << ": In boot mode" << std::endl;

	if (!status.test(CCS811_APP_VALID))
		std::cerr << __PRETTY_FUNCTION__ << ": Invalid app code started" << std::endl;

	return false;
}

uint8_t CCS811::readStatus()
{
	int status;

//	m_omega2.Set(m_gpio, 0);
//	sensorSleep(0, 50);
	i2c_writeBytes(m_deviceNumber, m_deviceAddress, CCS811_STATUS, 0, 1);

	i2c_readByte(m_deviceNumber, m_deviceAddress, CCS811_STATUS, &status);
//	m_omega2.Set(m_gpio, 1);
	return (uint8_t)status;
}

uint8_t CCS811::readHW_ID()
{
	int hwid = 0;

//	m_omega2.Set(m_gpio, 0);
//	sensorSleep(0, 70);
	i2c_writeBytes(m_deviceNumber, m_deviceAddress, CCS811_HW_ID, 1, 1);
	i2c_readByte(m_deviceNumber, m_deviceAddress, CCS811_HW_ID, &hwid);
//	m_omega2.Set(m_gpio, 1);
	return (uint8_t)hwid;
}

void CCS811::sensorSleep(int sec, int msec)
{
	struct timespec ts;

	ts.tv_sec = sec;
	ts.tv_nsec = msec * 1000;
	nanosleep(&ts, NULL);
}

