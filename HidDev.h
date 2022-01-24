#pragma once
#include <linux/input.h>
#include <cstdio>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <chrono>
#include "Settings.h"
#include "sdrberry.h"
#include "vfo.h"

class HidDev
{
  public:
	void init(string input_dev);
	HidDev();
	~HidDev();
	void step_vfo();

  private:
	int			m_fd;
	int			numIndex;
	char		HidDevName[80];
	bool		bstep;
	input_event in_event;
	int			step;
	int			value;
	int			last_val;
	int			speed;
	int			status;
	std::chrono::high_resolution_clock::time_point last_time;

	bool		read_event();
	string		HidName;
};

