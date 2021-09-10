#include <unistd.h>
#include <pthread.h>
#include <mutex>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "lvgl/lvgl.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"
#include "Settings.h"
#include "devices.h"
#include "vfo.h"
#include "gui_right_pane.h"
#include "gui_top_bar.h"
#include "sdrstream.h"
#include "gui_vfo.h"

/*
unsigned long bandswitch[] = { 160, 80, 60, 40, 30, 20, 17, 15, 10, 6, 4, 3, 2,  70, 23 , 13};
unsigned char bandlabda[][3] = { "m", "m", "m", "m", "m", "m", "m", "m", "m", "m", "m", "m", "m", "cm", "cm", "cm" };
uint8_t bandconf[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1 };
unsigned long freqswitch_low[] = { 1800000, 3500000, 5350000, 7000000, 10100000, 14000000, 18068000, 21000000, 28000000,
50000000, 70000000, 83000000 ,144000000, 430000000, 1240000000, 2320000000};
unsigned long freqswitch_high[] = { 1880000, 3800000, 5450000, 7200000, 10150000, 14350000, 18168000, 21450000, 29000000,
52000000, 70500000, 107000000,146000000, 436000000, 1300000000, 2400000000};
*/
	
extern mutex	gui_mutex;
CVfo	vfo;

CVfo::CVfo()
{
	memset(&vfo_setting, 0, sizeof(struct vfo_settings_struct));
	vfo_setting.frq_step = 10;
	vfo_setting.tx = false;
	vfo_setting.tx = true;
}

void CVfo::vfo_rxtx(bool brx, bool btx)
{
	vfo_setting.tx = btx;
	vfo_setting.rx = brx;
}

void CVfo::vfo_init(long long freq, long ifrate)
{
	vfo_setting.vfo_freq[0] = freq;						// initialize the frequency to user default
	vfo_setting.vfo_freq_sdr[0] = freq - ifrate / 4;	// position sdr frequency 1/4 of samplerate lower -> user frequency will be in center of fft display
	vfo_setting.m_offset[0] = freq - vfo_setting.vfo_freq_sdr[0];  					// 
	gui_vfo_inst.set_vfo_gui(0, freq);
	vfo_setting.vfo_freq[1] = freq;
	vfo_setting.vfo_freq_sdr[1] = freq - ifrate / 4;
	vfo_setting.m_offset[1] = freq - vfo_setting.vfo_freq_sdr[1];   					// 
	gui_vfo_inst.set_vfo_gui(1, freq);
	vfo_setting.m_max_offset = ifrate / 2;				// Max offset is 1/2 samplefrequency (Nyquist limit)
	if(freq < vfo_setting.vfo_low || freq > vfo_setting.vfo_high)
		return ;
	get_band(0);
	get_band(1);
	gui_band_instance.set_gui(vfo_setting.band[0]);
	stream_rx_set_frequency(vfo_setting.sdr_dev, vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo]);
	stream_tx_set_frequency(vfo_setting.sdr_dev, vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo]);	
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
	
	if (freq < vfo_setting.vfo_low || freq > vfo_setting.vfo_high)
		return -1;
	//vfo_setting.band[1] = band;
	vfo_setting.vfo_freq[vfo_setting.active_vfo] = freq;
	//printf("freq %lld, sdr %lld offset %lld\n", freq, vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo], freq - vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo]);
	if(abs(freq - vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo]) > vfo_setting.m_max_offset)
	{
		vfo_setting.vfo_freq[vfo_setting.active_vfo] = freq;  						// initialize the frequency to user default
		vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo] = freq - ifrate / 4;  		// position sdr frequency 1/4 of samplerate lower -> user frequency will be in center of fft display
		vfo_setting.m_offset[vfo_setting.active_vfo] = freq - vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo];  					// 
		if (vfo_setting.rx) stream_rx_set_frequency(vfo_setting.sdr_dev, vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo]);
		if (vfo_setting.tx) stream_tx_set_frequency(vfo_setting.sdr_dev, vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo]);					
		tune_flag = true;	
	}
	else if (freq > vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo])
	{
		// frequency increase
	   if((freq - vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo]) > vfo_setting.m_max_offset)
		{
			// if ferquency increase is larger than 1/2 sample frequency calculate new center
			// set a new center frequency
			vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo] += vfo_setting.m_max_offset / 2;										// increase sdr with 1/4 sample frequency
			vfo_setting.m_offset[vfo_setting.active_vfo] = freq - vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo]; 					// 
			if (vfo_setting.rx) stream_rx_set_frequency(vfo_setting.sdr_dev, vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo]);
			if (vfo_setting.tx) stream_tx_set_frequency(vfo_setting.sdr_dev, vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo]);					
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
		vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo] -= vfo_setting.m_max_offset /2;										// decrease sdr with 1/4 sample frequency
		vfo_setting.m_offset[vfo_setting.active_vfo] = freq - vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo]; 					// offset is freq - sdr frequency
		if (vfo_setting.rx) stream_rx_set_frequency(vfo_setting.sdr_dev, vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo]);
		if (vfo_setting.tx) stream_tx_set_frequency(vfo_setting.sdr_dev, vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo]);					
		tune_flag = true;
		//printf("freq %lld, sdr %lld offset %lld\n", freq, vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo], freq - vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo]);
	}
	printf("freq %lld, sdr %lld offset %ld\n", freq, vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo], vfo_setting.m_offset[vfo_setting.active_vfo]);
	
	if (lock)
		unique_lock<mutex> gui_lock(gui_mutex);
	gui_vfo_inst.set_vfo_gui(vfo_setting.active_vfo, freq);
	Wf.set_pos(vfo.vfo_setting.m_offset[vfo.vfo_setting.active_vfo]);
	get_band(vfo_setting.active_vfo);
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
		set_vfo(freq, lock);	
	}
}

long CVfo::get_active_vfo()
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
	vfo_setting.band[vfo_setting.active_vfo] = band;
	set_vfo(freq, false);
}


void CVfo::get_band(int active_vfo)
{
	long long freq = vfo_setting.vfo_freq[active_vfo];
	
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
}