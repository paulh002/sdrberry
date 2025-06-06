#pragma once
//
//    FILE: PCF8574.H
//  AUTHOR: Rob Tillaart
//    DATE: 02-febr-2013
// VERSION: 0.3.1
// PURPOSE: Arduino library for PCF8574 - 8 channel I2C IO expander
//     URL: https://github.com/RobTillaart/PCF8574
//          http://forum.arduino.cc/index.php?topic=184800
//
//			PA0PHH Adjusted for Raspberry PI
// HISTORY:
// see PCF8574.cpp file
//

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <stdint.h>
#include <unistd.h>
#include <memory>
#include "wiringPiI2C.h"

#define PCF8574_LIB_VERSION (F("0.3.1"))

#ifndef PCF8574_INITIAL_VALUE
#define PCF8574_INITIAL_VALUE 0xFF
#endif

#define PCF8574_OK 0x00
#define PCF8574_PIN_ERROR 0x81
#define PCF8574_I2C_ERROR 0x82

#define HIGH 1
#define LOW 0

class PCF8574
{
  public:
	PCF8574() = default;
	explicit PCF8574(const uint8_t deviceAddress);

#if defined(ESP8266) || defined(ESP32)
	bool begin(uint8_t sda, uint8_t scl, uint8_t val = PCF8574_INITIAL_VALUE);
#endif
	bool begin(uint8_t val = PCF8574_INITIAL_VALUE);
	bool isConnected();
	bool getConnected() { return _connected; }

	uint8_t read8();
	uint8_t read(uint8_t pin);
	uint8_t value() const { return _dataIn; };

	void write8(const uint8_t value);
	void write(const uint8_t pin, const uint8_t value);
	uint8_t valueOut() const { return _dataOut; }

	//added 0.1.07/08 Septillion
	inline uint8_t readButton8() { return PCF8574::readButton8(_buttonMask); }
	uint8_t readButton8(const uint8_t mask);
	uint8_t readButton(const uint8_t pin);
	inline void setButtonMask(uint8_t mask) { _buttonMask = mask; };

	// rotate, shift, toggle, reverse expect all lines are output
	void toggle(const uint8_t pin);
	void toggleMask(const uint8_t mask = 0xFF); // default 0xFF ==> invertAll()
	void shiftRight(const uint8_t n = 1);
	void shiftLeft(const uint8_t n = 1);
	void rotateRight(const uint8_t n = 1);
	void rotateLeft(const uint8_t n = 1);
	void reverse();

	int lastError();
	uint8_t getAddress() { return _address; }

  private:
	uint8_t _address;
	uint8_t _dataIn;
	uint8_t _dataOut;
	uint8_t _buttonMask;
	bool _connected;
	int _error;
	int fd;
};

// -- END OF FILE --
