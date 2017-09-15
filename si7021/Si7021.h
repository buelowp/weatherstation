/*
 * Si7021.h
 *
 *  Created on: Sep 14, 2017
 *      Author: buelowp
 */

#ifndef SI7021_SI7021_H_
#define SI7021_SI7021_H_

#include <onion-i2c.h>

#define SI7021_I2C_ADDR				0x40

// I2C commands
#define SI7021_RH_READ				0xF5
#define SI7021_TEMP_READ			0xF3
#define SI7021_POST_RH_TEMP_READ	0xF0
#define SI7021_RESET				0xFE
#define SI7021_USER1_READ			0xF7
#define SI7021_USER1_WRITE			0xF6

#define SI7021_DEV_ID_BYTE_1		0xFA
#define SI7021_DEV_ID_BYTE_2		0x0F

class Si7021 {
public:
	Si7021(int device = 0, int address = SI7021_I2C_ADDR);
	virtual ~Si7021();

	unsigned int getHumidityPercent();
	unsigned int getHumidityBasisPoints();
	void setPrecision(int);
	int getDeviceId();

private:
	int m_deviceAddress;
	int m_deviceNumber;
};

#endif /* SI7021_SI7021_H_ */
