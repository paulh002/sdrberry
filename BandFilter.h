#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <stdint.h>
#include <unistd.h>
#include <memory>
#include <variant>
#include "vfo.h"
#include "Settings.h"
#include "PCF8574.h"
#include "TCA9548.h"

class BandFilter
{
public:
	void initFilter();
	void SetBand(int band, bool rx);
	void Setpasstrough(bool b);

  private:
	std::vector<std::variant<PCF8574, TCA9548V2>> i2cDevices;
	bool	bandfilter_pass_trough{false};
};

extern BandFilter bpf;