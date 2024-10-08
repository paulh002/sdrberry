#pragma once
#include <linux/input.h>
#include <linux/hiddev.h>
#include <cstdio>
#include <string>
#include <time.h>
#include <sys/time.h>
#include <chrono>
#include "lvgl_.h"

class HidDev
{
  public:
	void init(std::string input_dev);
	HidDev();
	~HidDev();
	void step_vfo();

	lv_indev_state_t encoder_key_press();
	int encoder_rotate();

  private:
	int m_fd;
	int numIndex;
	char HidDevName[80];
	bool bstep;
	input_event in_event;
	hiddev_event hid_event;
	int step;
	int value;
	int last_val{0};
	int speed;
	bool encoder{false};
	std::chrono::high_resolution_clock::time_point last_time;
	bool enc_key_pressed{false};
	int enc_moved{0};
	bool enc_pressed{false};
	void rotate(int value);
	void rotate_vfo();
	bool read_event();
	std::string HidName;
	bool txstate{false};
	bool usb_hid;
	float fspeed;
};

