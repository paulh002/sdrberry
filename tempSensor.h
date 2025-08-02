#pragma once
#include "lvgl_.h"
#include <string>
#include <vector>
#include <atomic>
#include <thread>
#include <array>
#include <memory>

#define TEMP_DISCOVERY_PATH  "/sys/bus/w1/devices/"
#define TEMP_DISCOVERY_PATH_BUF_SIZE 132
#define TEMP_IDENTIFICATION "28"
#define MAX_AMOUNT_SENSORS 4

inline const int read_sleep_time = 100;

class tempSensor
{
  private:
	lv_timer_t *timer{NULL};
	std::vector<std::string> tempsensors;
	std::atomic<bool> stop_flag{false};
	std::array<std::atomic<int>, MAX_AMOUNT_SENSORS> temperature;

	bool discover_tempSensors();
	bool open_tempsensor(char *filename);
	int temp_discovery_stop(void);

  public:
	tempSensor();
	static int count_tempsensors();
	int read_tempsensor(int sensor);
	void operator()();

	inline static std::shared_ptr<tempSensor> ptr;
	static void start_read_out();
	void stop_read_out();
	inline static std::thread read_out_thread;
	int get_temperature_readout(int sensor);
	static int get_temperature(int sensor);
};

