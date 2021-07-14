#pragma once

#include <SoapySDR/Device.hpp>
#include <SoapySDR/Types.hpp>
#include <SoapySDR/Formats.hpp>
#include "wstring.h"

extern SoapySDR::Device *sdr;
extern String device_name;	

int discover_devices();