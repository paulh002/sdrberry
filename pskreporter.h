#pragma once
#include <string>
#include <vector>
#include <ctime>

struct ReceptionReport
{
	std::string mode;
	std::string call;
	long frequency;
	std::string SNR;
	std::string locator;
	std::string timeUTC;
	long timeEpoch;
};

// Struct for active callsigns
struct ActiveCallsign
{
	std::string callsign;
	int reports;
	std::string DXCC;
	std::string DXCCcode;
	long frequency;
};

extern void pskreporter(std::string call, std::vector<ReceptionReport> &reports, std::vector<ActiveCallsign> &activeList);