#include <cstdio>	//stdandard output
#include <cstdlib>
#include "devices.h"
#include "wstring.h"

#include <string>	// std::string
#include <vector>	// std::vector<...>
#include <map>		// std::map< ... , ... >
#include <algorithm> //sort, min, max
#include <unistd.h>
#include <ios>

#include <fstream>
#include <iostream>

int num_devices = 0;
struct device_structure	soapy_devices[MAX_NUM_DEVICES];

std::string SoapySDRDeviceProbe(struct device_structure	*sdev);

/***********************************************************************
 * Find devices and print args
 **********************************************************************/
static int findDevices()
{
	String	information[MAX_NUM_RANGES];
	const auto results = SoapySDR::Device::enumerate("");

	for (size_t i = 0; i < results.size(); i++)
	{
		std::cout << "Found device " << i << std::endl;
		int ii = 0;
		for (const auto &it : results[i])
		{
			information[ii++] = String((char *)it.first.c_str());
			information[ii++] = String((char *)it.second.c_str());
			
			std::cout << "  " << it.first << " = " << it.second << std::endl;
		}
		std::cout << std::endl;	
	}
	num_devices =  results.size();
	if (results.empty()) std::cerr << "No devices found! " <<  std::endl;
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
		soapy_devices[0].sdr = NULL;
		std::cerr << "Error probing device: " << ex.what() << std::endl;
		return EXIT_FAILURE;
	}
	std::cout << std::endl;
	return EXIT_SUCCESS;
}


int discover_devices(String driver)
{	int i;
	
	findDevices();
	SoapySDR_setLogLevel(SOAPY_SDR_DEBUG);
	
	i =  probeDevice((char *)driver.c_str());
	// Channel information is collected in SoapySDRDeviceProbe
	
	return i;
}



//////////////////////////////////////////////////////////////////////////////
//
// process_mem_usage(double &, double &) - takes two doubles by reference,
// attempts to read the system-dependent data for a process' virtual memory
// size and resident set size, and return the results in KB.
//
// On failure, returns 0.0, 0.0

void process_mem_usage(double& vm_usage, double& resident_set)
{
	using std::ios_base;
	using std::ifstream;
	using std::string;

	vm_usage     = 0.0;
	resident_set = 0.0;

	// 'file' stat seems to give the most reliable results
	//
	ifstream stat_stream("/proc/self/stat", ios_base::in);

	// dummy vars for leading entries in stat that we don't care about
	//
	string pid, comm, state, ppid, pgrp, session, tty_nr;
	string tpgid, flags, minflt, cminflt, majflt, cmajflt;
	string utime, stime, cutime, cstime, priority, nice;
	string O, itrealvalue, starttime;

	// the two fields we want
	//
	unsigned long vsize;
	long rss;

	stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
	            >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
	            >> utime >> stime >> cutime >> cstime >> priority >> nice
	            >> O >> itrealvalue >> starttime >> vsize >> rss;  // don't care about the rest

	         stat_stream.close();

	long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024;  // in case x86-64 is configured to use 2MB pages
	vm_usage     = vsize / 1024.0;
	resident_set = rss * page_size_kb;
}