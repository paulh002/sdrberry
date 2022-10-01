#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <stdint.h>
#include <unistd.h>
#include <memory>
#include "vfo.h"
#include "Settings.h"
#include "PCF8574.h"

class BandFilter
{
public:
	void initFilter();
	void SetBand(int band, bool rx);
	void Setpasstrough(bool b);

  private:
	std::vector<PCF8574>	pcf8574;
	bool	connected;
	bool	bandfilter_pass_trough{false};
};

extern BandFilter bpf;