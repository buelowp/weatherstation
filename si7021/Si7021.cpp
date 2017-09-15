/*
 * Si7021.cpp
 *
 *  Created on: Sep 14, 2017
 *      Author: buelowp
 */

#include "Si7021.h"

Si7021::Si7021(int device, int address)
{
	m_deviceAddress = address;
	m_deviceNumber = device;
}

Si7021::~Si7021()
{
}

unsigned int Si7021::getHumidityBasisPoints()
{
	uint8_t packet[2];
	unsigned int rval;

	i2c_writeBytes(m_deviceNumber, m_deviceAddress, SI7021_RH_READ, 1, 1);
	i2c_read(m_deviceNumber, m_deviceAddress, SI7021_RH_READ, packet, 2);

	unsigned int humraw = (unsigned int)packet[0] << 8 | packet[1];
	rval = ((125 * humraw) >> 16) - 6;
	return rval;
}

unsigned int Si7021::getHumidityPercent()
{
	uint8_t packet[2];
	unsigned int rval;

	i2c_writeBytes(m_deviceNumber, m_deviceAddress, SI7021_RH_READ, 1, 1);
	i2c_read(m_deviceNumber, m_deviceAddress, SI7021_RH_READ, packet, 2);

	unsigned int humraw = (unsigned int)packet[0] << 8 | packet[1];
	rval = ((12500 * humraw) >> 16) - 600;
	return rval;
}

int Si7021::getDeviceId()
{
	uint8_t packet[8];

	i2c_writeBytes(m_deviceNumber, m_deviceAddress, SI7021_DEV_ID_BYTE_1, 1, 1);
	i2c_writeBytes(m_deviceNumber, m_deviceAddress, SI7021_DEV_ID_BYTE_2, 1, 1);
	i2c_read(m_deviceNumber, m_deviceAddress, SI7021_DEV_ID_BYTE_1, packet, 8);

	return 0;
}
