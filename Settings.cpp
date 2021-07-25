#include <cstdio>
#include <iostream>
#include "wstring.h"
#include "Settings.h"
#include "configfile.h"

using namespace std;
Settings	Settings_file;

const cfg::File::ConfigMap defaultOptions = {
	{"SDR Receivers", {
			{ "default", cfg::makeOption("pluto") },
			{ "receiver1", cfg::makeOption("pluto") },
			{ "receiver2", cfg::makeOption("radioberry") }
		} 
	}, 
	{"probes", {
	{ "pluto", cfg::makeOption("driver=plutosdr,hostname=192.168.100.1") },
	{ "radioberry", cfg::makeOption("driver=radioberry") } } 
	},
	{ "ESP32", {
	{ "mac address", cfg::makeOption("7c:9e:bd:f8:64:92") } } 
	
	},
	{"Radio", {
			{ "gain", cfg::makeOption(0,0,100) },
			{ "Volume", cfg::makeOption(0, 0, 100) },
			{ "AGC", cfg::makeOption("off") }			
		} 
	}, 
	{"VFO1", {
			{ "freq", cfg::makeOption(3500000) },
			{ "Mode", cfg::makeOption("LSB") }
		} 
	},
	{"VFO2", {
			{ "freq", cfg::makeOption(3500000) },
			{ "Mode", cfg::makeOption("LSB") }
		} 
	},
	{"Audio", {
			{ "device", cfg::makeOption("default") }
		} 
	}
};


void Settings::read_settings(String settings_file)
{
	config = new cfg::File();
	
	
	if (!config->loadFromFile((char *)settings_file.c_str()))
	{
		config->setDefaultOptions(defaultOptions);
		
		
		
		config->writeToFile(settings_file.c_str());
	}
	int i = 0;

	config->useSection("ESP32");
	mac_address = String((char *)(*config)("mac address").toString().c_str());
	
	for (auto& option : config->getSection("SDR Receivers"))
	{
		//cout << "Option name: " << option.first << endl;
		sdr.insert(pair<string, string>(option.first, option.second));
	}
	for (auto& option : config->getSection("Radio"))
	{
		//cout << "Option name: " << option.first << endl;
		radio.insert(pair<string, string>(option.first, option.second));
	}
	for (auto& option : config->getSection("probes"))
	{
		//cout << "Option name: " << option.first << endl;
		probes.insert(pair<string, string>(option.first, option.second));
	}
	for (auto& option : config->getSection("VFO1"))
	{
		//cout << "Option name: " << option.first << endl;
		vfo1.insert(pair<string, string>(option.first, option.second));
	}
	for (auto& option : config->getSection("VFO2"))
	{
		//cout << "Option name: " << option.first << endl;
		vfo2.insert(pair<string, string>(option.first, option.second));
	}
	for (auto& option : config->getSection("Audio"))
	{
		//cout << "Option name: " << option.first << endl;
		audio.insert(pair<string, string>(option.first, option.second));
	}
	config->useSection("bands");
	
	for (auto& col : (*config)("meters"))
	{
		meters.push_back(col.toInt());
	}
	for (auto& col : (*config)("labels"))
	{
		labels.push_back(col.toStringWithQuotes());
	}
	for (auto& col : (*config)("f_low"))
	{
		f_low.push_back(col.toLong());
	}
	for (auto& col : (*config)("f_high"))
	{
		f_high.push_back(col.toLong());
	}
}

String Settings::find_sdr(string key)
{
	if (sdr.find(key) != sdr.end())
	{
		auto s = sdr.find(key);
		return String((char *)s->second.c_str());	
	}
	else 
		return String("");
}

String Settings::find_audio(string key)
{
	if (audio.find(key) != audio.end())
	{
		auto s = audio.find(key);
		return String((char *)s->second.c_str());	
	}
	else 
		return String("");
}

String Settings::find_radio(string key)
{
	if (radio.find(key) != radio.end())
	{
		auto s = radio.find(key);
		return String((char *)s->second.c_str());	
	}
	else 
		return String("");
}

String Settings::find_probe(string key)
{
	if (probes.find(key) != probes.end())
	{
		auto s = probes.find(key);
		return String((char *)s->second.c_str());
	}
	else 
		return String("");
}