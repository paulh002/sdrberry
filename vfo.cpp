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

struct vfo_settings_struct	vfo_setting;


void vfo_init(long long freq)
{
	memset(&vfo_setting, 0, sizeof(struct vfo_settings_struct));
	vfo_setting.frq_step = 1000;
	vfo_setting.vfo_freq1 = freq;
	gui_vfo_inst.set_vfo_gui(0, freq);
}

/* this function reads the device capability and translates it to f license bandplans*/
void set_vfo_capability(struct device_structure *sdr_dev)
{
	vfo_setting.sdr_dev = sdr_dev;
	
	
}

void set_vfo(int vfo, int band, long long freq)
{
	if (vfo)
	{
		vfo_setting.band[0] = band;
		vfo_setting.vfo_freq1 = freq;
		vfo_setting.active_vfo = 1;
	}
	else
	{
		vfo_setting.band[1] = band;
		vfo_setting.vfo_freq2 = freq;				
		vfo_setting.active_vfo = 0;
	}
	stream_rx_set_frequency(vfo_setting.sdr_dev, freq);
	gui_vfo_inst.set_vfo_gui(vfo, freq);
}

void step_vfo(int vfo, int icount)
{
	long long freq;
	if (vfo)
	{
		//vfo_setting.band[0] = band;
		vfo_setting.vfo_freq2 += (vfo_setting.frq_step * icount);		
		freq = vfo_setting.vfo_freq2;
	}
	else
	{
		//vfo_setting.band[1] = band;
		vfo_setting.vfo_freq1 += (vfo_setting.frq_step * icount);	
		freq = vfo_setting.vfo_freq1;
	}
	if (vfo_setting.sdr_dev != NULL)
		stream_rx_set_frequency(vfo_setting.sdr_dev, freq);
	gui_vfo_inst.set_vfo_gui(vfo, freq);
}

long get_active_vfo()
{
	if (vfo_setting.active_vfo)
		return vfo_setting.vfo_freq2;
	else
		return vfo_setting.vfo_freq1;		
}

std::string get_vfo_str(int vfo)
{
	char	str[20];
	long	freq;
	
	if (vfo)
		freq = vfo_setting.vfo_freq2;
	else
		freq = vfo_setting.vfo_freq1;
	
	if (freq > 10000000LU)
	{
		sprintf(str, "%3ld.%03ld,%02ld Khz", (long)(freq / 1000000), (long)((freq / 1000) % 1000), (long)((freq / 10) % 100));
	}
	else
	{
		sprintf(str, "%3ld.%03ld,%02ld Khz", (long)(freq / 1000000), (long)((freq / 1000) % 1000), (long)((freq / 10) % 100));
	}
	std::string s(str);
	return s;
}