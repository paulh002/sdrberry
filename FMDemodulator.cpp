#include <cmath>
#include <iostream>
#include <csignal>
#include <complex>
#include "FMDemodulator.h"
#include <liquid.h>
#include "sdrstream.h"
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h> 

FMDemodulator	fm_demod;

using namespace std;
const int	CBUFFSIZE = 1024;

complex<float>  fm_buff[CBUFFSIZE];
complex<float>  fm_resample_buff[CBUFFSIZE];
float			audio_resample_buff[CBUFFSIZE];

FMDemodulator::~FMDemodulator()
{
	if (resampler !=NULL)
		msresamp_crcf_destroy(resampler);
	if (filter != NULL)
		iirfilt_crcf_destroy(filter);
}


void FMDemodulator::init(float iInSampleRate, float iOutSampleRate)
{
	InSampleRate  = (float)iInSampleRate;
	OutSampleRate = (float)iOutSampleRate;
	

	filter = iirfilt_crcf_create_prototype(ftype, btype,format,order,fc,f0,Ap,As);	
	iirfilt_crcf_print(filter);
	
	InSampleRate  = (float)iInSampleRate;
	float r =  iOutSampleRate / iInSampleRate;
	float As = 60.0f; 
	resampler = msresamp_crcf_create(r, As);
	msresamp_crcf_print(resampler);
	
	float kf = 0.1f;
	fdem = freqdem_create(kf);
	
	audio_filter = iirfilt_crcf_create_prototype(ftype, btype, format, order, 0.34013, 0.0, Ap, As);	
	iirfilt_crcf_print(audio_filter); 
	
}



void FMDemodulator::run(complex<float> *buf)
{
	for (int i = 0; i < CBUFFSIZE; i++) {
		// run filter
		iirfilt_crcf_execute(filter, buf[i], &fm_buff[i]);
	}
	
	unsigned int num_written; 
	
	
	msresamp_crcf_execute(resampler, fm_buff, CBUFFSIZE, fm_resample_buff, &num_written);
	
	/*
	for (int i = 0; i < CBUFFSIZE; i++) {
		// run filter
		freqdem_demodulate(fdem, fm_resample_buff[i], &audio_resample_buff[i]);
	}
	
	complex<float> a, b;
	for (int i = 0; i < CBUFFSIZE; i++) {
		// run filter
		
		a.real(audio_resample_buff[i]);
		a.imag(0.0) ;
		iirfilt_crcf_execute(audio_filter, a, &b);
		audio_resample_buff[i] = b.real();
	}
	*/
}

pthread_t	fm_threads[2];

void* rx_fmdemodulator_thread(void* psdr_dev)
{
	int ii = 0;
	
	while (1)
	{
		//sem_wait(&rx_buffer_mutex1);
		if (ii == 0)
		{
			//fm_demod.run(rx_buff1);
			ii = 1;
		}
		else
		{
			//fm_demod.run(rx_buff2);				
			ii = 0;
		}
	}
}

void create_rx_fm_thread(float iInSampleRate, float iOutSampleRate)
{
	fm_demod.init(iInSampleRate, iOutSampleRate);
	//int rc = pthread_create(&fm_threads[0], NULL, rx_fmdemodulator_thread, NULL);
}