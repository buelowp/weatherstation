/*
 * CCS811.h
 *
 *  Created on: Sep 13, 2017
 *      Author: buelowp
 */

#ifndef CCS811_CCS811_H_
#define CCS811_CCS811_H_

#include <list>
#include <string>
#include <iostream>
#include <fastgpioomega2.h>
#include <onion-i2c.h>

#define CCS811_ADDR            0x5B   // when I2C_ADDR pin is HIGH

// Registers for CCS811
#define CCS811_STATUS          0x00
#define CCS811_MEAS_MODE       0x01
#define CCS811_ALG_RESULT_DATA 0x02
#define CCS811_ENV_DATA        0x05
#define CCS811_APP_START       0xF4
#define CCS811_HW_ID           0x20
#define CCS811_ERROR_ID        0xE0
#define CCS811_SW_RESET        0xFF

#define OMEGA2_INPUT 	0
#define OMEGA2_OUTPUT 	1

class CCS811 {
public:
	CCS811(int pin, int addr = CCS811_ADDR);
	virtual ~CCS811();

	void prepare();
	uint8_t readHW_ID();
    uint8_t readStatus();
    uint8_t readErrorID(uint8_t);
    void getData(int*, int*);

private:
    void sensorSleep(int, int);
    void compensate(float, float);

	FastGpioOmega2 m_omega2;
	int m_gpio;
    int m_tvoc;
    int m_co2;
    int m_deviceNumber;
    int m_deviceAddress;
};

#endif /* CCS811_CCS811_H_ */
