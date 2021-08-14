#pragma once
#include <cstdio>
#include <iostream>
#include <iterator>
#include <string>
#include <map>
#include "configfile.h"

using namespace std;

class Settings
{
public:
	void read_settings(string settings_file);
	void init_settings(string settings_file);
	string get_mac_address() {return mac_address; };
	string find_audio(string key);
	string find_radio(string key);
	string find_probe(string key);
	string find_sdr(string key);
	long long find_vfo1_freq(string key);
	string	find_vfo1(string key);
	double	find_samplerate(string key);
	int		volume();
	int		gain();
	
	vector<int>				meters;
	vector<string>			labels;
	vector<long>			f_low;
	vector<long>			f_high;
	
private:
	
	cfg::File				*config;
	string					mac_address;
	map<string, string>		sdr;
	map<string, string>		radio;
	map<string, string>		probes;
	map<string, string>		vfo1;
	map<string, string>		vfo2;
	map<string, string>		audio;
	map<string, string>		samplerate;
};


extern Settings	Settings_file;
