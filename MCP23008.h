#pragma once
//
//    FILE: MCP23008.h
//  AUTHOR: Rob Tillaart
// VERSION: 0.1.5
// PURPOSE: Arduino library for I2C MCP23008 8 channel port expander
//    DATE: 2022-01-10
//     URL: https://github.com/RobTillaart/MCP23008

#include "wiringPiI2C.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <stdint.h>
#include <string>
#include <unistd.h>

#define MCP23008_LIB_VERSION (F("0.2.5"))

#define MCP23008_OK 0x00
#define MCP23008_PIN_ERROR 0x81
#define MCP23008_I2C_ERROR 0x82
#define MCP23008_VALUE_ERROR 0x83
#define MCP23008_PORT_ERROR 0x84

#define MCP23008_INVALID_READ -100

#define MCP23008_INPUT	0x00
#define MCP23008_OUTPUT	0x01
#define MCP23008_INPUT_PULLUP	0x03
#define MCP23008_HIGH 0x01
#define MCP23008_LOW 0x00

class MCP23008
{
  public:
	MCP23008() = default;
	MCP23008(uint8_t address);

	bool begin();
	bool isConnected();
	bool getConnected() { return _connected; }

	//      single pin interface
	//      mode =  INPUT,  OUTPUT or  INPUT_PULLUP (== INPUT)
	bool pinMode(uint8_t pin, uint8_t mode);
	bool digitalWrite(uint8_t pin, uint8_t value);
	uint8_t digitalRead(uint8_t pin);

	bool setPolarity(uint8_t pin, bool reversed);
	bool getPolarity(uint8_t pin, bool &reversed);
	bool setPullup(uint8_t pin, bool pullup);
	bool getPullup(uint8_t pin, bool &pullup);

	//      8 pins interface
	//      value = bit pattern
	bool pinMode8(uint8_t value);
	bool write8(uint8_t value);
	int read8();

	bool setPolarity8(uint8_t mask);
	bool getPolarity8(uint8_t &mask);
	bool setPullup8(uint8_t mask);
	bool getPullup8(uint8_t &mask);

	int lastError();

	//  DEBUG functions
	uint8_t getPinMode8();
	uint8_t getAddress() { return _address; }
	
  private:
	bool writeReg(uint8_t reg, uint8_t value);
	uint8_t readReg(uint8_t reg);

	uint8_t _address;
	bool _connected;
	uint8_t _error;
	int fd;
};

//  -- END OF FILE --
