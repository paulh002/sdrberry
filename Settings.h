#pragma once

#include <cstdio>

#include <iostream>
#include <iterator>
#include <map>
#include "configfile.h"
#include "wstring.h"

using namespace std;

class Settings
{
public:
	void read_settings(String settings_file);
	void init_settings(String settings_file);
	String get_mac_address() {return mac_address; };
	String find_audio(string key);
	String find_radio(string key);
	String find_probe(string key);
	String find_sdr(string key);
private:
	
	cfg::File				*config;
	String					mac_address;
	map<string, string>		sdr;
	map<string, string>		radio;
	map<string, string>		probes;
	map<string, string>		vfo1;
	map<string, string>		vfo2;
	map<string, string>		audio;
	
	vector<int>				meters;
	vector<string>			labels;
	vector<long>			f_low;
	vector<long>			f_high;
	
	
};


extern Settings	Settings_file;
