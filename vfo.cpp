#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "wstring.h"
#include "lvgl/lvgl.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"
#include "devices.h"
#include "vfo.h"
#include "gui_right_pane.h"
#include "gui_top_bar.h"

unsigned long bandswitch[] = { 160, 80, 60, 40, 30, 20, 17, 15, 10, 6, 4, 2,  70, 23 , 13};
unsigned char bandlabda[][3] = { "m", "m", "m", "m", "m", "m", "m", "m", "m", "m", "m", "m", "cm", "cm", "cm" };
uint8_t bandconf[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1 };
unsigned long freqswitch_low[] = { 1800000, 3500000, 5350000, 7000000, 10100000, 14000000, 18068000, 21000000, 28000000,
50000000, 70000000, 144000000, 430000000, 1240000000, 2320000000};
unsigned long freqswitch_high[] = { 1880000, 3800000, 5450000, 7200000, 10150000, 14350000, 18168000, 21450000, 29000000,
52000000, 70500000, 146000000, 436000000, 1300000000, 2400000000};

struct vfo_settings_struct	vfo;


/* this function reads the device capability and translates it to f license bandplans*/

void set_vfo_capability(struct device_structure *sdr_dev)
{
	double	bandwidth[MAX_NUM_BANDWIDTHS];
	
	vfo.gain_min = floor(sdr_dev->channel_structure_rx[0].gain_range[0][0]);
	vfo.gain_max = floor(sdr_dev->channel_structure_rx[0].gain_range[0][1]);
	set_gain_range(vfo.gain_min, vfo.gain_max);	
	
	if (sdr_dev->channel_structure_rx[0].agc == false)
		hide_agc_slider();
	
	for (int i = 0; i < sdr_dev->channel_structure_rx[0].bandwidth_range_count; i++)
	{
		bandwidth[i] = sdr_dev->channel_structure_rx[0].bandwidth_range[i][0];
	}
	set_bandwidth_dropdown(bandwidth, sdr_dev->channel_structure_rx[0].bandwidth_range_count);
}