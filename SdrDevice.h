#pragma once
#include <atomic>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <SoapySDR/ConverterRegistry.hpp>
#include <SoapySDR/Device.hpp>
#include <SoapySDR/Errors.hpp>
#include <SoapySDR/Formats.hpp>
#include <SoapySDR/Modules.hpp>
#include <SoapySDR/Registry.hpp>
#include <SoapySDR/Types.hpp>
#include <SoapySDR/Version.hpp>

class SdrDeviceChannel
{
  public:
	SdrDeviceChannel(SoapySDR::Device *dev, int dir, int no);
	~SdrDeviceChannel();
	SoapySDR::Range get_full_frequency_range() { return full_frequency_range.front(); }
	SoapySDR::RangeList get_full_frequency_range_list() { return full_frequency_range; }
	std::vector<double> get_sample_rates() { return sample_rates; }
	std::string probeChannel();
	SoapySDR::Range get_full_gain_range() { return full_gain_range; }
	bool get_agc() { return agc; }
	int get_bandwith_count() { return bandwidth_range.size(); }
	long get_bandwith(int no) { return bandwidth_range[no].minimum(); }
	std::vector<std::string> get_antennas() { return antennas;}

  private:
	SoapySDR::Device *soapyDevice{nullptr};

	int dir{0};
	int chan{0};

	bool full_duplex{false};
	bool agc{false};
	std::string formats;
	double fullScale{0.0};
	std::string Native_format;
	std::string streamArgs;
	std::vector <std::string> antennas;
	std::vector<std::string> correctionsList;
	SoapySDR::Range full_gain_range;
	std::vector<std::string> gainsList;
	std::vector<SoapySDR::Range> gain_range;
	SoapySDR::RangeList full_frequency_range;
	std::vector<std::string> freqsList;
	std::vector<SoapySDR::RangeList> frequency_range;
	std::string freqArgs;
	SoapySDR::RangeList sample_range;
	std::vector<double> sample_rates;
	SoapySDR::RangeList bandwidth_range;
	std::string sensors;
	std::string settings;
};

class SdrDevice
{
  public:
	SdrDevice(std::string driver);
	~SdrDevice();
	int SoapySDRRateTest(const std::string &argStr, const double sampleRate, const std::string &channelStr, const std::string &directionStr);
	static int checkDriver(const std::string &driverName);
	int MakeSdrDevice();
	int UnmakeSdrDevice();
	bool isDeviceActive() { return (soapyDevice != nullptr); }
	std::vector<double> get_rx_sample_rates(int channel) { return rx_channels.at(channel)->get_sample_rates(); }
	std::vector<double> get_tx_sample_rates(int channel) { return tx_channels.at(channel)->get_sample_rates(); }
	int get_txchannels() { return numTxChans; }
	int get_rxchannels() { return numRxChans; }
	int get_bandwith_count(int channel) { return rx_channels.at(channel)->get_bandwith_count(); }
	long get_bandwith(int channel, int no) { return rx_channels.at(channel)->get_bandwith(no); }
	std::vector<std::string> get_antennas(int channel) { return rx_channels.at(channel)->get_antennas(); }
	size_t getStreamMTU(SoapySDR::Stream *stream)
	{
		return soapyDevice->getStreamMTU(stream);
	}

	void setFrequency(const int direction, const size_t channel, const double frequency)
	{
		if (direction == SOAPY_SDR_TX && numTxChans < 1)
			return;
		return soapyDevice->setFrequency(direction, channel, frequency);
	}

	void setSampleRate(const int direction, const size_t channel, const double rate)
	{
		if (direction == SOAPY_SDR_TX && numTxChans < 1)
			return;
		return soapyDevice->setSampleRate(direction, channel, rate);
	}

	double getSampleRate(const int direction, const size_t channel) const
	{
		if (direction == SOAPY_SDR_TX && numTxChans < 1)
			return 0.0;
		return soapyDevice->getSampleRate(direction, channel);
	}

	void setBandwidth(const int direction, const size_t channel, const double bw)
	{
		if (direction == SOAPY_SDR_TX && numTxChans < 1)
			return;
		return soapyDevice->setBandwidth(direction, channel, bw);
	}
		
	void setGain(const int direction, const size_t channel, const double value)
	{
		if (direction == SOAPY_SDR_TX && numTxChans < 1)
			return;
		if (direction == SOAPY_SDR_RX)
		{
			if (rx_channels.at(channel)->get_agc())
			{
				if (soapyDevice->getGainMode(direction, channel))
					return;
			}
		}
		return soapyDevice->setGain(direction, channel, value);
	}

	void setGain(const int direction, const size_t channel, const std::string &name, const double value)
	{
		if (direction == SOAPY_SDR_TX && numTxChans < 1)
			return;
		if (direction == SOAPY_SDR_RX)
		{
			if (rx_channels.at(channel)->get_agc())
			{
				if (soapyDevice->getGainMode(direction, channel))
					return;
			}
		}

		return soapyDevice->setGain(direction, channel, name, value);
	}

	bool hasGainMode(const int direction, const size_t channel) const
	{
		return soapyDevice->hasGainMode(direction, channel);
	}

	void setGainMode(const int direction, const size_t channel, const bool automatic)
	{
		return soapyDevice->setGainMode(direction, channel, automatic);
	}

	bool getGainMode(const int direction, const size_t channel) const
	{
		return soapyDevice->getGainMode(direction, channel);
	}

	SoapySDR::Stream *setupStream(const int direction, const std::string &format, const std::vector<size_t> &channels = std::vector<size_t>(), const SoapySDR::Kwargs &args = SoapySDR::Kwargs())
	{
		return soapyDevice->setupStream(direction, format, channels, args);
	}

	int activateStream(SoapySDR::Stream *stream, const int flags = 0, const long long timeNs = 0, const size_t numElems = 0)
	{
		return soapyDevice->activateStream(stream, flags, timeNs, numElems);
	}

	int readStream(SoapySDR::Stream *stream, void *const *buffs, const size_t numElems, int &flags, long long &timeNs, const long timeoutUs = 100000)
	{
		return soapyDevice->readStream(stream, buffs, numElems, flags, timeNs, timeoutUs);
	}

	int readStreamStatus(SoapySDR::Stream *stream, size_t &chanMask, int &flags, long long &timeNs, const long timeoutUs = 100000)
	{
		return soapyDevice->readStreamStatus(stream, chanMask, flags, timeNs, timeoutUs);
	}

	int deactivateStream(SoapySDR::Stream *stream, const int flags = 0, const long long timeNs = 0)
	{
		return soapyDevice->deactivateStream(stream, flags, timeNs);
	}

	void closeStream(SoapySDR::Stream *stream)
	{
		return soapyDevice->closeStream(stream);
	}

	void setAntenna(const int direction, const size_t channel, const std::string &name)
	{
		if (direction == SOAPY_SDR_TX && numTxChans < 1)
			return;
		return soapyDevice->setAntenna(direction, channel, name);
	}

	int writeStream(SoapySDR::Stream *stream, const void *const *buffs, const size_t numElems, int &flags, const long long timeNs = 0, const long timeoutUs = 100000)
	{
		return soapyDevice->writeStream(stream, buffs, numElems, flags, timeNs, timeoutUs);
	}

	bool hasFullDuplex(const int direction, const size_t channel) const
	{
		return soapyDevice->getFullDuplex(direction, channel);
	}

	std::vector<std::unique_ptr<SdrDeviceChannel>> rx_channels;
	std::vector<std::unique_ptr<SdrDeviceChannel>> tx_channels;

  private:
	std::string driver;
	int active_channel;
	SoapySDR::Device *soapyDevice;
	int makeDevice();
	std::string SoapySDRDeviceProbe();
	std::string driver_name, hardware;
	std::vector<std::string> information;
	size_t numRxChans{0};
	size_t numTxChans{0};
	bool bTimestamps{false};
	std::string clockSources;
	std::string timeSources;
	std::string sensors;
	std::string registers;
	std::string settings;
	std::string gpios;
	std::string uarts;
};

class SdrDeviceVector
{
  public:
	void AddDevice(std::string name, std::string probe);
	bool MakeDevice(std::string);
	SoapySDR::Range get_full_tx_frequency_range(std::string name, int chan) { return SdrDevices[name]->tx_channels.at(chan)->get_full_frequency_range(); }
	SoapySDR::Range get_full_frequency_range(std::string name, int chan) { return SdrDevices[name]->rx_channels.at(chan)->get_full_frequency_range(); }
	SoapySDR::RangeList get_full_frequency_range_list(std::string name, int chan);
	std::map<std::string, std::unique_ptr<SdrDevice>> SdrDevices;
	int get_tx_channels(std::string name);
	int get_rx_channels(std::string name);
	bool isValid(std::string name);
	int get_device_index(std::string dev);
	bool UnMakeDevice(std::string name);

  private:
	std::map<std::string, int> dev_map;
};