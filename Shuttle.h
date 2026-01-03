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
	unsigned char button_a, button_b;
	bool debugflag;
	std::chrono::high_resolution_clock::time_point last_time;
	void decode_buttons(unsigned char a, unsigned char b);

  public:
	Shuttle();
	void start();
	void step_vfo();
};

