#pragma once
#include <cstdio>
#include <iostream>
#include <iterator>
#include <string>
#include <map>
#include "configfile.h"
#include "sdrberry.h"


using namespace std;

class Settings
{
public:
	void read_settings(string settings_file);
	void init_settings(string settings_file);
	void write_settings();
	string get_mac_address() {return mac_address; }
	;
	string find_audio(string key);
	string find_radio(string key);
	string find_probe(string key);
	string find_sdr(string key);
	long long find_vfo1_freq(string key);
	string	find_vfo1(string key);
	string	find_vfo2(string key);
	double	find_samplerate(string key);
	double	find_samplerate_tx(string key);
	int		volume();
	int		gain();
	int		txgain();
	int		micgain();
	int		drive();
	int		agc_delay();
	int		agc_slope();
	int		agc_threshold();
	int		agc_mode();
	void	set_drive(int drive);
	void	set_micgain(int gain);
	void	set_txgain(int gain);
	void	set_gain(int gain);
	string	find_input(string key);
	string	find_cat(string key);
	int		convert_mode(string s);
		
	vector<int>				meters;
	vector<string>			labels;
	vector<long>			f_low;
	vector<long>			f_high;
	vector<int>				address;
	vector<int>				command_rx;
	vector<int>				command_tx;
	vector<string>			mode;
	vector <string>			receivers;
	
private:
	
	cfg::File				*config;
	string					file;
	string					mac_address;
	map<string, string>		sdr;
	map<string, string>		radio;
	map<string, string>		probes;
	map<string, string>		vfo1;
	map<string, string>		vfo2;
	map<string, string>		audio;
	map<string, string>		samplerate;
	map<string, string>		samplerate_tx;
	map<string, string>		input_dev;
	map<string, string>		cat;
	map<string, string>		agc;
};


extern Settings	Settings_file;
