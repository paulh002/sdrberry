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

using namespace std;

class Settings
{
  public:
	void read_settings(string settings_file);
	void init_settings(string settings_file);
	void write_settings();
	string get_mac_address() { return mac_address; }
	string find_audio(string key);
	string find_radio(string key);
	string find_probe(string key);
	string find_sdr(string key);
	long long find_vfo1_freq(string key);
	int volume();
	int gain();
	int txgain();
	int drive();
	int getagc(string key);
	int getspeech(string key);
	void set_drive(int drive);
	void set_txgain(int gain);
	void set_gain(int gain);
	string find_input(string key);
	string find_cat(string key);
	int convert_mode(string s);
	long long get_ft8(int band);
	void getagc_preset(std::string key, int &atack, int &release);
	void getspeech_preset(std::string key, int &atack, int &release);
	void save_speech(std::string key, int value);
	void save();
	void save_vol(int vol);
	void save_rf(int rf);
	void save_vfo(int vfo, long freq);
	int gain(string sdrdevice);
	int get_int(string section, string key, int defaultValue = 0);
	long long get_longlong(string section, string key, long defaultValue = 0L);
	string get_string(string sdrdevice, string key);
	void save_int(string section, string key, int value);
	void save_string(string section, string key, string value);
	void get_array_long(std::string section, std::string key, vector<long> &array);
	void set_array_long(std::string section, std::string key, vector<long> &array);
	void set_array_int(std::string section, std::string key, vector<int> &array);
	void get_array_int(string section, std::string key, vector<int> &array);
	std::vector<std::string> get_array_string(std::string section, std::string key);
	void set_array_string(std::string section, std::string key, const vector<std::string> &array);

	vector<int> meters;
	vector<string> labels;
	vector<long> f_low;
	vector<long> f_high;
	vector<long> ft8_freq;
	vector<string> mode;
	vector<string> receivers;

  private:
	void default_settings();
	cfg::File *config;
	string file;
	string mac_address;
	map<string, string> sdr;
	map<string, string> radio;
	map<string, string> probes;
	map<string, string> audio;
	map<string, string> input_dev;
	map<string, string> cat;
	map<string, string> agc;
	map<string, string> speech;
};

extern Settings Settings_file;
