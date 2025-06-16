#include "i2cinput.h"
#include <unistd.h>

shared_ptr<i2cinput> ptr_i2cinput_thread;
std::thread i2cinput_thread;

void i2cinput::initI2Cdevice()
{
	std::string col = Settings_file.get_string("i2c", "input_device");
	std::string input_address = Settings_file.get_string("i2c", "input_address");

	if (col == "PCF8574" || col == "PCF8574A")
	{
		int I2Caddress;
		sscanf(input_address.c_str(), "%x", &I2Caddress);
		PCF8574 i2cdevice(I2Caddress);
		if (i2cdevice.begin(0))
		{
			i2cDevice = i2cdevice;
			printf("Connected to PCF8574(A) %x \n", (int)i2cdevice.getAddress());
			i2cdevice.write8(0xFF);
			isconnected = true;
		}
		else
		{
			printf("Cannot connect to %x \n", (int)i2cdevice.getAddress());
		}
	}
	if (col == "TCA9548")
	{
		int I2Caddress;
		sscanf(input_address.c_str(), "%x", &I2Caddress);
		TCA9548V2 i2cdevice(I2Caddress);
		if (i2cdevice.begin(0))
		{
			i2cDevice = i2cdevice;
			printf("Connected to TCA9548 %x \n", (int)i2cdevice.getAddress());
			isconnected = true;
		}
		else
		{
			printf("Cannot connect to %x \n", (int)i2cdevice.getAddress());
		}
	}
	if (col == "MCP23008")
	{
		int I2Caddress;
		sscanf(input_address.c_str(), "%x", &I2Caddress);
		MCP23008 i2cdevice(I2Caddress);
		if (i2cdevice.begin())
		{
			i2cDevice = i2cdevice;
			i2cdevice.pinMode8(MCP23008_INPUT);
			i2cdevice.write8(0x00);
			printf("Connected to MCP23008 %x \n", (int)i2cdevice.getAddress());
			isconnected = true;
		}
		else
		{
			printf("Cannot connect to %x \n", (int)i2cdevice.getAddress());
		}
	}
}

i2cinput::i2cinput()
{
}

void i2cinput::operator()()
{
	initI2Cdevice();
	while (!stop_flag.load())
	{
		switch (i2cDevice.index())
		{
		case 0:
			if (std::get<PCF8574>(i2cDevice).getConnected())
				data.store(std::get<PCF8574>(i2cDevice).read8());
			break;
		case 1:
			if (std::get<TCA9548V2>(i2cDevice).getConnected())
				data = std::get<TCA9548V2>(i2cDevice).getChannelMask();
			break;
		case 2:
			if (std::get<MCP23008>(i2cDevice).getConnected())
				data = std::get<MCP23008>(i2cDevice).read8();
			break;
		}
		usleep(100000);
	}
}

void i2cinput::create_i2c_input_thread()
{
	std::string input_device = Settings_file.get_string("i2c", "input_device");
	std::string input_address = Settings_file.get_string("i2c", "input_address");

	if (input_device.size() > 0 && input_address.size() > 0)
	{
		if (ptr_i2cinput_thread == nullptr)
		{
			ptr_i2cinput_thread = make_shared<i2cinput>();
			ptr_i2cinput_thread->stop_flag.store(false);
			i2cinput_thread = std::thread(&i2cinput::operator(), ptr_i2cinput_thread);
		}
	}
}

void i2cinput::destroy_i2c_input_thread()
{
	if (ptr_i2cinput_thread != nullptr)
	{
		ptr_i2cinput_thread->stop_flag.store(true);
		i2cinput_thread.join();
		ptr_i2cinput_thread.reset();
	}
}

uint16_t i2cinput::get_data()
{
	if (ptr_i2cinput_thread != nullptr)
		return ptr_i2cinput_thread->data.load();
	return 0;
}

uint16_t i2cinput::get_pin(int pin)
{
	if (ptr_i2cinput_thread != nullptr)
	{
		return (ptr_i2cinput_thread->data.load() & (0x01 << pin));
	}

	return 0;
}

bool i2cinput::connected()
{
	if (ptr_i2cinput_thread != nullptr)
	{
		return (ptr_i2cinput_thread->isconnected);
	}

	return false;
}