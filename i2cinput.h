#pragma once
#include "MCP23008.h"
#include "PCF8574.h"
#include "Settings.h"
#include "TCA9548.h"
#include <memory>
#include <variant>
#include <semaphore.h>
#include <thread>

class i2cinput
{
  public:
	i2cinput();
	
	static void create_i2c_input_thread();
	static void destroy_i2c_input_thread();
	static uint16_t get_data();
	static uint16_t get_pin(int pin);

  private:
	atomic<bool> stop_flag{false};
	void operator()();
	void initI2Cdevice();
	std::variant<PCF8574, TCA9548V2, MCP23008> i2cDevice;
	std::atomic<uint16_t> data;
};
