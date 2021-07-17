#pragma once

#include "devices.h"
#include "vfo.h"

#define NUM_THREADS	4
#define RX_THREAD	0


void create_rx_streaming_thread(struct device_structure *sdr_dev, vfo_settings_struct	*vfo);
void stream_rx_set_frequency(struct device_structure *sdr_dev, unsigned long freq);