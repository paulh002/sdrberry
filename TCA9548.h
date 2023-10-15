#pragma once
#include <stddef.h>
#include <stdint.h>
#include <string>
#include <memory>
#include "wiringPiI2C.h"

//
//    FILE: TCA9548.h
//  AUTHOR: Rob Tillaart
// VERSION: 0.1.0
//    DATE: 2021-03-16
// PURPOSE: Library for TCA9548 I2C multiplexer
//
//     URL: https://github.com/RobTillaart/TCA9548
//
// Adapted for rpi PA0PHH 

#define TCA9548_LIB_VERSION (F("0.2.0"))

class TCA9548V2
{
  public:
	// address = 0x70 .. 0x77
	TCA9548V2(const char *i2c_device_filepath, const uint8_t deviceAddress = 0x70);
	bool begin(uint8_t mask = 0x00);   // default no channels enabled
	bool isConnected();				   // find multiplexer on I2C bus

	// channel = 0.. 7
	void enableChannel(uint8_t channel);
	void disableChannel(uint8_t channel);
	void selectChannel(uint8_t channel); // enable only this channel
	bool isEnabled(uint8_t channel);

	// mask = 0x00 .. 0xFF - every bit is a channel.
	void setChannelMask(uint8_t mask);
	uint8_t getChannelMask();

	// set forced write
	void setForced(bool forced) { _forced = forced; };
	bool getForced() { return _forced; };

	// TODO improve errorhandling ?
	int getError();

  private:
	uint8_t _mask = 0x00; // caching mask
	uint8_t _resetPin = -1;
	int _error = 0;
	uint8_t _address;
	bool _forced;
	int fd;
};

// -- END OF FILE --
