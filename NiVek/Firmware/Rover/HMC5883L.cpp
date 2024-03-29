/*
HMC5883L.cpp - Class file for the HMC5883L Triple Axis Magnetometer Arduino Library.
Copyright (C) 2011 Love Electronics (loveelectronics.co.uk)/ 2012 bildr.org (Arduino 1.0 compatible)

This program is free software: you can redistribute it and/or modify
it under the terms of the version 3 GNU General Public License as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

WARNING: THE HMC5883L IS NOT IDENTICAL TO THE HMC5883!
Datasheet for HMC5883L:
http://www51.honeywell.com/aero/common/documents/myaerospacecatalog-documents/Defense_Brochures-documents/HMC5883L_3-Axis_Digital_Compass_IC.pdf

*/

#include <Arduino.h> 
#include "HMC5883L.h"

HMC5883L::HMC5883L()
{
	m_Scale = 1;
}

MagnetometerRaw HMC5883L::ReadRawAxis()
{
	uint8_t* buffer = Read(DataRegisterBegin, 6);
	m_Raw.XAxis = (buffer[0] << 8) | buffer[1];
	m_Raw.ZAxis = (buffer[2] << 8) | buffer[3];
	m_Raw.YAxis = (buffer[4] << 8) | buffer[5];

	if (m_calibrating)
	{
		if (m_Raw.XAxis < m_xMin) m_xMin = m_Raw.XAxis;
		if (m_Raw.YAxis < m_yMin) m_yMin = m_Raw.YAxis;
		if (m_Raw.XAxis > m_xMax) m_xMax = m_Raw.XAxis;
		if (m_Raw.YAxis > m_yMax) m_yMax = m_Raw.YAxis;
	}

	return m_Raw;
}

MagnetometerScaled HMC5883L::ReadScaledAxis()
{
	MagnetometerRaw raw = ReadRawAxis();
	MagnetometerScaled scaled = MagnetometerScaled();
	scaled.XAxis = raw.XAxis * m_Scale;
	scaled.ZAxis = raw.ZAxis * m_Scale;
	scaled.YAxis = raw.YAxis * m_Scale;
	return scaled;
}

int HMC5883L::SetScale(int gauss)
{
	uint8_t regValue = 0x00;
	if (gauss == 88)
	{
		regValue = 0x00;
		m_Scale = 0.73;
	}
	else if (gauss == 130)
	{
		regValue = 0x01;
		m_Scale = 0.92;
	}
	else if (gauss == 190)
	{
		regValue = 0x02;
		m_Scale = 1.22;
	}
	else if (gauss == 250)
	{
		regValue = 0x03;
		m_Scale = 1.52;
	}
	else if (gauss == 400)
	{
		regValue = 0x04;
		m_Scale = 2.27;
	}
	else if (gauss == 470)
	{
		regValue = 0x05;
		m_Scale = 2.56;
	}
	else if (gauss == 560)
	{
		regValue = 0x06;
		m_Scale = 3.03;
	}
	else if (gauss == 810)
	{
		regValue = 0x07;
		m_Scale = 4.35;
	}
	else
		return ErrorCode_1_Num;

	// Setting is in the top 3 bits of the register.
	regValue = regValue << 5;
	Write(ConfigurationRegisterB, regValue);
	return 0;
}


void HMC5883L::Print()
{

}

int HMC5883L::SetMeasurementMode(uint8_t mode)
{
	Write(ModeRegister, mode);
	return 0;
}

void HMC5883L::Write(int address, int data)
{
	Wire.beginTransmission(HMC5883L_Address);
	Wire.write(address);
	Wire.write(data);
	Wire.endTransmission();
}

uint8_t* HMC5883L::Read(int address, int length)
{
	Wire.beginTransmission(HMC5883L_Address);
	Wire.write(address);
	Wire.endTransmission();

	uint8_t buffer[16];

	int available = Wire.requestFrom(HMC5883L_Address, length);
	if (available == length)
	{
		for (uint8_t i = 0; i < length; i++)
			buffer[i] = (uint8_t)Wire.read();		
	}

	return buffer;
}

char* HMC5883L::GetErrorText(int errorCode)
{
	if (ErrorCode_1_Num == 1)
		return ErrorCode_1;

	return "Error not defined.";
}