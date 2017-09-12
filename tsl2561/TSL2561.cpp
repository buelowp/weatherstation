/*
 * TSL2561.cpp
 *
 *  Created on: Sep 12, 2017
 *      Author: buelowp
 */

#include "TSL2561.h"

TSL2561::TSL2561(int device, int addr)
{
	m_deviceNumber = device;
	m_type = 0;
	m_autoGain = false;
	m_integrationTime = TSL2561_INTEGRATION_TIME_402MS;
	m_deviceAddress = addr;

	enable();
	setTiming(m_integrationTime, TSL2561_GAIN_0X);
}

TSL2561::~TSL2561()
{
}

bool TSL2561::writeByte(uint8_t addr, uint8_t reg)
{
	if (i2c_writeBytes(m_deviceNumber, TSL2561_I2C_ADDR_DEFAULT, addr, reg, 1) == EXIT_FAILURE) {
		return false;
	}

	return true;
}

bool TSL2561::writeWord(uint8_t addr, uint8_t reg)
{
	if (i2c_writeBytes(m_deviceNumber, TSL2561_I2C_ADDR_DEFAULT, addr, reg, 1) == EXIT_FAILURE) {
		return false;
	}

	return true;
}

uint16_t TSL2561::readWord(uint8_t addr)
{
	int value;

	if (i2c_readByte(m_deviceNumber, m_deviceAddress, (int)addr, &value) == EXIT_FAILURE)
		return 0;

	return (uint16_t)value;
}

void TSL2561::setTiming(int inttime, int gain)
{
	m_integrationTime = inttime;
	m_gain = gain;

	writeByte(TSL2561_CMD_BIT | TSL2561_REG_TIMING, inttime | gain);
}

void TSL2561::setGain(int gain)
{
	m_gain = gain;

	writeByte(TSL2561_CMD_BIT | TSL2561_REG_TIMING, m_integrationTime | m_gain);
}

void TSL2561::setIntegrationTime(int t)
{
	m_integrationTime = t;

	writeByte(TSL2561_CMD_BIT | TSL2561_REG_TIMING, m_integrationTime | m_gain);
}

void TSL2561::enableAutoGain()
{
	m_autoGain = true;
}

void TSL2561::disableAutoGain()
{
	m_autoGain = false;
}

void TSL2561::enable()
{
	writeByte(TSL2561_CMD_BIT | TSL2561_REG_CTRL, TSL2561_CTRL_PWR_ON);
}

void TSL2561::disable()
{
	writeByte(TSL2561_CMD_BIT | TSL2561_REG_CTRL, TSL2561_CTRL_PWR_OFF);
}

void TSL2561::setType(int t)
{
	m_type = t;
}

void TSL2561::read(int *broadband, int *ir)
{
	enable();

	switch (m_integrationTime) {
	case TSL2561_INTEGRATION_TIME_402MS:
		usleep(0.403 * TSL2561_FACTOR_US);
		break;
	case TSL2561_INTEGRATION_TIME_101MS:
		usleep(0.102 * TSL2561_FACTOR_US);
		break;
	case TSL2561_INTEGRATION_TIME_13MS:
		usleep(0.014 * TSL2561_FACTOR_US);
		break;
	default:
		usleep(0.403 * TSL2561_FACTOR_US);
		break;
	}

	*broadband = readWord(TSL2561_CMD_BIT | TSL2561_WORD_BIT | TSL2561_REG_CH0_LOW);
	*ir = readWord(TSL2561_CMD_BIT | TSL2561_WORD_BIT | TSL2561_REG_CH1_LOW);

	if (broadband < 0 || ir < 0) {
		std::cerr << __PRETTY_FUNCTION__ << ": broadband is " << broadband << ", and ir is " << ir;
	}
	disable();
}

unsigned long TSL2561::lux()
{
	int visible;
	int channel1;
	int threshold;

	luminosity(&visible, &channel1);

	switch (m_integrationTime) {
	case TSL2561_INTEGRATION_TIME_13MS:
		threshold = TSL2561_CLIPPING_13MS;
		break;
	case TSL2561_INTEGRATION_TIME_101MS:
		threshold = TSL2561_CLIPPING_101MS;
		break;
	default:
		threshold = TSL2561_CLIPPING_402MS;
		break;
	}

	if((visible > threshold) || (channel1 > threshold))
		return 0;

	return computeLux(visible, channel1);
}

void TSL2561::luminosity(int *channel0, int *channel1)
{
	uint16_t hi;
	uint16_t lo;
	bool agc_check = false;
	bool valid = false;

	if (!m_autoGain) {
		read(channel0, channel1);
	}
	else {
		while (!valid) {
			switch(m_integrationTime) {
			case TSL2561_INTEGRATION_TIME_13MS:
				hi = TSL2561_AGC_THI_13MS;
				lo = TSL2561_AGC_TLO_13MS;
				break;
			case TSL2561_INTEGRATION_TIME_101MS:
				hi = TSL2561_AGC_THI_101MS;
				lo = TSL2561_AGC_TLO_101MS;
				break;
			default:
				hi = TSL2561_AGC_THI_402MS;
				lo = TSL2561_AGC_TLO_402MS;
				break;
			}
		}
		if (!agc_check) {
			if((*channel0 < lo) && (m_gain == TSL2561_GAIN_0X)) {
				setGain(TSL2561_GAIN_16X);
				read(channel0, channel1);
				agc_check = true;

			} else if ((*channel0 > hi) && (m_gain == TSL2561_GAIN_16X)) {
				setGain(TSL2561_GAIN_0X);
				read(channel0, channel1);
				agc_check = true;
			} else {
				valid = true;
			}
		}
		else {
			valid = true;
		}
	}
}

unsigned long TSL2561::computeLux(int ch0, int ch1)
{
	unsigned long ch_scale;
	unsigned long channel0;
	unsigned long channel1;

	// first, scale the channel values depending on the gain and integration time
	// 16X, 402mS is nominal.
	// scale if integration time is NOT 402 msec
	switch(m_integrationTime) {
		case TSL2561_INTEGRATION_TIME_13MS:
			ch_scale = CH_SCALE_TINT0;
			break;

		case TSL2561_INTEGRATION_TIME_101MS:
			ch_scale = CH_SCALE_TINT1;
			break;

		default:
			ch_scale = (1 << CH_SCALE);
			break;
	}

	// scale if gain is NOT 16X
	if(!m_gain)
		ch_scale = (ch_scale << 4);	// scale 1X to 16X

	// scale the channel values
	channel0 = (ch0 * ch_scale) >> CH_SCALE;
	channel1 = (ch1 * ch_scale) >> CH_SCALE;

	unsigned long ratio = 0;
	unsigned long ratio1 = 0;

	// find the ratio of the channel values (Channel1/Channel0)
	// protect against divide by zero
	if(channel0 != 0)
		ratio1 = (channel1 << (RATIO_SCALE + 1)) / channel0;

	// round the ratio value
	ratio = (ratio1 + 1) >> 1;

	int b, m;

	// is ratio <= eachBreak ?
	switch(m_type){
		case 1:
			if((ratio >= 0) && (ratio <= TSL2561_K1C)){
				b = TSL2561_B1C; m = TSL2561_M1C;
			} else if(ratio <= TSL2561_K2C) {
				b = TSL2561_B2C; m = TSL2561_M2C;
			} else if(ratio <= TSL2561_K3C) {
				b = TSL2561_B3C; m = TSL2561_M3C;
			} else if(ratio <= TSL2561_K4C) {
				b = TSL2561_B4C; m = TSL2561_M4C;
			} else if (ratio <= TSL2561_K5T) {
				b = TSL2561_B5C; m = TSL2561_M5C;
			} else if(ratio <= TSL2561_K6T) {
				b = TSL2561_B6C; m = TSL2561_M6C;
			} else if(ratio <= TSL2561_K7T) {
				b = TSL2561_B7C; m = TSL2561_M7C;
			} else if(ratio > TSL2561_K8C) {
				b = TSL2561_B8C; m = TSL2561_M8C;
			}
			break;

		case 0:
		default:
			if((ratio >= 0) && (ratio <= TSL2561_K1T)){
				b = TSL2561_B1T; m = TSL2561_M1T;
			} else if(ratio <= TSL2561_K2T) {
				b = TSL2561_B2T; m = TSL2561_M2T;
			} else if(ratio <= TSL2561_K3T) {
				b = TSL2561_B3T; m = TSL2561_M3T;
			} else if(ratio <= TSL2561_K4T) {
				b = TSL2561_B4T; m = TSL2561_M4T;
			} else if (ratio <= TSL2561_K5T) {
				b = TSL2561_B5T; m = TSL2561_M5T;
			} else if(ratio <= TSL2561_K6T) {
				b = TSL2561_B6T; m = TSL2561_M6T;
			} else if(ratio <= TSL2561_K7T) {
				b = TSL2561_B7T; m = TSL2561_M7T;
			} else if(ratio > TSL2561_K8T) {
				b = TSL2561_B8T; m = TSL2561_M8T;
			}
			break;
	}

	unsigned long tmp = (channel0 * b) - (channel1 * m);

	if(tmp < 0)
		tmp = 0;

	tmp += (1 << (LUX_SCALE-1));
	unsigned long lux = (tmp >> LUX_SCALE);

	return lux;
}




