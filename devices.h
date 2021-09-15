#pragma once

#include <SoapySDR/Version.hpp>
#include <SoapySDR/Modules.hpp>
#include <SoapySDR/Registry.hpp>
#include <SoapySDR/Device.hpp>
#include <SoapySDR/ConverterRegistry.hpp>
#include <SoapySDR/Types.hpp>
#include <SoapySDR/Formats.hpp>
#include "DataBuffer.h"
#include "sdrberry.h"

extern SoapySDR::Device *sdr;
extern string device_name;	

int discover_devices(std::string driver);
void process_mem_usage(double& vm_usage, double& resident_set);

#define MAX_NUM_DEVICES			10
#define MAX_NUM_RANGES			20
#define MAX_NUM_CHANNELS		4
#define MAX_NUM_GAINS			4
#define MAX_NUM_FRQ_RANGES		4
#define	MAX_NUM_BANDWIDTHS		20


extern	int num_devices;

// struct is initated in probeChannel function
struct channel_structure {
	string					information[20];
	bool					full_duplex;
	bool					agc;
	string					stream_formats;
	string					antennas;
	string					sGains[MAX_NUM_GAINS];
	SoapySDR::Range			gain_range[MAX_NUM_GAINS];
	SoapySDR::Range			full_gain_range;
	double					fullScale;
	string					Native_format;
	string					streamArgs;
	SoapySDR::RangeList		full_frequency_range;
	SoapySDR::RangeList		frequency_range[MAX_NUM_FRQ_RANGES];
	SoapySDR::RangeList		sample_range;
	SoapySDR::RangeList		bandwidth_range;
	pthread_t				thread;
	DataBuffer<IQSample>	*source_buffer_rx;
	DataBuffer<IQSample16>	*source_buffer_tx;
	double					gain;
};



struct device_structure {
	int		device_no;
	int		rx_channel;	// channel in use
	int		tx_channel;
	int		rx_channels; // amount of channels
	int		tx_channels;
	string	driver;
	string	hardware;
	string	information[MAX_NUM_RANGES];
	bool	bTimestamps;
	bool	tx;
	bool	rx;
	double	frequency_range_rx[MAX_NUM_RANGES];
	double	frequency_range_tx[MAX_NUM_RANGES];
	double	bandwidth_rx[MAX_NUM_RANGES];
	double	bandwidth_tx[MAX_NUM_RANGES];
	SoapySDR::Device *sdr;
	struct	channel_structure channel_structure_rx[MAX_NUM_CHANNELS];
	struct	channel_structure channel_structure_tx[MAX_NUM_CHANNELS];
};


extern struct device_structure	soapy_devices[MAX_NUM_DEVICES];