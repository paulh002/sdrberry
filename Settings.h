#pragma once
#include "Modes.h"
#include "configfile.h"
#include <algorithm>
#include <cstdio>
#include <ctype.h>
#include <iostream>
#include <iterator>
#include <map>
#include <string>
#include <vector>

class Settings
{
  public:
	void read_settings(std::string settings_file);
	void init_settings(std::string settings_file);
	void write_settings();
	std::string get_mac_address() { return mac_address; }
	std::string find_audio(std::string key);
	std::string find_radio(std::string key);
	std::string find_probe(std::string key);
	std::string find_sdr(std::string key);
	long long find_vfo1_freq(std::string key);
	int volume();
	int txgain();
	int getagc(std::string key);
	int getspeech(std::string key);
	void set_txgain(int gain);
	std::string find_input(std::string key);
	std::string find_cat(std::string key);
	int convert_mode(std::string s);
	long long get_ft8(int band);
	void getagc_preset(std::string key, int &atack, int &release);
	void getspeech_preset(std::string key, int &atack, int &release);
	void save_speech(std::string key, int value);
	void save();
	void save_vol(int vol);
	void save_vfo(int vfo, long freq);
	int gain(std::string sdrdevice);
	int get_int(std::string section, std::string key, int defaultValue = 0);
	long long get_longlong(std::string section, std::string key, long defaultValue = 0L);
	std::string get_string(std::string sdrdevice, std::string key);
	std::string get_string(std::string sdrdevice, std::string key, std::string default_string);
	void save_int(std::string section, std::string key, int value);
	void save_string(std::string section, std::string key, std::string value);
	void get_array_long(std::string section, std::string key, std::vector<long> &array);
	void set_array_long(std::string section, std::string key, std::vector<long> &array);
	void set_array_int(std::string section, std::string key, std::vector<int> &array);
	void get_array_int(std::string section, std::string key, std::vector<int> &array);
	std::vector<std::string> get_array_string(std::string section, std::string key);
	void set_array_string(std::string section, std::string key, const std::vector<std::string> &array);
	void save_band();
	void set_map_string(std::string section, std::string key, const std::map<std::string, int> &value);
	std::map<int, std::pair<std::string, long>> get_map_string_pair(std::string section, std::string key);
	void set_map_string_pair(std::string section, std::string key, const std::map<int, std::pair<std::string, long>> &value_map);
	std::map<std::string, int> get_map_string(std::string section, std::string key);
	std::map<int, std::tuple<std::string, long, std::string>> get_map_string_tuple(std::string section, std::string key);
	void set_map_string_tuple(std::string section, std::string key, const std::map<int, std::tuple<std::string, long, std::string>> &value_map);

	std::vector<int> meters;
	std::vector<std::string> labels;
	std::vector<long> f_low;
	std::vector<long> f_high;
	std::vector<long> ft8_freq;
	std::vector<std::string> mode;
	std::vector<std::string> receivers;

  private:
	void default_settings();
	cfg::File *config;
	std::string file;
	std::string mac_address;
	std::map<std::string, std::string> sdr;
	std::map<std::string, std::string> radio;
	std::map<std::string, std::string> probes;
	std::map<std::string, std::string> audio;
	std::map<std::string, std::string> input_dev;
	std::map<std::string, std::string> cat;
	std::map<std::string, std::string> agc;
	std::map<std::string, std::string> speech;
};

extern Settings Settings_file;
