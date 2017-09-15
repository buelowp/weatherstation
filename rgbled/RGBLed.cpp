/*
 * RGBLed.cpp
 *
 *  Created on: Sep 15, 2017
 *      Author: buelowp
 */

#include "RGBLed.h"

RGBLed::RGBLed(int r, int g, int b)
{
	m_red = r;
	m_green = g;
	m_blue = b;

	m_omega2.SetDirection(m_red, OMEGA2_OUTPUT);
	m_omega2.SetDirection(m_green, OMEGA2_OUTPUT);
	m_omega2.SetDirection(m_blue, OMEGA2_OUTPUT);

	m_omega2.Set(m_red, 1);
	m_omega2.Set(m_green, 1);
	m_omega2.Set(m_blue, 1);
}

RGBLed::~RGBLed()
{
}

void RGBLed::red()
{
	m_omega2.Set(m_red, 0);
	m_omega2.Set(m_green, 1);
	m_omega2.Set(m_blue, 1);
}

void RGBLed::green()
{
	m_omega2.Set(m_red, 1);
	m_omega2.Set(m_green, 0);
	m_omega2.Set(m_blue, 1);
}

void RGBLed::blue()
{
	m_omega2.Set(m_red, 1);
	m_omega2.Set(m_green, 1);
	m_omega2.Set(m_blue, 0);
}

void RGBLed::white()
{
	m_omega2.Set(m_red, 0);
	m_omega2.Set(m_green, 0);
	m_omega2.Set(m_blue, 0);
}

void RGBLed::teal()
{
	m_omega2.Set(m_red, 1);
	m_omega2.Set(m_green, 0);
	m_omega2.Set(m_blue, 0);
}

void RGBLed::magenta()
{
	m_omega2.Set(m_red, 0);
	m_omega2.Set(m_green, 1);
	m_omega2.Set(m_blue, 0);
}
