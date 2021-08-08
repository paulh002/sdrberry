#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "wstring.h"
#include "lvgl/lvgl.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"
#include "devices.h"
#include "vfo.h"
#include "gui_right_pane.h"
#include "gui_top_bar.h"
#include "sdrstream.h"
#include "gui_vfo.h"

unsigned long bandswitch[] = { 160, 80, 60, 40, 30, 20, 17, 15, 10, 6, 4, 3, 2,  70, 23 , 13};
unsigned char bandlabda[][3] = { "m", "m", "m", "m", "m", "m", "m", "m", "m", "m", "m", "m", "m", "cm", "cm", "cm" };
uint8_t bandconf[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1 };
unsigned long freqswitch_low[] = { 1800000, 3500000, 5350000, 7000000, 10100000, 14000000, 18068000, 21000000, 28000000,
50000000, 70000000, 83000000 ,144000000, 430000000, 1240000000, 2320000000};
unsigned long freqswitch_high[] = { 1880000, 3800000, 5450000, 7200000, 10150000, 14350000, 18168000, 21450000, 29000000,
52000000, 70500000, 107000000,146000000, 436000000, 1300000000, 2400000000};

CVfo	vfo;

CVfo::CVfo()
{
	memset(&vfo_setting, 0, sizeof(struct vfo_settings_struct));
	vfo_setting.frq_step = 10;
}

void CVfo::vfo_init(long long freq)
{
	vfo_setting.vfo_freq[0] = freq;
	gui_vfo_inst.set_vfo_gui(0, freq);
}

/* this function reads the device capability and translates it to f license bandplans*/
void CVfo::set_vfo_capability(struct device_structure *sdr_dev)
{
	vfo_setting.sdr_dev = sdr_dev;
}

int CVfo::set_vfo(long long freq)
{
	if (freq < vfo_setting.vfo_low || freq > vfo_setting.vfo_high)
		return -1;
	
	//vfo_setting.band[1] = band;
	vfo_setting.vfo_freq[vfo_setting.active_vfo] = freq;
	stream_rx_set_frequency(vfo_setting.sdr_dev, freq);
	gui_vfo_inst.set_vfo_gui(vfo_setting.active_vfo, freq);
	return 0;
}

void CVfo::step_vfo(long icount)
{
	long long freq;

	//vfo_setting.band[0] = band;
	vfo_setting.vfo_freq[vfo_setting.active_vfo] += (vfo_setting.frq_step * icount);		
	freq = vfo_setting.vfo_freq[vfo_setting.active_vfo];
	
	if (freq < vfo_setting.vfo_low || freq > vfo_setting.vfo_high)
		return;	
	if (vfo_setting.sdr_dev != NULL)
	{
		stream_rx_set_frequency(vfo_setting.sdr_dev, freq);
	}
	gui_vfo_inst.set_vfo_gui(vfo_setting.active_vfo, freq);
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
	set_vfo(vfo_setting.vfo_freq[vfo_setting.active_vfo]);
}

void CVfo::set_vfo_range(long long low, long long high)
{
	vfo_setting.vfo_low = low;
	vfo_setting.vfo_high = high;
}

void CVfo::set_band(int band, long long freq)
{
	vfo_setting.band[vfo_setting.active_vfo] = band;
	set_vfo(freq);
}