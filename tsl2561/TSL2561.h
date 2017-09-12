/*
 * TSL2561.h
 *
 *  Created on: Sep 12, 2017
 *      Author: buelowp
 */

#ifndef TSL2561_TSL2561_H_
#define TSL2561_TSL2561_H_

#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <onion-i2c.h>

#define TSL2561_I2C_ADDR_LOW 0x29
#define TSL2561_I2C_ADDR_DEFAULT 0x39
#define TSL2561_I2C_ADDR_HIGH 0x49

#define TSL2561_INTEGRATION_TIME_13MS 0x00
#define TSL2561_INTEGRATION_TIME_101MS 0x01
#define TSL2561_INTEGRATION_TIME_402MS 0x02

#define TSL2561_GAIN_0X 0x00
#define TSL2561_GAIN_16X 0x10

#define TSL2561_REG_CTRL 0x00
#define TSL2561_REG_TIMING 0x01

#define TSL2561_REG_CH0_LOW 0x0C
#define TSL2561_REG_CH0_HIGH 0x0D
#define TSL2561_REG_CH1_LOW 0x0E
#define TSL2561_REG_CH1_HIGH 0x0F

#define TSL2561_CMD_BIT (0x80)
#define TSL2561_WORD_BIT (0x20)

#define TSL2561_CTRL_PWR_ON 0x03
#define TSL2561_CTRL_PWR_OFF 0x00

/*
 * Autogain thresholds
 */
#define TSL2561_AGC_THI_13MS 4850	// Max value at Ti 13ms = 5047
#define TSL2561_AGC_TLO_13MS 100
#define TSL2561_AGC_THI_101MS 36000 // Max value at Ti 101ms = 37177
#define TSL2561_AGC_TLO_101MS 200
#define TSL2561_AGC_THI_402MS 63000	// Max value at Ti 402ms = 65535
#define TSL2561_AGC_TLO_402MS 500

/*
 * Clipping thresholds
 */
#define TSL2561_CLIPPING_13MS 4900
#define TSL2561_CLIPPING_101MS 37000
#define TSL2561_CLIPPING_402MS 65000

/*
 * Constats for simplified lux calculation
 * according TAOS Inc.
 */
#define LUX_SCALE 14
#define RATIO_SCALE 9

#define CH_SCALE 10
#define CH_SCALE_TINT0 0x7517
#define CH_SCALE_TINT1 0x0FE7

/*
 * T, FN, and CL Package coefficients
 */
#define TSL2561_K1T 0x0040
#define TSL2561_B1T 0x01F2
#define TSL2561_M1T 0x01BE
#define TSL2561_K2T 0x0080
#define TSL2561_B2T 0x0214
#define TSL2561_M2T 0x02D1
#define TSL2561_K3T 0x00C0
#define TSL2561_B3T 0x023F
#define TSL2561_M3T 0x037B
#define TSL2561_K4T 0x0100
#define TSL2561_B4T 0x0270
#define TSL2561_M4T 0x03FE
#define TSL2561_K5T 0x0138
#define TSL2561_B5T 0x016F
#define TSL2561_M5T 0x01fC
#define TSL2561_K6T 0x019A
#define TSL2561_B6T 0x00D2
#define TSL2561_M6T 0x00FB
#define TSL2561_K7T 0x029A
#define TSL2561_B7T 0x0018
#define TSL2561_M7T 0x0012
#define TSL2561_K8T 0x029A
#define TSL2561_B8T 0x0000
#define TSL2561_M8T 0x0000

/*
 * CS package coefficients
 */
#define TSL2561_K1C 0x0043
#define TSL2561_B1C 0x0204
#define TSL2561_M1C 0x01AD
#define TSL2561_K2C 0x0085
#define TSL2561_B2C 0x0228
#define TSL2561_M2C 0x02C1
#define TSL2561_K3C 0x00C8
#define TSL2561_B3C 0x0253
#define TSL2561_M3C 0x0363
#define TSL2561_K4C 0x010A
#define TSL2561_B4C 0x0282
#define TSL2561_M4C 0x03DF
#define TSL2561_K5C 0x014D
#define TSL2561_B5C 0x0177
#define TSL2561_M5C 0x01DD
#define TSL2561_K6C 0x019A
#define TSL2561_B6C 0x0101
#define TSL2561_M6C 0x0127
#define TSL2561_K7C 0x029A
#define TSL2561_B7C 0x0037
#define TSL2561_M7C 0x002B
#define TSL2561_K8C 0x029A
#define TSL2561_B8C 0x0000
#define TSL2561_M8C 0x0000

#define TSL2561_FACTOR_US 1000000

class TSL2561 {
public:
	TSL2561(int dev = 0, int addr = TSL2561_I2C_ADDR_DEFAULT);
	virtual ~TSL2561();

	void enable();
	void disable();
	void setTiming(int, int);
	void setGain(int);
	void setIntegrationTime(int);
	void setType(int);
	void read(int*, int*);
	unsigned long lux();
	void luminosity(int*, int*);
	void enableAutoGain();
	void disableAutoGain();

private:
	bool writeByte(uint8_t, uint8_t);
	bool writeWord(uint8_t, uint8_t);
	uint16_t readWord(uint8_t);
	unsigned long computeLux(int, int);

	int m_deviceNumber;
	int m_integrationTime;
	int m_gain;
	bool m_autoGain;
	int m_type;
	int m_deviceAddress;
};

#endif /* TSL2561_TSL2561_H_ */
