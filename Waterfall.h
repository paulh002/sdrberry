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
#include "sdrberry.h"
#include "sma.h"

extern const int screenWidth;
extern const int screenHeight;
extern const int bottomHeight;
extern const int topHeight;
extern const int tunerHeight;
extern const int rightWidth;

const int nfft_samples	{1024};

class Waterfall
{
public:
  void init(lv_obj_t *scr, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h, float ifrate);
  void load_data();
  void set_pos(int32_t offset);
  std::vector<lv_coord_t> data_set;
  lv_chart_cursor_t *get_cursor()  { return m_cursor;}
  void set_fft_if_rate(float ifrate, int n);
		
private:
	lv_obj_t						*chart;
	lv_chart_series_t				*ser;
	lv_style_t						waterfall_style;
	lv_chart_cursor_t				*m_cursor;
	atomic<float>					m_ifrate;
	atomic<int>						m_n;
};

class Fft_calculator
{
public:
  void process_samples(const IQSampleVector &input);
  void plan_fft(int size);
  void upload_fft(std::vector<lv_coord_t> &data_set);
  void set_signal_strength(double strength);
  double get_signal_strength();

  Fft_calculator();
  ~Fft_calculator();
private:
  const int type = LIQUID_FFT_FORWARD;
  int flags = 0; // FFT flags (typically ignored)
  int nfft = 0;  // transform size
  std::vector<std::complex<float>> fft_output;
  fftplan plan{0};
  std::mutex m_mutex;
  std::condition_variable m_cond;
  double signal_strength{0};
  IQSampleVector m_input;
  std::vector<float> v_window;
  int fft_avg;
  std::vector<SMA<2>> avg_filter;
  std::vector<EMA<2>> ema_filter;
};

template <typename _Real>
	static inline
	void rotshift(complex<_Real> * complexVector, const size_t count)
	{
		int center = (int) floor((float)count / 2);
		if (count % 2 != 0) {
			center++;
		}
		// odd: 012 34 changes to 34 012
		std::rotate(complexVector, complexVector + center, complexVector + count);
	}

template <typename _Real>
	static inline
	void irotshift(complex<_Real> * complexVector, const size_t count)
	{
		int center = (int) floor((float)count / 2);
		// odd: 01 234 changes to 234 01
		std::rotate(complexVector, complexVector + center, complexVector + count);
	}

extern Fft_calculator	Fft_calc;
extern Waterfall		Wf;

IQSample::value_type rms_level_approx(const IQSampleVector& samples);