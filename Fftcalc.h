#pragma once


#include <complex.h>
#include <math.h>
#include <cstdio>
#include <cstdlib>
#include "api/fftw3.h"
#include "liquid.h"

class Fftcalc
{
public:
	void	init();
	void	run_fft(std::complex<float> *buffer);
	~Fftcalc();

private:
	fftw_complex	*in = NULL, *out = NULL;
	fftw_plan		p;
	int				N;
	int				Slide_N;
};

extern Fftcalc		fft_calulator;