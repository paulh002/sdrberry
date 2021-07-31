#pragma once
#include <cstdio>
#include <cmath>
#include <fftw3.h>
#include <liquid.h>
#include <complex>
#include <vector>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "lvgl/lvgl.h"

extern const int screenWidth;
extern const int screenHeight;
extern const int bottomHeight;
extern const int topHeight;
extern const int tunerHeight;
extern const int rightWidth;

const int nfft_samples	{4096};

class Waterfall
{
public:
	void init(lv_obj_t* scr, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h);
	void load_data();
	std::vector<lv_coord_t>			data_set;

private:
	lv_obj_t						*chart;
	lv_chart_series_t				*ser;
	lv_style_t						waterfall_style;
	
};

class Fft_calculator
{
public:	
	void	process_samples();
	void	plan_fft(float *f_in);
	void	upload_fft(std::vector<lv_coord_t>&	data_set);
	
	Fft_calculator();
	~Fft_calculator();
private:
	const int					type = LIQUID_FFT_FORWARD; 
	int							flags = 0;                     // FFT flags (typically ignored)
	const unsigned int			nfft = nfft_samples;    // transform size
	std::vector<std::complex<float>> fft_output;
	fftplan						plan;
	liquid_float_complex		*p_in, *p_out;
	std::mutex					m_mutex;
	std::condition_variable		m_cond;
	
};

extern Fft_calculator	Fft_calc;