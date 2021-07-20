#include <cstdio>	//stdandard output
#include <cstdlib>
#include "devices.h"
#include "wstring.h"

#include <string>	// std::string
#include <vector>	// std::vector<...>
#include <map>		// std::map< ... , ... >
#include <algorithm> //sort, min, max

#include <iostream>

int num_devices = 0;
struct device_structure	soapy_devices[MAX_NUM_DEVICES];

std::string SoapySDRDeviceProbe(struct device_structure	*sdev);

/***********************************************************************
 * Find devices and print args
 **********************************************************************/
static int findDevices(const std::string &argStr, struct device_structure *dev)
{
	const auto results = SoapySDR::Device::enumerate(argStr);

	for (size_t i = 0; i < results.size(); i++)
	{
		std::cout << "Found device " << i << std::endl;
		dev[i].device_no = i;
		int ii = 0;
		for (const auto &it : results[i])
		{
			dev[i].information[ii++] = String(it.first.c_str());
			dev[i].information[ii++] = String(it.second.c_str());
			
			std::cout << "  " << it.first << " = " << it.second << std::endl;
		}
		std::cout << std::endl;
	}
	if (results.empty()) std::cerr << "No devices found! " << argStr << std::endl;
		else std::cout << std::endl;

	return results.empty() ? EXIT_FAILURE : EXIT_SUCCESS;
}

/***********************************************************************
 * Make device and print hardware info
 **********************************************************************/
static int makeDevice(const std::string &argStr)
{
	std::cout << "Make device " << argStr << std::endl;
	try
	{
		auto device = SoapySDR::Device::make(argStr);
		std::cout << "  driver=" << device->getDriverKey() << std::endl;
		std::cout << "  hardware=" << device->getHardwareKey() << std::endl;
		for (const auto &it : device->getHardwareInfo())
		{
			std::cout << "  " << it.first << "=" << it.second << std::endl;
		}
		SoapySDR::Device::unmake(device);
	}
	catch (const std::exception &ex)
	{
		std::cerr << "Error making device: " << ex.what() << std::endl;
		return EXIT_FAILURE;
	}
	std::cout << std::endl;
	return EXIT_SUCCESS;
}

/***********************************************************************
 * Make device and print detailed info
 **********************************************************************/
static int probeDevice(const std::string &argStr)
{
	std::cout << "Probe device " << argStr << std::endl;
	try
	{
		auto device = SoapySDR::Device::make(argStr);
		soapy_devices[0].sdr = device;
		std::cout << SoapySDRDeviceProbe(&soapy_devices[0]) << std::endl;
		// SoapySDR::Device::unmake(device);
	}
	catch (const std::exception &ex)
	{
		std::cerr << "Error probing device: " << ex.what() << std::endl;
		return EXIT_FAILURE;
	}
	std::cout << std::endl;
	return EXIT_SUCCESS;
}


int discover_devices()
{

	findDevices("", soapy_devices);
	SoapySDR_setLogLevel(SOAPY_SDR_DEBUG);
	return probeDevice("driver=plutosdr,hostname=192.168.100.1");
}