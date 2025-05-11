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
	i2cgeneric();
	bool connected();
	int read();
	void write(int data);

  private:
	bool isconnected{false};
	void initI2Cdevice(std::string device_name, std::string i2c_address);
	std::variant<PCF8574, TCA9548V2, MCP23008> i2cDevice;
	
};
