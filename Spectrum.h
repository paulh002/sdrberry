#pragma once
#include "lvgl_.h"
#include "sdrberry.h"
#include "sma.h"
#include <atomic>
#include <cmath>
#include <complex>
#include <condition_variable>
#include <cstdio>
#include <fftw3.h>
#include <liquid/liquid.h>
#include <mutex>
#include <vector>
#include "Waterfall.h"
#include "PeakMeasurement.h"

extern const int screenWidth;
extern const int screenHeight;
extern const int bottomHeight;
extern const int topHeight;
extern const int tunerHeight;
extern const int rightWidth;

const int nfft_samples{800};
const int s_poits_max{100};

class Spectrum
{
  private:
	lv_obj_t *chart;
	lv_chart_series_t *ser;
	lv_style_t Spectrum_style;
	lv_chart_cursor_t *m_cursor;
	std::unique_ptr<Waterfall> waterfall;
	std::atomic<double> signal_strength{0};
	std::vector<SMA<2>> avg_filter;
	void upload_fft();
	std::unique_ptr<FastFourier> fft;
	lv_point_t drag{0};
	PeakMeasurement finder;
	int signal_strength_offset;

	void draw_event_cb_class(lv_event_t *e);
	void click_event_cb_class(lv_event_t *e);
	void pressing_event_cb_class(lv_event_t *e);
	

  public:
	void init(lv_obj_t *scr, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h, float ifrate);
	void load_data();
	void set_pos(int32_t offset);
	std::vector<lv_coord_t> data_set;
	lv_chart_cursor_t *get_cursor() { return m_cursor; }
	void DrawWaterfall();
	void ProcessWaterfall(const IQSampleVector &input);
	void set_signal_strength(double strength);
	double get_signal_strength() { return signal_strength; }
	void SetFftParts();
	float getSuppression();
	
	static constexpr auto draw_event_cb = EventHandler<Spectrum, &Spectrum::draw_event_cb_class>::staticHandler;
	static constexpr auto click_event_cb = EventHandler<Spectrum, &Spectrum::click_event_cb_class>::staticHandler;
	static constexpr auto pressing_event_cb = EventHandler<Spectrum, &Spectrum::pressing_event_cb_class>::staticHandler;
	
};



template <typename _Real>
static inline void rotshift(complex<_Real> *complexVector, const size_t count)
{
	int center = (int)floor((float)count / 2);
	if (count % 2 != 0)
	{
		center++;
	}
	// odd: 012 34 changes to 34 012
	std::rotate(complexVector, complexVector + center, complexVector + count);
}

template <typename _Real>
static inline void irotshift(complex<_Real> *complexVector, const size_t count)
{
	int center = (int)floor((float)count / 2);
	// odd: 01 234 changes to 234 01
	std::rotate(complexVector, complexVector + center, complexVector + count);
}

extern Spectrum SpectrumGraph;

IQSample::value_type rms_level_approx(const IQSampleVector &samples);
