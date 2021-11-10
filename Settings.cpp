#include "Settings.h"

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

void Settings::write_settings()
{
	for (auto& option : config->getSection("Radio"))
	{
		auto s = radio.find(option.first);
		option.second.setString(s->second);
	}
	
	config->writeToFile(file.c_str());
}

void Settings::read_settings(string settings_file)
{
	config = new cfg::File();
	
	file = settings_file;
	if (!config->loadFromFile((char *)settings_file.c_str()))
	{
		config->setDefaultOptions(defaultOptions);
		config->writeToFile(settings_file.c_str());
	}
	int i = 0;

	config->useSection("ESP32");
	mac_address = string((char *)(*config)("mac address").toString().c_str());
	for (auto& option : config->getSection("CAT"))
	{
		//cout << "Option name: " << option.first << endl;
		cat.insert(pair<string, string>(option.first, option.second));
	}
	
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
	for (auto& option : config->getSection("samplerate"))
	{
		//cout << "Option name: " << option.first << endl;
		samplerate.insert(pair<string, string>(option.first, option.second));
	}
	for (auto& option : config->getSection("samplerate_tx"))
	{
		//cout << "Option name: " << option.first << endl;
		samplerate_tx.insert(pair<string, string>(option.first, option.second));
	}
	for (auto& option : config->getSection("input"))
	{
		//cout << "Option name: " << option.first << endl;
		input_dev.insert(pair<string, string>(option.first, option.second));
	}
	for (auto& option : config->getSection("Agc"))
	{
		//cout << "Option name: " << option.first << endl;
		agc.insert(pair<string, string>(option.first, option.second));
	}
	
	config->useSection("bands");
	
	for (auto& col : (*config)("meters"))
	{
		if (col.toInt() > 0)
			meters.push_back(col.toInt());
	}
	for (auto& col : (*config)("labels"))
	{
		if (col.toString().length() > 0)
			labels.push_back(col.toString());
	}
	for (auto& col : (*config)("f_low"))
	{
		if (col.toLong() > 0L)
			f_low.push_back(col.toLong());
	}
	for (auto& col : (*config)("f_high"))
	{
		if (col.toLong() > 0L)
			f_high.push_back(col.toLong());
	}
	for (auto& col : (*config)("mode"))
	{
		if (col.toString().length() > 0L)
			mode.push_back(col.toString());
	}
	
	config->useSection("filter");
	
	for (auto& col : (*config)("address"))
	{
		if (col.toInt() > 0)
			address.push_back(col.toInt());
	}
	for (auto& col : (*config)("command_rx"))
	{
		if (col.toInt() > 0)
			command_rx.push_back(col.toInt());
	}
	for (auto& col : (*config)("command_tx"))
	{
		if (col.toInt() > 0)
			command_tx.push_back(col.toInt());
	}
	
}


string Settings::find_sdr(string key)
{
	if (sdr.find(key) != sdr.end())
	{
		auto s = sdr.find(key);
		return string((char *)s->second.c_str());	
	}
	else 
		return string("");
}

string Settings::find_audio(string key)
{
	if (audio.find(key) != audio.end())
	{
		auto s = audio.find(key);
		return string((char *)s->second.c_str());	
	}
	else 
		return string("");
}

string Settings::find_radio(string key)
{
	if (radio.find(key) != radio.end())
	{
		auto s = radio.find(key);
		return string((char *)s->second.c_str());	
	}
	else 
		return string("");
}

string Settings::find_probe(string key)
{
	if (probes.find(key) != probes.end())
	{
		auto s = probes.find(key);
		return string((char *)s->second.c_str());
	}
	else 
		return string("");
}

long long Settings::find_vfo1_freq(string key)
{
	if (vfo1.find(key) != vfo1.end())
	{
		auto s = vfo1.find(key);
		return strtoll((const char *)s->second.c_str(),NULL,0);
	}
	else 
		return 0LL;
}

string Settings::find_vfo1(string key)
{
	if (vfo1.find(key) != vfo1.end())
	{
		auto s = vfo1.find(key);
		return string((char *)s->second.c_str());
	}
	else 
		return string("");
}

string Settings::find_vfo2(string key)
{
	if (vfo2.find(key) != vfo2.end())
	{
		auto s = vfo2.find(key);
		return string((char *)s->second.c_str());
	}
	else 
		return string("");
}

double Settings::find_samplerate(string key)
{
	if (samplerate.find(key) != samplerate.end())
	{
		auto s = samplerate.find(key);
		long l= atol((const char *)s->second.c_str());
		return (double) l * 1000.0;
	}
	else 
		return 0LL;
}

int Settings::volume()
{
	if (radio.find("volume") != radio.end())
	{
		auto s = radio.find("volume");
		return atoi((const char *)s->second.c_str());
	}
	else 
		return 0;
}

int Settings::gain()
{
	if (radio.find("gain") != radio.end())
	{
		auto s = radio.find("gain");
		return atoi((const char *)s->second.c_str());
	}
	else 
		return 0;
}

void Settings::set_gain(int gain)
{
	if (radio.find("gain") != radio.end())
	{
		auto s = radio.find("gain");
		s->second = to_string(gain);
		return;
	}
	else 
		return;
}

int Settings::micgain()
{
	if (radio.find("micgain") != radio.end())
	{
		auto s = radio.find("micgain");
		return atoi((const char *)s->second.c_str());
	}
	else 
		return 0;
}

void Settings::set_micgain(int gain)
{
	if (radio.find("micgain") != radio.end())
	{
		auto s = radio.find("micgain");
		s->second = to_string(gain);
		return;
	}
	else 
		return;
}

int Settings::drive()
{
	if (radio.find("drive") != radio.end())
	{
		auto s = radio.find("drive");
		return atoi((const char *)s->second.c_str());
	}
	else 
		return 0;
}

void Settings::set_drive(int drive)
{
	if (radio.find("drive") != radio.end())
	{
		auto s = radio.find("drive");
		s->second = to_string(drive);
		return;
	}
	else 
		return;
}

double Settings::find_samplerate_tx(string key)
{
	if (samplerate_tx.find(key) != samplerate_tx.end())
	{
		auto s = samplerate_tx.find(key);
		long l = atol((const char *)s->second.c_str());
		return (double) l * 1000.0;
	}
	else 
		return 0LL;
}

string Settings::find_input(string key)
{
	if (input_dev.find(key) != input_dev.end())
	{
		auto s = input_dev.find(key);
		return string((char *)s->second.c_str());	
	}
	else 
		return string("");
}

string Settings::find_cat(string key)
{
	if (cat.find(key) != cat.end())
	{
		auto s = cat.find(key);
		return string((char *)s->second.c_str());
	}
	else 
		return string("");
}



int Settings::agc_mode()
{
	if (agc.find("mode") != agc.end())
	{
		auto s = agc.find("mode");
		return atoi((const char *)s->second.c_str());
	}
	else 
		return 0;
}

int Settings::agc_threshold()
{
	if (agc.find("threshold") != agc.end())
	{
		auto s = agc.find("threshold");
		return atoi((const char *)s->second.c_str());
	}
	else 
		return 0;
}

int Settings::agc_slope()
{
	if (agc.find("slope") != agc.end())
	{
		auto s = agc.find("slope");
		return atoi((const char *)s->second.c_str());
	}
	else 
		return 0;
}

int Settings::agc_delay()
{
	if (agc.find("delay") != agc.end())
	{
		auto s = agc.find("delay");
		return atoi((const char *)s->second.c_str());
	}
	else 
		return 0;
}

int Settings::convert_mode(string s)
{
	int mode = mode_lsb;
	
	to_upper(s);
	if (s == "FM")
		mode = mode_broadband_fm;
	if (s == "LSB")
		mode = mode_lsb;
	if (s == "USB")
		mode = mode_usb;
	if (s == "DSB")
		mode = mode_dsb;
	if (s == "AM")
		mode = mode_am;
	if (s == "CW")
		mode = mode_cw;
	return mode;
}