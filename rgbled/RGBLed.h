/*
 * RGBLed.h
 *
 *  Created on: Sep 15, 2017
 *      Author: buelowp
 */

#ifndef RGBLED_RGBLED_H_
#define RGBLED_RGBLED_H_

#include <fastgpioomega2.h>

#define OMEGA2_INPUT 	0
#define OMEGA2_OUTPUT 	1

class RGBLed {
public:
	RGBLed(int r = 17, int g = 16, int b = 15);
	virtual ~RGBLed();

	void red();
	void blue();
	void green();
	void white();
	void magenta();
	void teal();

private:
	int m_red;
	int m_blue;
	int m_green;
	FastGpioOmega2 m_omega2;
};

#endif /* RGBLED_RGBLED_H_ */
