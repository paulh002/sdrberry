#include "TCA9548.h"

//
//    FILE: TCA9548.cpp
//  AUTHOR: Rob Tillaart
// VERSION: 0.1.0
//    DATE: 2021-03-16
// PURPOSE: Library for TCA9548 I2C multiplexer
//
//  HISTORY:
//  0.1.0   2021-03-16  initial version
//  Adapted for RPI by PA0PHH 2023-10-16

#include "TCA9548.h"
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
extern "C" {
#include <linux/i2c-dev.h>
}
#include <sys/ioctl.h>

TCA9548V2::TCA9548V2(const uint8_t deviceAddress)
	: _address(deviceAddress)
{
	_mask = 0x00;
	_resetPin = -1;
	_forced = false;
	fd = -1;
	_connected = false;
}

bool TCA9548V2::begin(uint8_t mask)
{
	//Open i2c device
	if ((fd = wiringPiI2CSetup(_address)) < 0)
		return false;

	if (!isConnected())
		return false;
	setChannelMask(mask);
	_connected = true;
	return true;
}

bool TCA9548V2::isConnected()
{
	int ret = wiringPiI2CRead(fd);
	return (ret != -1);
}

void TCA9548V2::enableChannel(uint8_t channel)
{
	if (isEnabled(channel))
		return;
	setChannelMask(_mask | (0x01 << channel));
}

void TCA9548V2::disableChannel(uint8_t channel)
{
	if (!isEnabled(channel))
		return;
	setChannelMask(_mask & ~(0x01 << channel));
}

void TCA9548V2::selectChannel(uint8_t channel)
{
	setChannelMask(0x01 << channel);
}

bool TCA9548V2::isEnabled(uint8_t channel)
{
	if (channel > 7)
		return false;
	return (_mask & (0x01 << channel));
}

void TCA9548V2::setChannelMask(uint8_t mask)
{
	if ((_mask == mask) && (!_forced))
		return;
	_mask = mask;
	wiringPiI2CWrite(fd, mask);
}

uint8_t TCA9548V2::getChannelMask()
{
	return _mask;
}

int TCA9548V2::getError()
{
	int e = _error;
	_error = 0;
	return e;
}

// -- END OF FILE --
