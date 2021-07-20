#pragma once

#include <SoapySDR/Version.hpp>
#include <SoapySDR/Modules.hpp>
#include <SoapySDR/Registry.hpp>
#include <SoapySDR/Device.hpp>
#include <SoapySDR/ConverterRegistry.hpp>
#include <SoapySDR/Types.hpp>
#include <SoapySDR/Formats.hpp>
#include "wstring.h"
#include "DataBuffer.h"
#include "SoftFM.h"

extern SoapySDR::Device *sdr;
extern String device_name;	

int discover_devices();

#define MAX_NUM_DEVICES			10
#define MAX_NUM_RANGES			20
#define MAX_NUM_CHANNELS		4
#define MAX_NUM_GAINS			4
#define MAX_NUM_FRQ_RANGES		4
#define	MAX_NUM_BANDWIDTHS		20


extern	int num_devices;

struct channel_structure {
	String	information[20];
	bool	full_duplex;
	bool	agc;
	String	stream_formats[MAX_NUM_RANGES];
	String	antennas;
	double	full_gain_range[2];
	String	sGains[MAX_NUM_GAINS];
	double	gain_range[MAX_NUM_GAINS][2];
	double	fullScale;
	String	Native_format;
	String	streamArgs;
	double	full_frequency_range[2];
	double	frequency_range[MAX_NUM_FRQ_RANGES][2];
	double	sample_range[2];
	double	bandwidth_range[MAX_NUM_BANDWIDTHS][2];
	int		bandwidth_range_count;
	DataBuffer<IQSample> *source_buffer;
};

struct device_structure {
	int		device_no;
	String	driver;
	String	hardware;
	String	information[MAX_NUM_RANGES];
	String	firmware_version;
	String	hardware_verion;
	String	gateware_version;
	String	protocol_version;
	double	gain;
	int		channels_rx;
	int		channels_tx;
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