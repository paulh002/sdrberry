#pragma once

#include <complex>
#include <pthread.h>
#include <semaphore.h>
#include "devices.h"
#include "vfo.h"

#include "DataBuffer.h"
#include "SoftFM.h"

#define NUM_THREADS	4
#define RX_THREAD	0


void create_rx_streaming_thread(struct device_structure *sdr_dev, vfo_settings_struct	*vfo, double samplerate, DataBuffer<IQSample> *source_buffer);
void stream_rx_set_frequency(struct device_structure *sdr_dev, unsigned long freq);