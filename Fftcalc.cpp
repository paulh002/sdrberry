#include "Fftcalc.h"

const int	buffer_size = 1024;

Fftcalc		fft_calulator;

void Fftcalc::init()
{
	N = buffer_size;
	Slide_N = buffer_size / 10;
	in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
	out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
	p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
}

Fftcalc::~Fftcalc()
{
	fftw_destroy_plan(p);
	fftw_free(in);
	fftw_free(out);
}


void Fftcalc::run_fft(std::complex<float> *buffer)
{
	//buffer->imag
	//buffer->real
		
	//hamming(buffer, buffer + Slide_N);
	fftw_execute(p);
}