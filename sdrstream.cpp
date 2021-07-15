#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <math.h>
#include <cstdio>
#include <cstdlib>
#include <complex.h>
#include "wstring.h"
#include "lvgl/lvgl.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"
#include "devices.h"
#include "sdrstream.h"
#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>
#include <SoapySDR/Types.h>
#include <pthread.h>
#include <semaphore.h>

#include <iostream>

std::complex<float> rx_buff1[1024], rx_buff2[1024];
std::complex<float> tx_buff1[1024], tx_buff2[1024]; 

sem_t rx_buffer_mutex1;


void* rx_streaming_thread(void* args)
{
	while (1)
	{
		// Read buffer and signal output process to process it
		// Use 2 buffers which will be swapped every time
		
	}
}

void create_rx_streaming_thread(struct device_structure *sdr_dev)
{
	sem_init(&rx_buffer_mutex1, 0, 1);
	
}