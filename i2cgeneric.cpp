#include "i2cgeneric.h"

void i2cgeneric::initI2Cdevice(std::string device_name, std::string i2c_address)
{
	if (device_name == "PCF8574" || device_name == "PCF8574A")
	{
		int I2Caddress;
		sscanf(i2c_address.c_str(), "%x", &I2Caddress);
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
	if (device_name == "TCA9548")
	{
		int I2Caddress;
		sscanf(i2c_address.c_str(), "%x", &I2Caddress);
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
	if (device_name == "MCP23008")
	{
		int I2Caddress;
		sscanf(i2c_address.c_str(), "%x", &I2Caddress);
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

int i2cgeneric::read()
{
	int data = 0;
	
	switch (i2cDevice.index())
	{
	case 0:
		if (std::get<PCF8574>(i2cDevice).getConnected())
			data = std::get<PCF8574>(i2cDevice).read8();
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
	return data;
}

void i2cgeneric::write(int data)
{
	switch (i2cDevice.index())
	{
	case 0:
		if (std::get<PCF8574>(i2cDevice).getConnected())
			std::get<PCF8574>(i2cDevice).write8(data);
		break;
	case 1:
		//if (std::get<TCA9548V2>(i2cDevice).getConnected())
		//	std::get<TCA9548V2>(i2cDevice).getChannelMask();
		break;
	case 2:
		if (std::get<MCP23008>(i2cDevice).getConnected())
			std::get<MCP23008>(i2cDevice).write8(data);
		break;
	}
}