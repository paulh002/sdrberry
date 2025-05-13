#pragma once
#include "MCP23008.h"
#include "PCF8574.h"
#include "Settings.h"
#include "TCA9548.h"
#include <memory>
#include <semaphore.h>
#include <thread>
#include <variant>
#include <string>

class i2cgeneric
{
  public:
	bool connected();
	int read();
	void write(uint8_t data);
	void write_pin(uint8_t pin, uint8_t data);
	void initI2Cdevice(std::string device_name, std::string i2c_address);

  protected:
	bool isconnected{false};
	std::variant<PCF8574, TCA9548V2, MCP23008> i2cDevice;
	
};
