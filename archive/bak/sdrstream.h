#pragma once

#include <complex>
#include <pthread.h>
#include <semaphore.h>
#include "devices.h"
#include "vfo.h"
#include "linear_ringbuffer.hpp"


#define NUM_THREADS	4
#define RX_THREAD	0

extern	bev::linear_ringbuffer	rbuffer;

void create_rx_streaming_thread(struct device_structure *sdr_dev, vfo_settings_struct	*vfo, double samplerate);
void stream_rx_set_frequency(struct device_structure *sdr_dev, unsigned long freq);