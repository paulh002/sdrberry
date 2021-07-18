#include <cmath>
#include <iostream>
#include <csignal>
#include <complex>
#include "FMDemodulator.h"
#include <liquid.h>

using namespace std;

complex<float> fm_buff[1024];

void FMDemodulator::init()
{

	

	filter = iirfilt_crcf_create_prototype(ftype, btype,format,order,fc,f0,Ap,As);	
	iirfilt_crcf_print(filter);
	
}



void FMDemodulator::run(complex<float> *buf)
{
	for (int i = 0; i < n; i++) {
		// run filter
		iirfilt_crcf_execute(filter, buf[i], &fm_buff[i]);
	}
}