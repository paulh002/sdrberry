#include "BandFilter.h"

void BandFilter::initFilter()
{
	std::vector<std::string> devices;
	std::vector<std::string> addresses;

	devices = Settings_file.get_array_string("i2c", "devices");
	addresses = Settings_file.get_array_string("i2c", "address");

	int index = 0;
	for (auto col : devices)
	{
		if (col == "PCF8574" || col == "PCF8574A")
		{
			int I2Caddress;
			sscanf(addresses.at(index).c_str(),"%x",&I2Caddress);
			PCF8574 i2cdevice(I2Caddress);
			if (i2cdevice.begin(0))
			{
				i2cDevices.push_back(i2cdevice);
				printf("Connected to %d \n", (int)i2cdevice.getAddress());
			}
			else
			{
				i2cDevices.push_back(i2cdevice);
				printf("Cannot connect to %d \n", (int)i2cdevice.getAddress());
			}
		}
		if (col == "TCA9548")
		{
			int I2Caddress;
			sscanf(addresses.at(index).c_str(), "%x", &I2Caddress);
			TCA9548V2 i2cdevice(I2Caddress);
			if (i2cdevice.begin(0))
			{			
				i2cDevices.push_back(i2cdevice);
				printf("Connected to %d \n", (int)i2cdevice.getAddress());
			}
			else
			{
				i2cDevices.push_back(i2cdevice);
				printf("Cannot connect to %d \n", (int)i2cdevice.getAddress());
			}
		}
		if (col == "MCP23008")
		{
			int I2Caddress;
			sscanf(addresses.at(index).c_str(), "%x", &I2Caddress);
			MCP23008 i2cdevice(I2Caddress);
			if (i2cdevice.begin())
			{
				i2cdevice.pinMode8(0);
				i2cDevices.push_back(i2cdevice);
				printf("Connected to %d \n", (int)i2cdevice.getAddress());
			}
			else
			{
				i2cDevices.push_back(i2cdevice);
				printf("Cannot connect to %d \n", (int)i2cdevice.getAddress());
			}
		}
		index++;
	}	
}

void BandFilter::SetBand(int band, bool rx)
{
	int index = vfo.getBandIndex(band);
	char str[80];
	std::vector<int> I2CCommands_rx, I2CCommands_tx;

	if (!bandfilter_pass_trough)
		sprintf(str, "%dm", band);
	else
		strcpy(str, "Through");
	std::string bandMeters(str);
	std::vector<std::string> bandCommands = Settings_file.get_array_string("i2c", bandMeters);
	int i = 0;
	for (auto col : bandCommands)
	{
		int I2Caddress;
		sscanf(col.c_str(), "%x", &I2Caddress);
		if (i < bandCommands.size() / 2)
			I2CCommands_rx.push_back(I2Caddress);
		else
			I2CCommands_tx.push_back(I2Caddress);
		i++;
	}

	if (i2cDevices.size() > 0)
	{
		const auto startTime = std::chrono::high_resolution_clock::now();
		int ii = 0;
		printf("%d m i2c ", band);
		for (auto &col : i2cDevices)
		{
			if (rx)
			{
				switch (col.index())
				{
				case 0:
					if (std::get<PCF8574>(col).getConnected())
						std::get<PCF8574>(col).write8(I2CCommands_rx.at(ii));
					break;
				case 1:
					if (std::get<TCA9548V2>(col).getConnected())
						std::get<TCA9548V2>(col).setChannelMask(I2CCommands_rx.at(ii));
					break;
				case 2:
					if (std::get<MCP23008>(col).getConnected())
						std::get<MCP23008>(col).write8(I2CCommands_rx.at(ii));
					break;
				}
				printf("rx %d ", I2CCommands_rx.at(ii));
			}
			else
			{
				switch (col.index())
				{
				case 0:
					if (std::get<PCF8574>(col).getConnected())
						std::get<PCF8574>(col).write8(I2CCommands_tx.at(ii));
					break;
				case 1:
					if (std::get<TCA9548V2>(col).getConnected())
						std::get<TCA9548V2>(col).setChannelMask(I2CCommands_tx.at(ii));
					break;
				case 2:
					if (std::get<MCP23008>(col).getConnected())
						std::get<MCP23008>(col).write8(I2CCommands_tx.at(ii));
					break;
				}
				printf("tx %d ", I2CCommands_tx.at(ii));
			}
			ii++;
		}
		auto now = std::chrono::high_resolution_clock::now();
		const auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);
		printf(" I2C time %ld\n", timePassed.count());
		return;
	}
}

void BandFilter::Setpasstrough(bool b)
{
	bandfilter_pass_trough = b;
	SetBand(vfo.get_band_in_meters(), vfo.get_rx());
}