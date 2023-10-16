#include "Settings.h"

using namespace std;
Settings	Settings_file;

const cfg::File::ConfigMap defaultOptions = {
	{"SDR Receivers", {{"default", cfg::makeOption("radioberry")}}},
	{"input", {{"mouse", cfg::makeOption("Mouse")}, {"touchscreen", cfg::makeOption("ft5x06")}}},
	{"probes", {{"plutosdr", cfg::makeOption("driver=plutosdr,hostname=192.168.100.1")}, {"radioberry", cfg::makeOption("driver=radioberry")}, {"rtlsdr", cfg::makeOption("driver=rtlsdr")}, {"sdrplay", cfg::makeOption("driver=sdrplay")}}},
	{"ESP32", {{"mac address", cfg::makeOption("")}}},
	{"CAT", {{"USB", cfg::makeOption("/dev/serial/by-id/usb-Silicon_Labs_CP2102_USB_to_UART_Bridge_Controller_0001-if00-port0")}}},
	{"samplerate", {{"radioberry", cfg::makeOption(384)}, {"plutosdr", cfg::makeOption(1000)}, {"rtlsdr", cfg::makeOption(1000)}, {"sdrplay", cfg::makeOption(1000)}}},
	{"samplerate_tx", {{"radioberry", cfg::makeOption(384)}}},
	{"Radio", {{"gain", cfg::makeOption(0, 0, 100)}, {"volume", cfg::makeOption(50)}, {"drive", cfg::makeOption(89)}, {"micgain", cfg::makeOption(50)}, {"band", cfg::makeOption("ham")}, {"AGC", cfg::makeOption("off")}, {"if-gain", cfg::makeOption(30)}, {"noise", cfg::makeOption(4)}, {"noisefloor", cfg::makeOption(30)}, {"waterfallsize", cfg::makeOption(3)}}},
	{"radioberry", {{"gain", cfg::makeOption(60)}, {"drive", cfg::makeOption(89)}, {"if-gain", cfg::makeOption(30)}, {"samplerate", cfg::makeOption("384")}, {"samplerate_tx", cfg::makeOption("48")}, {"AGC", cfg::makeOption("off")}, {"span", cfg::makeOption(96)}, {"audiobuffer", cfg::makeOption(4096)}}},
	{"hifiberry", {{"gain", cfg::makeOption(0)}, {"drive", cfg::makeOption(89)}, {"if-gain", cfg::makeOption(40)}, {"samplerate", cfg::makeOption("192")}, {"samplerate_tx", cfg::makeOption("192")}, {"AGC", cfg::makeOption("off")}, {"span", cfg::makeOption(96)}, {"audiobuffer", cfg::makeOption(4096)}, {"correction", cfg::makeOption(1)}, {"dc", cfg::makeOption(1)}}},
	{"sdrplay", {{"gain", cfg::makeOption(30)}, {"drive", cfg::makeOption(89)}, {"if-gain", cfg::makeOption(30)}, {"AGC", cfg::makeOption("off")}, {"samplerate", cfg::makeOption("1000")}, {"span", cfg::makeOption(384)}, {"audiobuffer", cfg::makeOption(4096)}}},
	{"rtlsdr", {{"gain", cfg::makeOption(40)}, {"drive", cfg::makeOption(89)}, {"if-gain", cfg::makeOption(60)}, {"samplerate", cfg::makeOption("1000")}, {"span", cfg::makeOption(384)}}},
	{"hackrf", {{"gain", cfg::makeOption(30)}, {"drive", cfg::makeOption(89)}, {"if-gain", cfg::makeOption(3)}, {"samplerate", cfg::makeOption("2000")}, {"samplerate_tx", cfg::makeOption("384")}, {"span", cfg::makeOption(384)}}},
	{"plutosdr", {{"gain", cfg::makeOption(60)}, {"drive", cfg::makeOption(89)}, {"if-gain", cfg::makeOption(30)}, {"samplerate", cfg::makeOption("384")}, {"samplerate_tx", cfg::makeOption("384")}, {"AGC", cfg::makeOption("off")}, {"span", cfg::makeOption(384)}, {"audiobuffer", cfg::makeOption(4096)}}},
	{"VFO1", {{"freq", cfg::makeOption(3500000)}, {"Mode", cfg::makeOption("LSB")}}},
	{"VFO2", {{"freq", cfg::makeOption(3500000)}, {"Mode", cfg::makeOption("LSB")}}},
	{"Audio", {{"device", cfg::makeOption("USB Audio Device")}}},
	{"Agc", {{"mode", cfg::makeOption(1)}, {"ratio", cfg::makeOption(10)}, {"threshold", cfg::makeOption(10)}}},
	{"Speech", {{"mode", cfg::makeOption(1)}, {"ratio", cfg::makeOption(12)}, {"threshold", cfg::makeOption(0)}, {"bass", cfg::makeOption(0)}, {"treble", cfg::makeOption(0)}}},
	{"filter", {{"i2cdevice", cfg::makeOption("pcf8574")}}},
	{"wsjtx", {{"call", cfg::makeOption("PA0PHH")}, {"locator", cfg::makeOption("JO22")}, {"tx", cfg::makeOption("1200")}, {"rx", cfg::makeOption("1200")}}}
};

void Settings::write_settings()
{	
	config->writeToFile(file.c_str());
}

void Settings::default_settings()
{
	config->useSection("SDR Receivers");
	(*config)("receivers").push(cfg::makeOption("radioberry"));
	(*config)("receivers").push(cfg::makeOption("plutosdr"));
	(*config)("receivers").push(cfg::makeOption("rtlsdr"));
	(*config)("receivers").push(cfg::makeOption("sdrplay"));
	(*config)("receivers").push(cfg::makeOption("hifiberry"));

	config->useSection("Agc");
	(*config)("fast").push(cfg::makeOption(10));
	(*config)("fast").push(cfg::makeOption(100));
	(*config)("medium").push(cfg::makeOption(50));
	(*config)("medium").push(cfg::makeOption(250));
	(*config)("slow").push(cfg::makeOption(100));
	(*config)("slow").push(cfg::makeOption(500));
	
	config->useSection("Speech");
	(*config)("fast").push(cfg::makeOption(10));
	(*config)("fast").push(cfg::makeOption(100));
	(*config)("medium").push(cfg::makeOption(50));
	(*config)("medium").push(cfg::makeOption(250));
	(*config)("slow").push(cfg::makeOption(100));
	(*config)("slow").push(cfg::makeOption(500));

	config->useSection("bands");
	(*config)("meters").push(cfg::makeOption(160));
	(*config)("meters").push(cfg::makeOption(80));
	(*config)("meters").push(cfg::makeOption(60));
	(*config)("meters").push(cfg::makeOption(40));
	(*config)("meters").push(cfg::makeOption(30));
	(*config)("meters").push(cfg::makeOption(20));
	(*config)("meters").push(cfg::makeOption(17));
	(*config)("meters").push(cfg::makeOption(15));
	(*config)("meters").push(cfg::makeOption(10));
	(*config)("meters").push(cfg::makeOption(6));
	(*config)("meters").push(cfg::makeOption(4));
	(*config)("meters").push(cfg::makeOption(3));
	(*config)("meters").push(cfg::makeOption(2));
	(*config)("meters").push(cfg::makeOption(70));
	(*config)("meters").push(cfg::makeOption(23));
	(*config)("meters").push(cfg::makeOption(13));

	(*config)("labels").push(cfg::makeOption("m"));
	(*config)("labels").push(cfg::makeOption("m"));
	(*config)("labels").push(cfg::makeOption("m"));
	(*config)("labels").push(cfg::makeOption("m"));
	(*config)("labels").push(cfg::makeOption("m"));
	(*config)("labels").push(cfg::makeOption("m"));
	(*config)("labels").push(cfg::makeOption("m"));
	(*config)("labels").push(cfg::makeOption("m"));
	(*config)("labels").push(cfg::makeOption("m"));
	(*config)("labels").push(cfg::makeOption("m"));
	(*config)("labels").push(cfg::makeOption("m"));
	(*config)("labels").push(cfg::makeOption("m"));
	(*config)("labels").push(cfg::makeOption("m"));
	(*config)("labels").push(cfg::makeOption("cm"));
	(*config)("labels").push(cfg::makeOption("cm"));
	(*config)("labels").push(cfg::makeOption("cm"));

	(*config)("f_low").push(cfg::makeOption(1800000));
	(*config)("f_low").push(cfg::makeOption(3500000));
	(*config)("f_low").push(cfg::makeOption(5350000));
	(*config)("f_low").push(cfg::makeOption(7000000));
	(*config)("f_low").push(cfg::makeOption(10100000));
	(*config)("f_low").push(cfg::makeOption(14000000));
	(*config)("f_low").push(cfg::makeOption(18068000));
	(*config)("f_low").push(cfg::makeOption(21000000));
	(*config)("f_low").push(cfg::makeOption(28000000));
	(*config)("f_low").push(cfg::makeOption(50000000));
	(*config)("f_low").push(cfg::makeOption(70000000));
	(*config)("f_low").push(cfg::makeOption(83000000));
	(*config)("f_low").push(cfg::makeOption(144000000));
	(*config)("f_low").push(cfg::makeOption(430000000));
	(*config)("f_low").push(cfg::makeOption(1240000000));
	(*config)("f_low").push(cfg::makeOption(2320000000));

	(*config)("f_high").push(cfg::makeOption(1880000));
	(*config)("f_high").push(cfg::makeOption(3800000));
	(*config)("f_high").push(cfg::makeOption(5450000));
	(*config)("f_high").push(cfg::makeOption(7200000));
	(*config)("f_high").push(cfg::makeOption(10150000));
	(*config)("f_high").push(cfg::makeOption(14350000));
	(*config)("f_high").push(cfg::makeOption(18168000));
	(*config)("f_high").push(cfg::makeOption(21450000));
	(*config)("f_high").push(cfg::makeOption(29000000));
	(*config)("f_high").push(cfg::makeOption(52000000));
	(*config)("f_high").push(cfg::makeOption(70500000));
	(*config)("f_high").push(cfg::makeOption(107000000));
	(*config)("f_high").push(cfg::makeOption(146000000));
	(*config)("f_high").push(cfg::makeOption(436000000));
	(*config)("f_high").push(cfg::makeOption(1300000000));
	(*config)("f_high").push(cfg::makeOption(2400000000));

	(*config)("mode").push(cfg::makeOption("lsb"));
	(*config)("mode").push(cfg::makeOption("lsb"));
	(*config)("mode").push(cfg::makeOption("lsb"));
	(*config)("mode").push(cfg::makeOption("lsb"));
	(*config)("mode").push(cfg::makeOption("lsb"));
	(*config)("mode").push(cfg::makeOption("usb"));
	(*config)("mode").push(cfg::makeOption("usb"));
	(*config)("mode").push(cfg::makeOption("usb"));
	(*config)("mode").push(cfg::makeOption("usb"));
	(*config)("mode").push(cfg::makeOption("usb"));
	(*config)("mode").push(cfg::makeOption("usb"));
	(*config)("mode").push(cfg::makeOption("usb"));
	(*config)("mode").push(cfg::makeOption("usb"));
	(*config)("mode").push(cfg::makeOption("usb"));
	(*config)("mode").push(cfg::makeOption("usb"));
	(*config)("mode").push(cfg::makeOption("usb"));

	config->useSection("wsjtx");
	(*config)("freqFT8").push(cfg::makeOption(1840));
	(*config)("freqFT8").push(cfg::makeOption(3573));
	(*config)("freqFT8").push(cfg::makeOption(5357));
	(*config)("freqFT8").push(cfg::makeOption(7073));
	(*config)("freqFT8").push(cfg::makeOption(10133));
	(*config)("freqFT8").push(cfg::makeOption(14074));
	(*config)("freqFT8").push(cfg::makeOption(18100));
	(*config)("freqFT8").push(cfg::makeOption(21074));
	(*config)("freqFT8").push(cfg::makeOption(28074));

	(*config)("freqFT4").push(cfg::makeOption(1840));
	(*config)("freqFT4").push(cfg::makeOption(3568));
	(*config)("freqFT4").push(cfg::makeOption(3575));
	(*config)("freqFT4").push(cfg::makeOption(7047));
	(*config)("freqFT4").push(cfg::makeOption(10140));
	(*config)("freqFT4").push(cfg::makeOption(14080));
	(*config)("freqFT4").push(cfg::makeOption(18104));
	(*config)("freqFT4").push(cfg::makeOption(21140));
	(*config)("freqFT4").push(cfg::makeOption(28180));

	config->useSection("i2c");
	(*config)("devices").push(cfg::makeOption("PCF8574"));
	(*config)("devices").push(cfg::makeOption("PCF8574"));
	(*config)("devices").push(cfg::makeOption("PCF8574A"));
	
	(*config)("address").push(cfg::makeOption("20"));
	(*config)("address").push(cfg::makeOption("21"));
	(*config)("address").push(cfg::makeOption("3F"));
}

void Settings::read_settings(string settings_file)
{
	config = new cfg::File();
	
	file = settings_file;
	if (!config->loadFromFile((char *)settings_file.c_str()))
	{
		config->setDefaultOptions(defaultOptions);
		default_settings();
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
	
	for (auto &option : config->getSection("Speech"))
	{
		cout << "Option name: " << option.first << " value: " << option.second <<endl;
		speech.insert(pair<string, string>(option.first, option.second));
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
	
	config->useSection("SDR Receivers");
	for (auto& col : (*config)("receivers"))
	{
		if (col.toString().length() > 0L)
			receivers.push_back(col.toString());
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

int Settings::if_gain()
{
	if (radio.find("if-gain") != radio.end())
	{
		auto s = radio.find("if-gain");
		return atoi((const char *)s->second.c_str());
	}
	else
		return 0;
}

int Settings::if_gain(string sdrdevice)
{
	auto option = config->getSection(sdrdevice);
	auto s = option.find("if-gain");
	string st = s->second;
	return atoi((const char *)st.c_str());
}

int Settings::gain(string sdrdevice)
{
	auto option = config->getSection(sdrdevice);
	auto s = option.find("gain");
	string st = s->second;
	return atoi((const char *)st.c_str());
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

int Settings::getagc(std::string key)
{
	if (agc.find(key) != agc.end())
	{
		auto s = agc.find(key);
		return atoi((const char *)s->second.c_str());
	}
	else
		return 0;
}

int Settings::getspeech(std::string key)
{
	if (speech.find(key) != speech.end())
	{
		auto s = speech.find(key);
		return atoi((const char *)s->second.c_str());
	}
	else
		return 0;
}

int Settings::convert_mode(string s)
{
	int mode = mode_lsb;

	std::transform(s.begin(), s.end(), s.begin(), ::toupper);
	if (s == "FM")
		mode = mode_narrowband_fm;
	if (s == "BFM")
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

void Settings::getagc_preset(std::string key, int &atack, int &release)
{
	int i = 0;
	atack = 0;
	release = 0;
	config->useSection("Agc");
	for (auto &col : (*config)(key))
	{
		if (i == 0)
			atack = col.toInt();
		if (i == 1)			
			release = col.toInt();
		i++;
	}
}

void Settings::getspeech_preset(std::string key, int &atack, int &release)
{
	int i = 0;
	atack = 0;
	release = 0;
	config->useSection("Speech");
	for (auto &col : (*config)(key))
	{
		if (i == 0)
			atack = col.toInt();
		if (i == 1)
			release = col.toInt();
		i++;
	}
}

void Settings::save_speech(std::string key, int value)
{
	config->useSection("Speech");
	auto &col = (*config)(key);
	col = value;
}

void Settings::save()
{
	config->writeToFile(file.c_str());
}

void Settings::save_ifgain(int ifgain)
{
	config->useSection("Radio");
	auto &col = (*config)("if-gain");
	col = ifgain;
}

void Settings::save_vol(int vol)
{
	config->useSection("Radio");
	auto &col = (*config)("volume");
	col = vol;
}

void Settings::save_rf(int rf)
{
	config->useSection("Radio");
	auto &col = (*config)("gain");
	col = rf;
}

void Settings::save_vfo(int vfo, long freq)
{
	if (vfo)
	{
		config->useSection("VFO2");
	}
	else
	{
		config->useSection("VFO1");
	}
	auto &col = (*config)("freq");
	col = freq;
}

void Settings::save_span(int span)
{
	config->useSection("Radio");
	auto &col = (*config)("span");
	col = span;
}

// New functions

int Settings::get_int(string sdrdevice, string key, int defaultValue)
{
	auto option = config->getSection(sdrdevice);
	auto s = option.find(key);
	if (s == option.end())
		return defaultValue;
	string st = s->second;
	int value = atoi((const char *)st.c_str());
	return value;
}

void Settings::save_int(string section, string key, int value)
{
	config->useSection(section);
	auto &col = (*config)(key);
	col = value;
}

string Settings::get_string(string sdrdevice, string key)
{
	string st;
	auto option = config->getSection(sdrdevice);
	auto s = option.find(key);
	if (s != option.end())
		st = s->second;
	return st;
}

void Settings::save_string(string section, string key, string value)
{
	config->useSection(section);
	auto &col = (*config)(key);
	col = value;
}

void Settings::get_array_long(std::string section, std::string key, vector<long> &array)
{
	config->useSection(section);
	for (auto &col : (*config)(key))
		array.push_back(col.toInt());
}

void Settings::set_array_long(std::string section, std::string key, vector<long> &array)
{
	int i = 0;
	config->useSection(section);
	auto &val = (*config)(key);
	for (auto col : array)
	{
		if (val.size() <= i)
			val.push(cfg::makeOption(col));
		else
			val[i] = col;
		i++;
	}
	write_settings();
}

void Settings::get_array_int(std::string section, std::string key, vector<int> &array)
{
	if (config->optionExists(key, section))
	{
		config->useSection(section);
		for (auto &col : (*config)(key))
		{
			array.push_back(col.toInt());
		}
	}
}

void Settings::set_array_int(std::string section, std::string key, vector<int> &array)
{
	int i = 0;

	// new keys are automaticly created
	config->useSection(section);
	auto &val = (*config)(key);

	for (auto col : array)
	{
		if (val.size() <= i)
			val.push(cfg::makeOption(col));
		else
			val[i] = col;
		i++;
	}
	write_settings();
}

std::vector<std::string> Settings::get_array_string(std::string section, std::string key)
{
	std::vector<std::string> result;
	config->useSection(section);
	for (auto &col : (*config)(key))
	{
		if (col.toString().length() > 0L)
			result.push_back(col.toString());
	}
	return result;
}

void Settings::set_array_string(std::string section, std::string key, const vector<std::string> &array)
{
	int i = 0;

	// new keys are automaticly created
	config->useSection(section);
	auto &val = (*config)(key);

	val.clear();
	for (auto col : array)
	{
		if (val.size() <= i)
			val.push(cfg::makeOption(col));
		else
			val[i] = col;
		i++;
	}
	write_settings();
}