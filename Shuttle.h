#pragma once
#include "HIDDeviceMonitor.h"
#include <memory>
#include <chrono>

class Shuttle
{
  private:
	std::unique_ptr<HIDDeviceMonitor> device;
	int step;
	int value;
	uint8_t wheel;
	std::chrono::high_resolution_clock::time_point last_time;

  public:
	Shuttle();
	void start();
	void step_vfo();
};

