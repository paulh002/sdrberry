#include <map>		// std::map< ... , ... >
#include <algorithm> //sort, min, max
#include <string>
#include <cstdlib>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <iomanip>
#include <csignal>
#include <chrono>
#include <cstdio>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "SdrDevice.h"

template <typename Type>
	std::string toString(const std::vector<Type> &options)
	{
		std::stringstream ss;
		if (options.empty()) return "";
		for (size_t i = 0; i < options.size(); i++)
		{
			if (not ss.str().empty()) ss << ", ";
			ss << options[i];
		}
		return ss.str();
	}

std::string toString(const SoapySDR::Range &range)
{
	std::stringstream ss;
	ss << "[" << range.minimum() << ", " << range.maximum();
	if (range.step() != 0.0) ss << ", " << range.step();
	ss << "]";
	return ss.str();
}

std::string toString(const SoapySDR::RangeList &range, const double scale)
{
	const size_t MAXRLEN = 10; //for abbreviating long lists
	std::stringstream ss;
	for (size_t i = 0; i < range.size(); i++)
	{
		if (range.size() >= MAXRLEN and i >= MAXRLEN / 2 and i < (range.size() - MAXRLEN / 2))
		{
			if (i == MAXRLEN) ss << ", ...";
			continue;
		}
		if (not ss.str().empty()) ss << ", ";
		if (range[i].minimum() == range[i].maximum()) ss << (range[i].minimum() / scale);
		else ss << "[" << (range[i].minimum() / scale) << ", " << (range[i].maximum() / scale) << "]";
	}
	return ss.str();
}

std::string toString(const std::vector<double> &nums, const double scale)
{
	std::stringstream ss;

	if (nums.size() > 5)
	{
		ss << "[" << (nums.front() / scale) << ", " << (nums.back() / scale) << "]";
		return ss.str();
	}

	for (size_t i = 0; i < nums.size(); i++)
	{
		if (not ss.str().empty()) ss << ", ";
		ss << (nums[i] / scale);
	}
	return "[" + ss.str() + "]";
}

std::string toString(const SoapySDR::ArgInfo &argInfo, const std::string indent = "    ")
{
	std::stringstream ss;

	//name, or use key if missing
	std::string name = argInfo.name;
	if (argInfo.name.empty()) name = argInfo.key;
	ss << indent << " * " << name;

	//optional description
	std::string desc = argInfo.description;
	const std::string replace("\n" + indent + "   ");
	for (size_t pos = 0; (pos = desc.find("\n", pos)) != std::string::npos; pos += replace.size())
	{
		desc.replace(pos, 1, replace);
	}
	if (not desc.empty()) ss << " - " << desc << std::endl << indent << "  ";

	//other fields
	ss << " [key=" << argInfo.key;
	if (not argInfo.units.empty()) ss << ", units=" << argInfo.units;
	if (not argInfo.value.empty()) ss << ", default=" << argInfo.value;

	//type
	switch (argInfo.type)
	{
	case SoapySDR::ArgInfo::BOOL: ss << ", type=bool"; break;
	case SoapySDR::ArgInfo::INT: ss << ", type=int"; break;
	case SoapySDR::ArgInfo::FLOAT: ss << ", type=float"; break;
	case SoapySDR::ArgInfo::STRING: ss << ", type=string"; break;
	}

	//optional range/enumeration
	if (argInfo.range.minimum() < argInfo.range.maximum()) ss << ", range=" << toString(argInfo.range);
	if (not argInfo.options.empty()) ss << ", options=(" << toString(argInfo.options) << ")";

	ss << "]";

	return ss.str();
}

std::string toString(const SoapySDR::ArgInfoList &argInfos)
{
	std::stringstream ss;

	for (size_t i = 0; i < argInfos.size(); i++)
	{
		ss << toString(argInfos[i]) << std::endl;
	}

	return ss.str();
}

std::string sensorReadings(SoapySDR::Device *device)
{
	std::stringstream ss;

	/*******************************************************************
	 * Sensor readings
	 ******************************************************************/
	std::vector<std::string> sensors = device->listSensors();

	for (size_t i = 0; i < sensors.size(); i++)
	{
		std::string key = sensors[i];
		SoapySDR::ArgInfo info = device->getSensorInfo(key);
		std::string reading = device->readSensor(key);

		ss << "     * " << sensors[i];
		if (not info.name.empty()) ss << " (" << info.name << ")";
		ss << ":";
		if (info.range.maximum() > std::numeric_limits<double>::min()) ss << toString(info.range);
		ss << toString(info.options);
		ss << " " << reading;
		if (not info.units.empty()) ss << " " << info.units;
		ss << std::endl;
		if (not info.description.empty()) ss << "        " << info.description << std::endl;
	}

	return ss.str();
}

std::string channelSensorReadings(SoapySDR::Device *device, const int dir, const size_t chan)
{
	std::stringstream ss;

	/*******************************************************************
	 * Channel sensor readings
	 ******************************************************************/
	std::vector<std::string> sensors = device->listSensors(dir, chan);

	for (size_t i = 0; i < sensors.size(); i++)
	{
		std::string key = sensors[i];
		SoapySDR::ArgInfo info = device->getSensorInfo(dir, chan, key);
		std::string reading = device->readSensor(dir, chan, key);

		ss << "     * " << sensors[i];
		if (not info.name.empty()) ss << " (" << info.name << ")";
		ss << ":";
		if (info.range.maximum() > std::numeric_limits<double>::min()) ss << toString(info.range);
		ss << toString(info.options);
		ss << " " << reading;
		if (not info.units.empty()) ss << " " << info.units;
		ss << std::endl;
		if (not info.description.empty()) ss << "        " << info.description << std::endl;
	}

	return ss.str();
}

SdrDevice::SdrDevice(std::string drv)
	: soapyDevice(nullptr)
{
	driver = drv;
}

int SdrDevice::MakeSdrDevice()
{
	if (makeDevice() == EXIT_SUCCESS)
	{
		std::cout << SoapySDRDeviceProbe() << std::endl;
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}

SdrDevice::~SdrDevice()
{
	rx_channels.clear();	
	tx_channels.clear();	
	if (soapyDevice != nullptr) 
		SoapySDR::Device::unmake(soapyDevice);
}

int	SdrDevice::makeDevice()
{
	std::cout << "Make device " << driver << std::endl;
	try
	{
		soapyDevice = SoapySDR::Device::make(driver);
		if (!soapyDevice)
		{
			std::cout << "Error making device: " << std::endl;
			return EXIT_FAILURE;
		}			
		std::cout << "  driver=" << soapyDevice->getDriverKey() << std::endl;
		std::cout << "  hardware=" << soapyDevice->getHardwareKey() << std::endl;
		for (const auto &it : soapyDevice->getHardwareInfo())
		{
			std::cout << "  " << it.first << "=" << it.second << std::endl;
		}
	}
	catch (const std::exception &ex)
	{
		std::cerr << "Error making device: " << ex.what() << std::endl;
		return EXIT_FAILURE;
	}
	std::cout << std::endl;
	return EXIT_SUCCESS;
}

int	SdrDevice::UnmakeSdrDevice()
{
	std::cout << "UnMake device " << driver << std::endl;
	try
	{
		SoapySDR::Device::unmake(soapyDevice);
	}
	catch (const std::exception &ex)
	{
		std::cerr << "Error unmaking device: " << ex.what() << std::endl;
		return EXIT_FAILURE;
	}
	std::cout << std::endl;
	return EXIT_SUCCESS;
}


std::string SdrDevice::SoapySDRDeviceProbe()
{
	std::stringstream ss;

	/*******************************************************************
	 * Identification info
	 ******************************************************************/
	ss << std::endl;
	ss << "----------------------------------------------------" << std::endl;
	ss << "-- Device identification" << std::endl;
	ss << "----------------------------------------------------" << std::endl;

	driver_name = std::string(soapyDevice->getDriverKey().c_str());
	hardware = std::string(soapyDevice->getHardwareKey().c_str());	
	
	ss << "  driver=" << soapyDevice->getDriverKey() << std::endl;
	ss << "  hardware=" << soapyDevice->getHardwareKey() << std::endl;
	int ii = 0;
	for (const auto &it : soapyDevice->getHardwareInfo())
	{
			information.push_back(std::string(it.first.c_str()));
			information.push_back(std::string(it.first.c_str()));   
			ss << "  " << it.first << "=" << it.second << std::endl;
	}

	/*******************************************************************
	 * Available peripherals
	 ******************************************************************/
	ss << std::endl;
	ss << "----------------------------------------------------" << std::endl;
	ss << "-- Peripheral summary" << std::endl;
	ss << "----------------------------------------------------" << std::endl;

	numRxChans = soapyDevice->getNumChannels(SOAPY_SDR_RX);
	numTxChans = soapyDevice->getNumChannels(SOAPY_SDR_TX);
	ss << "  Channels: " << numRxChans << " Rx, " << numTxChans << " Tx" << std::endl;
	ss << "  Timestamps: " << (soapyDevice->hasHardwareTime() ? "YES" : "NO") << std::endl;
	bTimestamps = soapyDevice->hasHardwareTime();

	clockSources = toString(soapyDevice->listClockSources());
	if (not clockSources.empty()) ss << "  Clock sources: " << clockSources << std::endl;

	timeSources = toString(soapyDevice->listTimeSources());
	if (not timeSources.empty()) ss << "  Time sources: " << timeSources << std::endl;

	sensors = toString(soapyDevice->listSensors());
	if (not sensors.empty()) ss << "  Sensors: " << sensors << std::endl;
	ss << sensorReadings(soapyDevice);
	
	registers = toString(soapyDevice->listRegisterInterfaces());
	if (not registers.empty()) ss << "  Registers: " << registers << std::endl;

	settings = toString(soapyDevice->getSettingInfo());
	if (not settings.empty()) ss << "  Other Settings:" << std::endl << settings;

	gpios = toString(soapyDevice->listGPIOBanks());
	if (not gpios.empty()) ss << "  GPIOs: " << gpios << std::endl;

	uarts = toString(soapyDevice->listUARTs());
	if (not uarts.empty()) ss << "  UARTs: " << uarts << std::endl;

	/*******************************************************************
	 * Per-channel info
	 ******************************************************************/

	for (size_t chan = 0; chan < numRxChans; chan++)
	{
		std::unique_ptr<SdrDeviceChannel> pchannel(new SdrDeviceChannel(soapyDevice, SOAPY_SDR_RX, chan));
		ss << pchannel->probeChannel();
		rx_channels.push_back(std::move(pchannel));
	}
	for (size_t chan = 0; chan < numTxChans; chan++)
	{
		std::unique_ptr<SdrDeviceChannel> pchannel(new SdrDeviceChannel(soapyDevice, SOAPY_SDR_TX, chan));
		ss <<  pchannel->probeChannel();
		tx_channels.push_back(std::move(pchannel));
	}
	return ss.str();
}

SdrDeviceChannel::SdrDeviceChannel(SoapySDR::Device	*dev, int direction, int channel_no)
{
	soapyDevice = dev;
	dir = direction;
	chan = channel_no;
}

SdrDeviceChannel::~SdrDeviceChannel()
{
}

std::string SdrDeviceChannel::probeChannel()
{
	std::stringstream ss;
	std::string dirName = (dir == SOAPY_SDR_TX) ? "TX" : "RX";
	ss << std::endl;
	ss << "----------------------------------------------------" << std::endl;
	ss << "-- " << dirName << " Channel " << chan << std::endl;
	ss << "----------------------------------------------------" << std::endl;

	// info
	const auto info = soapyDevice->getChannelInfo(dir, chan);
	if (info.size() > 0)
	{
		ss << "  Channel Information:" << std::endl;
		for (const auto &it : info)
		{
			ss << "    " << it.first << "=" << it.second << std::endl;
		}
	}
	
	full_duplex = soapyDevice->getFullDuplex(dir, chan);
	agc = soapyDevice->hasGainMode(dir, chan);
		
	ss << "  Full-duplex: " << (soapyDevice->getFullDuplex(dir, chan) ? "YES" : "NO") << std::endl;
	ss << "  Supports AGC: " << (soapyDevice->hasGainMode(dir, chan) ? "YES" : "NO") << std::endl;

	//formats
	formats = toString(soapyDevice->getStreamFormats(dir, chan));
	if (not formats.empty()) ss << "  Stream formats: " << formats << std::endl;
	
	//native
	Native_format = soapyDevice->getNativeStreamFormat(dir, chan, fullScale);
	ss << "  Native format: " << Native_format << " [full-scale=" << fullScale << "]" << std::endl;    
	
	//stream args
	streamArgs = toString(soapyDevice->getStreamArgsInfo(dir, chan));
	if (not streamArgs.empty()) ss << "  Stream args:" << std::endl << streamArgs;

	//antennas
	antennas = soapyDevice->listAntennas(dir, chan);
	for (auto col : antennas)
	{
		ss << "  Antennas: " << col << std::endl;
	}
	
	//corrections
	if (soapyDevice->hasDCOffsetMode(dir, chan)) correctionsList.push_back("DC removal");
	if (soapyDevice->hasDCOffset(dir, chan)) correctionsList.push_back("DC offset");
	if (soapyDevice->hasIQBalance(dir, chan)) correctionsList.push_back("IQ balance");
	std::string corrections = toString(correctionsList);
	if (not corrections.empty()) ss << "  Corrections: " << corrections << std::endl;
	
	//gains
	full_gain_range = soapyDevice->getGainRange(dir, chan);
	ss << "  Full gain range: " << toString(full_gain_range) << " dB" << std::endl;
		
	gainsList = soapyDevice->listGains(dir, chan);
	for (size_t i = 0; i < gainsList.size(); i++)
	{
		SoapySDR::Range r = soapyDevice->getGainRange(dir, chan, gainsList[i]);
		gain_range.push_back(r);
		ss << "    " << gainsList[i] << " gain range: " << toString(r) << " dB" << std::endl;
	}
	
	//frequencies
	full_frequency_range = soapyDevice->getFrequencyRange(dir, chan);
	ss << "  Full freq range: " << toString(full_frequency_range, 1e6) << " MHz" << std::endl;
	
	freqsList = soapyDevice->listFrequencies(dir, chan);
	for (size_t i = 0; i < freqsList.size(); i++)
	{
		const std::string name = freqsList[i];
		SoapySDR::RangeList rl = soapyDevice->getFrequencyRange(dir, chan, freqsList[i]);
		frequency_range.push_back(rl);
		ss << "    " << name << " freq range: " << toString(rl, 1e6) << " MHz" << std::endl;
	}

	//freq args
	freqArgs = toString(soapyDevice->getFrequencyArgsInfo(dir, chan));
	if (not freqArgs.empty()) ss << "  Tune args:" << std::endl << freqArgs;

	//rates
	sample_range = soapyDevice->getSampleRateRange(dir, chan);
	ss << "  Sample rates range: " << toString(sample_range, 1e6) << " MSps" << std::endl;

	//rates
	sample_rates = soapyDevice->listSampleRates(dir, chan);
	if (not sample_rates.empty()) ss << "  Sample rates: " << toString(sample_rates, 1e6) << " MSps" << std::endl;
	
	//bandwidths
	bandwidth_range = soapyDevice->getBandwidthRange(dir, chan);
	if (not bandwidth_range.empty()) ss << "  Filter bandwidths: " << toString(bandwidth_range, 1e6) << " MHz" << std::endl;
	
	//sensors
	sensors = toString(soapyDevice->listSensors(dir, chan));
	if (not sensors.empty()) ss << "  Sensors: " << sensors << std::endl;
	ss << channelSensorReadings(soapyDevice, dir, chan);

	//settings
	settings = toString(soapyDevice->getSettingInfo(dir, chan));
	if (not settings.empty()) ss << "  Other Settings:" << std::endl << settings;

	return ss.str();
}

static sig_atomic_t loopDone = false;
static void sigIntHandler(const int)
{
	loopDone = true;
}

void runRateTestStreamLoop(
    SoapySDR::Device *device,
	SoapySDR::Stream *stream,
	const int direction,
	const size_t numChans,
	const size_t elemSize)
{
	//allocate buffers for the stream read/write
	const size_t numElems = device->getStreamMTU(stream) * 2;
	std::vector<std::vector<char>> buffMem(numChans, std::vector<char>(elemSize*numElems));
	std::vector<void *> buffs(numChans);
	for (size_t i = 0; i < numChans; i++) buffs[i] = buffMem[i].data();

	//state collected in this loop
	unsigned int overflows(0);
	unsigned int underflows(0);
	unsigned long long totalSamples(0);
	const auto startTime = std::chrono::high_resolution_clock::now();
	auto timeLastPrint = std::chrono::high_resolution_clock::now();
	auto timeLastSpin = std::chrono::high_resolution_clock::now();
	auto timeLastStatus = std::chrono::high_resolution_clock::now();
	int spinIndex(0);

	std::cout << "Starting stream loop, press Ctrl+C to exit..." << std::endl;
	device->activateStream(stream);
	signal(SIGINT, sigIntHandler);
	while (not loopDone)
	{
		int ret(0);
		int flags(0);
		long long timeNs(0);
		switch (direction)
		{
		case SOAPY_SDR_RX:
			ret = device->readStream(stream, buffs.data(), numElems, flags, timeNs);
			break;
		case SOAPY_SDR_TX:
			ret = device->writeStream(stream, buffs.data(), numElems, flags, timeNs);
			break;
		}

		if (ret == SOAPY_SDR_TIMEOUT) continue;
		if (ret == SOAPY_SDR_OVERFLOW)
		{
			overflows++;
			continue;
		}
		if (ret == SOAPY_SDR_UNDERFLOW)
		{
			underflows++;
			continue;
		}
		if (ret < 0)
		{
			std::cerr << "Unexpected stream error " << SoapySDR::errToStr(ret) << std::endl;
			break;
		}
		totalSamples += ret;

		const auto now = std::chrono::high_resolution_clock::now();
		if (timeLastSpin + std::chrono::milliseconds(300) < now)
		{
			timeLastSpin = now;
			static const char spin[] = { "|/-\\" };
			printf("\b%c", spin[(spinIndex++) % 4]);
			fflush(stdout);
		}
		//occasionally read out the stream status (non blocking)
		if (timeLastStatus + std::chrono::seconds(1) < now)
		{
			timeLastStatus = now;
			while (true)
			{
				size_t chanMask; int flags; long long timeNs;
				ret = device->readStreamStatus(stream, chanMask, flags, timeNs, 0);
				if (ret == SOAPY_SDR_OVERFLOW) overflows++;
				else if (ret == SOAPY_SDR_UNDERFLOW) underflows++;
				else if (ret == SOAPY_SDR_TIME_ERROR) {}
				else break;
			}
		}
		if (timeLastPrint + std::chrono::seconds(5) < now)
		{
			timeLastPrint = now;
			const auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);
			const auto sampleRate = double(totalSamples) / timePassed.count();
			printf("\b%g Msps\t%g MBps", sampleRate, sampleRate*numChans*elemSize);
			if (overflows != 0) printf("\tOverflows %u", overflows);
			if (underflows != 0) printf("\tUnderflows %u", underflows);
			printf("\n ");
		}

	}
	device->deactivateStream(stream);
}

int SdrDevice::SoapySDRRateTest(
    const std::string &argStr,
	const double sampleRate,
	const std::string &channelStr,
	const std::string &directionStr)
{
	try
	{
		//parse the direction to the integer enum
		int direction(-1);
		if (directionStr == "RX" or directionStr == "rx") direction = SOAPY_SDR_RX;
		if (directionStr == "TX" or directionStr == "tx") direction = SOAPY_SDR_TX;
		if (direction == -1) throw std::invalid_argument("direction not in RX/TX: " + directionStr);

		//build channels list, using KwargsFromString is a easy parsing hack
		std::vector<size_t> channels;
		for (const auto &pair : SoapySDR::KwargsFromString(channelStr))
		{
			channels.push_back(std::stoi(pair.first));
		}
		if (channels.empty()) channels.push_back(0);

		double gain = 88;
		//initialize the sample rate for all channels
		for (const auto &it : soapyDevice->getHardwareInfo())
		{
			std::cout << "  " << it.first << "=" << it.second << std::endl;
		}
	    
		for (const auto &chan : channels)
		{
			soapyDevice->setSampleRate(direction, chan, sampleRate);
			soapyDevice->setGain(direction, chan, gain); 
			soapyDevice->setFrequency(SOAPY_SDR_RX, 0, 3600000.0);
		}

		//create the stream, use the native format
		double fullScale(0.0);
		const auto format = soapyDevice->getNativeStreamFormat(direction, channels.front(), fullScale);
		const size_t elemSize = SoapySDR::formatToSize(format);
		auto stream = soapyDevice->setupStream(direction, format, channels);

		//run the rate test one setup is complete
		std::cout << "Stream format: " << format << std::endl;
		std::cout << "Num channels: " << channels.size() << std::endl;
		std::cout << "Element size: " << elemSize << " bytes" << std::endl;
		std::cout << "Begin " << directionStr << " rate test at " << (sampleRate / 1e6) << " Msps" << std::endl;
		runRateTestStreamLoop(soapyDevice, stream, direction, channels.size(), elemSize);

		//cleanup stream and device
		soapyDevice->closeStream(stream);
	}
	catch (const std::exception &ex)
	{
		std::cerr << "Error in rate test: " << ex.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_FAILURE;
}

int SdrDevice::checkDriver(const std::string &driverName)
{
	std::cout << "Loading modules... " << std::flush;
	SoapySDR::loadModules();
	std::cout << "done" << std::endl;

	std::cout << "Checking driver '" << driverName << "'... " << std::flush;
	const auto factories = SoapySDR::Registry::listFindFunctions();

	if (factories.find(driverName) == factories.end())
	{
		std::cout << "MISSING!" << std::endl;
		return EXIT_FAILURE;
	}
	else
	{
		std::cout << "PRESENT" << std::endl;
		return EXIT_SUCCESS;
	}
}

/***********************************************************************
 * Print version and module info
 **********************************************************************/
int printInfo(void)
{
	std::cout << "Lib Version: v" << SoapySDR::getLibVersion() << std::endl;
	std::cout << "API Version: v" << SoapySDR::getAPIVersion() << std::endl;
	std::cout << "ABI Version: v" << SoapySDR::getABIVersion() << std::endl;
	std::cout << "Install root: " << SoapySDR::getRootPath() << std::endl;

	//max path length for alignment
	size_t maxPathLen(0);
	const auto searchPaths = SoapySDR::listSearchPaths();
	const auto modules = SoapySDR::listModules();
	for (const auto &path : searchPaths) maxPathLen = std::max(maxPathLen, path.size());
	for (const auto &mod : modules) maxPathLen = std::max(maxPathLen, mod.size());

	//print search path information
	for (const auto &path : searchPaths)
	{
		struct stat info;
		const bool missing = (stat(path.c_str(), &info) != 0);
		std::cout << "Search path:  " << path;
		if (missing) std::cout << std::string(maxPathLen - path.size(), ' ') << " (missing)";
		std::cout << std::endl;
	}

	//load each module and print information
	for (const auto &mod : modules)
	{
		std::cout << "Module found: " << mod;
		const auto &errMsg = SoapySDR::loadModule(mod);
		if (not errMsg.empty()) std::cout << "\n  " << errMsg;
		const auto version = SoapySDR::getModuleVersion(mod);
		if (not version.empty()) std::cout << std::string(maxPathLen - mod.size(), ' ') << " (" << version << ")";
		std::cout << std::endl;
	}
	if (modules.empty()) std::cout << "No modules found!" << std::endl;

	std::cout << "Available factories... ";
	std::string factories;
	for (const auto &it : SoapySDR::Registry::listFindFunctions())
	{
		if (not factories.empty()) factories += ", ";
		factories += it.first;
	}
	if (factories.empty()) factories = "No factories found!";
	std::cout << factories << std::endl;

	std::cout << "Available converters..." << std::endl;
	for (const auto &source : SoapySDR::ConverterRegistry::listAvailableSourceFormats())
	{
		std::string targets;
		for (const auto &target : SoapySDR::ConverterRegistry::listTargetFormats(source))
		{
			if (not targets.empty()) targets += ", ";
			targets += target;
		}
		std::cout << " - " << std::setw(5) << source << " -> [" << targets << "]" << std::endl;
	}

	return EXIT_SUCCESS;
}

int SdrDeviceVector::get_device_index(std::string dev)
{
	auto it = dev_map.find(dev);
	if (it != dev_map.end())
	{
		return it->second - 1; // make it zero based
	}
	return 0;
}

void SdrDeviceVector::AddDevice(std::string name, std::string probe)
{
	int i = dev_map.size() + 1;
	dev_map.insert(std::pair<std::string,int>(name,i));
	if (SdrDevice::checkDriver(name) == EXIT_SUCCESS)
	{
		std::unique_ptr<SdrDevice> device(new SdrDevice(probe));
		SdrDevices.insert(std::pair<std::string, std::unique_ptr<SdrDevice>>(name, std::move(device)));
	}
}

bool SdrDeviceVector::MakeDevice(std::string name)
{
	auto it = SdrDevices.find(name);
	if (it != SdrDevices.end())
	{ 
		if (SdrDevices[name]->MakeSdrDevice() == EXIT_SUCCESS)
			return true;
	}
	return false;
}

int	SdrDeviceVector::get_tx_channels(std::string name)
{
	auto it = SdrDevices.find(name);
	if (it != SdrDevices.end())
	{ 
		return SdrDevices[name]->get_txchannels();
	}
	return 0;
}

int SdrDeviceVector::get_rx_channels(std::string name)
{
	auto it = SdrDevices.find(name);
	if (it != SdrDevices.end())
	{
		return SdrDevices[name]->get_rxchannels();
	}
	return -1;

}

SoapySDR::RangeList SdrDeviceVector::get_full_frequency_range_list(std::string name, int chan) 
{
	SoapySDR::RangeList r;

	if (get_rx_channels(name) > chan)
		return SdrDevices[name]->rx_channels[chan]->get_full_frequency_range_list();

	if (get_tx_channels(name) > chan)
		return SdrDevices[name]->tx_channels[chan]->get_full_frequency_range_list();
	return r;
}


bool SdrDeviceVector::isValid(std::string name)
{
	auto it = SdrDevices.find(name);
	if (it != SdrDevices.end())
	{ 
		return true;
	}
	return false;
}

bool SdrDeviceVector::UnMakeDevice(std::string name)
{
	auto it = SdrDevices.find(name);
	if (it != SdrDevices.end())
	{ 
		if (SdrDevices[name]->UnmakeSdrDevice() == EXIT_SUCCESS)
			return true;
	}
	return false;
}