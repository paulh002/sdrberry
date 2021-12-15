#include "vfo.h"
#include "Catinterface.h"
#include "BandFilter.h"
#include "sdrstream.h"

extern mutex	gui_mutex;
CVfo	vfo;

CVfo::CVfo()
{
	memset(&vfo_setting, 0, sizeof(struct vfo_settings_struct));
	vfo_setting.frq_step = 10;
	vfo_setting.tx = false;
	vfo_setting.rx = true;
	limit_ham_band = true;
}

void CVfo::vfo_rxtx(bool brx, bool btx)
{
	vfo_setting.tx = btx;
	vfo_setting.rx = brx;
	bpf.SetBand(vfo_setting.band[vfo_setting.active_vfo], vfo_setting.rx);
}

void CVfo::vfo_init(long ifrate, long pcmrate, struct device_structure *dev)
{
	
	string s = Settings_file.find_vfo1("freq");
	long long freq = strtoll((const char *)s.c_str(), NULL, 0);
	sdr_dev = dev;
	string ham = Settings_file.find_radio("band");
	if (ham != "all")
		vfo.limit_ham_band = true;
	else
		vfo.limit_ham_band = false;
	SoapySDR::RangeList r = sdr_dev->channel_structure_rx[soapy_devices[0].rx_channel].full_frequency_range;
	vfo_setting.vfo_low = r.front().minimum();
	vfo_setting.vfo_high = r.front().maximum();
	auto it_band = Settings_file.meters.begin();
	auto it_high = Settings_file.f_high.begin();
	auto it_mode = Settings_file.mode.begin();
	for (auto col : Settings_file.f_low)
	{
		if ((col >= vfo_setting.vfo_low) && (col < vfo_setting.vfo_high))
		{
			vfo_setting.f_low.push_back(col);
			if (it_high != Settings_file.f_high.end())
				vfo_setting.f_high.push_back(*it_high);
			if (it_band != Settings_file.meters.end())
				vfo_setting.meters.push_back(*it_band);
			if (it_mode != Settings_file.mode.end())
			{
				if (*it_mode == "usb")
					vfo_setting.f_mode.push_back(mode_usb);
				if (*it_mode == "lsb")
					vfo_setting.f_mode.push_back(mode_lsb);
				if (*it_mode == "bfm")
					vfo_setting.f_mode.push_back(mode_broadband_fm);
				if (*it_mode == "am")
					vfo_setting.f_mode.push_back(mode_am);
				if (*it_mode == "dsb")
					vfo_setting.f_mode.push_back(mode_dsb);
			}
		}
		it_band++;
		it_high++;
		it_mode++;
	}
	vfo_setting.pcmrate = pcmrate;
	vfo_setting.vfo_freq[0] = freq;						// initialize the frequency to user default
	vfo_setting.vfo_freq_sdr[0] = freq - ifrate / 4;	// position sdr frequency 1/4 of samplerate lower -> user frequency will be in center of fft display
	vfo_setting.m_offset[0] = freq - vfo_setting.vfo_freq_sdr[0];  					// 
	
	s = Settings_file.find_vfo2("freq");
	freq = strtoll((const char *)s.c_str(), NULL, 0);
	s = Settings_file.find_vfo2("Mode");
	vfo_setting.mode[1] = Settings_file.convert_mode(s);
	vfo_setting.vfo_freq[1] = freq;
	vfo_setting.vfo_freq_sdr[1] = freq - ifrate / 4;
	vfo_setting.m_offset[1] = freq - vfo_setting.vfo_freq_sdr[1];   					// 
	vfo_setting.m_max_offset = ifrate / 2;				// Max offset is 1/2 samplefrequency (Nyquist limit)
	if(freq < vfo_setting.vfo_low || freq > vfo_setting.vfo_high)
		return ;
	get_band(0);
	get_band(1);
	gui_band_instance.set_gui(vfo_setting.band[vfo_setting.active_vfo]);
	rx_set_sdr_freq();
	tx_set_sdr_freq();	
	bpf.SetBand(vfo_setting.band[0], vfo_setting.rx);
	gui_vfo_inst.set_vfo_gui(0, freq);
	gui_vfo_inst.set_vfo_gui(1, freq);
	catinterface.SetBand(get_band_in_meters());
}

void CVfo::set_freq_to_sdr()
{
	if (vfo_setting.rx) rx_set_sdr_freq();
	if (vfo_setting.tx) tx_set_sdr_freq(); 
}


void	CVfo::rx_set_sdr_freq()
{
	if (sdr_dev != nullptr) 
	{
		if (sdr_dev->sdr != nullptr) 
			sdr_dev->sdr->setFrequency(SOAPY_SDR_RX, 0, vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo]);
	}
}

void	CVfo::tx_set_sdr_freq()
{
	if (sdr_dev != nullptr) 
	{
		if (sdr_dev->sdr != NULL)
		{
			sdr_dev->sdr->setFrequency(SOAPY_SDR_TX, 0, vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo]);	
			sdr_dev->sdr->setSampleRate(SOAPY_SDR_TX, 0, ifrate_tx); 
			sdr_dev->sdr->setBandwidth(SOAPY_SDR_TX, 0, ifrate_tx); 
			sdr_dev->sdr->setGain(SOAPY_SDR_TX, 0, sdr_dev->channel_structure_tx[0].gain);
		}
	}
}

void CVfo::vfo_re_init(long ifrate, long pcmrate)
{	
	vfo_setting.pcmrate = pcmrate;
	vfo_setting.vfo_freq_sdr[0] = vfo_setting.vfo_freq[0] - ifrate / 4; // position sdr frequency 1/4 of samplerate lower -> user frequency will be in center of fft display
	vfo_setting.m_offset[0] = vfo_setting.vfo_freq[0] - vfo_setting.vfo_freq_sdr[0]; // 
	
	vfo_setting.vfo_freq_sdr[1] = vfo_setting.vfo_freq[1] - ifrate / 4;
	vfo_setting.m_offset[1] = vfo_setting.vfo_freq[1] - vfo_setting.vfo_freq_sdr[1]; // 
	vfo_setting.m_max_offset = ifrate / 2; // Max offset is 1/2 samplefrequency (Nyquist limit)
}

/* this function reads the device capability and translates it to f license bandplans*/
void CVfo::set_vfo_capability(struct device_structure *sdr_dev)
{
	vfo_setting.sdr_dev = sdr_dev;
}

long CVfo::get_vfo_offset()
{
	unique_lock<mutex> lock(m_vfo_mutex); 
	return vfo_setting.m_offset[vfo_setting.active_vfo];
}
/*
 *
 * The sdr radio wil start with the frequency selected in the center of the fft
 * When the user changes the freqency the offset to the center is calculated and the mixer
 * in the modulator will up or down mix the signal until 1/2 the sample frequency.
 * If the frequency is changed higher or lower than half the sampling frequency the sdr oscilator is
 * changed 1/4 of the sample frequency higer or lower
 * 
 * This is done to minimize the changing of the local oscilator of the sdr receiver (like Pluto)
 * ToDo: for SDR receivers without local oscilator this behaviour is not necessary, but sill used
 * to be checked if this is ok for receivers like radioberry
 **/

int CVfo::set_vfo(long long freq, bool lock)
{
	unique_lock<mutex> lock_set_vfo(m_vfo_mutex);
	
	if (freq == 0L)
	{
		freq = vfo_setting.vfo_freq[vfo_setting.active_vfo];		
	}
	if (freq < vfo_setting.vfo_low || freq > vfo_setting.vfo_high)
		return -1;
	//vfo_setting.band[1] = band;
	vfo_setting.vfo_freq[vfo_setting.active_vfo] = freq;
	if (((abs(ifrate - ifrate_tx) > 0.1) || (abs(ifrate_tx - (double)vfo_setting.pcmrate) < 0.1)) && vfo_setting.tx)
	{  // incase of different ifrates for tx don't use offset or incase tx samplerate is equal to pcmrate
		tx_set_sdr_freq();
	}
	else
	{
		//printf("freq %lld, sdr %lld offset %lld\n", freq, vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo], freq - vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo]);
		if (abs(freq - vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo]) > vfo_setting.m_max_offset)
		{
			vfo_setting.vfo_freq[vfo_setting.active_vfo] = freq;  						// initialize the frequency to user default
			vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo] = freq - ifrate / 4;  		// position sdr frequency 1/4 of samplerate lower -> user frequency will be in center of fft display
			vfo_setting.m_offset[vfo_setting.active_vfo] = freq - vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo];  					// 
			if (vfo_setting.rx) rx_set_sdr_freq();
			if (vfo_setting.tx) tx_set_sdr_freq(); 
			tune_flag = true;
		}
		else if (freq > vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo])
		{
			// frequency increase
			if ((freq - vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo]) > vfo_setting.m_max_offset)
			{
				// if ferquency increase is larger than 1/2 sample frequency calculate new center
				// set a new center frequency
				vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo] += vfo_setting.m_max_offset / 2;										// increase sdr with 1/4 sample frequency
				vfo_setting.m_offset[vfo_setting.active_vfo] = freq - vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo]; 					// 
				if (vfo_setting.rx) rx_set_sdr_freq();
				if (vfo_setting.tx) tx_set_sdr_freq(); 
				tune_flag = true;																										// inform modulator of offset change
				//printf("set sdr frequency %lld %lld %lld\n", vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo], vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo] + vfo_setting.m_offset[vfo_setting.active_vfo], freq);
			}
			else
			{
				vfo_setting.m_offset[vfo_setting.active_vfo] = freq - vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo];
				tune_flag = true;
				//printf("set offset %ld\n", vfo_setting.m_offset[vfo_setting.active_vfo]);
			}
		}
		else
		{
			// if ferquency increase is lower than 1/2 sample frequency calculate new center
			// frequency decrease		
			vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo] -= vfo_setting.m_max_offset / 2;										// decrease sdr with 1/4 sample frequency
			vfo_setting.m_offset[vfo_setting.active_vfo] = freq - vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo]; 					// offset is freq - sdr frequency
			if (vfo_setting.rx) rx_set_sdr_freq();
			if (vfo_setting.tx) tx_set_sdr_freq(); 
			tune_flag = true;
			//printf("freq %lld, sdr %lld offset %lld\n", freq, vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo], freq - vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo]);
		}
	}
	printf("freq %lld, sdr %lld offset %ld\n", freq, vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo], vfo_setting.m_offset[vfo_setting.active_vfo]);
	if (lock)
		unique_lock<mutex> gui_lock(gui_mutex);
	gui_vfo_inst.set_vfo_gui(vfo_setting.active_vfo, freq);
	Wf.set_pos(vfo.vfo_setting.m_offset[vfo.vfo_setting.active_vfo]);
	if (get_band(vfo_setting.active_vfo))
	{ // Band Change?
		catinterface.SetBand(get_band_in_meters());
		bpf.SetBand(vfo_setting.band[vfo.vfo_setting.active_vfo], vfo_setting.rx);
		printf("vfo band change\n");
	}
	gui_band_instance.set_gui(vfo_setting.band[0]);
	return 0;
}

void CVfo::sync_rx_vfo()
{
	vfo_setting.vfo_freq[1] = vfo_setting.vfo_freq[0] ;
	gui_vfo_inst.set_vfo_gui(1, vfo_setting.vfo_freq[1]);
}
	
void CVfo::step_vfo(long icount, bool lock)
{
	long long freq;

	if (m_delay)
	{
		m_delay_counter += abs(icount);
		if (m_delay_counter < m_delay)
			return;
		m_delay_counter = 0;
	}
	vfo_setting.vfo_freq[vfo_setting.active_vfo] += (vfo_setting.frq_step * icount);		
	freq = vfo_setting.vfo_freq[vfo_setting.active_vfo];
	
	if (freq < vfo_setting.vfo_low || freq > vfo_setting.vfo_high)
		return;	
	if (vfo_setting.sdr_dev != NULL)
	{
		if (vfo.limit_ham_band)
			check_band(icount, freq);
		set_vfo(freq, lock);	
	}
}

long CVfo::get_active_vfo_freq()
{
	return vfo_setting.vfo_freq[vfo_setting.active_vfo];
}


std::string CVfo::get_vfo_str()
{
	char	str[20];
	long	freq;
	
	freq = vfo_setting.vfo_freq[vfo_setting.active_vfo];
	if (freq > 10000000LU)
	{
		sprintf(str, "%3ld.%03ld,%02ld", (long)(freq / 1000000), (long)((freq / 1000) % 1000), (long)((freq / 10) % 100));
	}
	else
	{
		sprintf(str, "%3ld.%03ld,%02ld", (long)(freq / 1000000), (long)((freq / 1000) % 1000), (long)((freq / 10) % 100));
	}
	std::string s(str);
	return s;
}

long long CVfo::get_tx_frequency()
{
	// incase of different ifrates for tx don't use offset
	if (fabs(ifrate - ifrate_tx) > 0.1)
		return vfo_setting.vfo_freq[vfo_setting.active_vfo];
	else
		return vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo];
}

void CVfo::set_tuner_offset(double offset)
{
	vfo_setting.tuner_offset = offset;
}

void CVfo::set_active_vfo(int active_vfo)
{
	vfo_setting.active_vfo = min(active_vfo,1);
	set_vfo(vfo_setting.vfo_freq[vfo_setting.active_vfo], false);
}

void CVfo::set_vfo_range(long long low, long long high)
{
	vfo_setting.vfo_low = low;
	vfo_setting.vfo_high = high;
}

void CVfo::set_band(int band, long long freq)
{
	int index = getBandIndex(band);
	if (index >= 0 && index < vfo_setting.f_mode.size())
	{
		if (vfo_setting.mode[vfo_setting.active_vfo] != vfo_setting.f_mode[index])
		{
			vfo_setting.mode[vfo_setting.active_vfo] = vfo_setting.f_mode[index];
			set_vfo(freq, false);
			select_mode(vfo_setting.mode[vfo_setting.active_vfo], false);
			return;
		}
	}
	set_vfo(freq, false);
}

int CVfo::get_band(int active_vfo)
{
	long long	freq = vfo_setting.vfo_freq[active_vfo];
	int			band = vfo_setting.band[active_vfo];
	
	auto it_high = Settings_file.f_high.begin();
	auto it_band = Settings_file.meters.begin();
	for (auto& col : Settings_file.f_low)
	{
		if ((freq >= col) && (freq <= *it_high))
			{
				vfo_setting.band[active_vfo] = *it_band;
			}
		it_high++;
		it_band++;
	}
	
	if (band == vfo_setting.band[active_vfo])
		return 0;
	else
		return 1;
}

void CVfo::check_band(int dir, long long& freq)
{
		// this function let the active vfo jump to next or previous band	
	int i = 0;
	auto it_band = vfo_setting.meters.begin();
	for (auto& col : vfo_setting.meters)
	{
		if (vfo_setting.band[vfo_setting.active_vfo] == col)
		{
			if ((freq > vfo_setting.f_high[i]) && (dir > 0))
			{
				i++;
				if (i >= vfo_setting.meters.size())
				{
					i = 0;
					freq = vfo_setting.f_low[i];
					break;
				}
				else
				{
					freq = vfo_setting.f_low[i];
					break;					
				}
			}
			
			if ((freq < vfo_setting.f_low[i]) && (dir < 0))
			{
				if (freq < vfo_setting.f_low[i]) 
				{
					i--;
					if (i < 0)
					{
						i = vfo_setting.meters.size() - 1;
						freq = vfo_setting.f_high[i];
						break;
					}
					else
					{
						freq = vfo_setting.f_high[i];
						break;					
					}	
				}
			}
			break;
		}
		i++;
	}
}


void CVfo::return_bands(vector<int> &bands)
{
	bands = vfo_setting.meters;
}

int CVfo::getBandIndex(int band)
{
	int i = 0;
	for (auto& col : vfo_setting.meters)
	{
		if (band == col)
			return i;
		i++;
	}
	return -1;
}