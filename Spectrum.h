#pragma once
#include "lvgl_.h"
#include "sma.h"
#include <atomic>
#include <cmath>
#include <complex>
#include <condition_variable>
#include <cstdio>
#include <fftw3.h>
#include <liquid/liquid.h>
#include <mutex>
#include <array>
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
const int s_poits_max{130};

class Spectrum
{
  private:
	lv_obj_t *chart;
	lv_chart_series_t *ser, *peak_ser{nullptr};
	lv_style_t Spectrum_style;
	lv_chart_cursor_t *FrequencyCursor;
	std::array<lv_chart_cursor_t *, 5> markers{nullptr};
	std::array<int32_t, 5> markers_location{0};
	std::unique_ptr<Waterfall> waterfall;
	std::atomic<double> signal_strength{0};
	std::vector<SMA<2>> avg_filter;
	std::vector<SMA<4>> avg_peak_filter;
	std::vector<lv_coord_t> data_set;
	std::vector<lv_coord_t> data_set_peak;
	void upload_fft();
	std::unique_ptr<FastFourier> fft;
	lv_point_t drag{0};
	PeakMeasurement finder;
	int signal_strength_offset;
	int noisefloor{0};
	std::vector<int> peaks;
	int active_markers{0};
	int drag_marker{0};
	bool hold_peak{false};

	void draw_event_cb_class(lv_event_t *e);
	void click_event_cb_class(lv_event_t *e);
	void pressing_event_cb_class(lv_event_t *e);
	void draw_marker_label(lv_chart_cursor_t *cursor, lv_obj_draw_part_dsc_t *dsc);
	std::pair<bool, int> cursor_intersect(lv_point_t p);
	

  public:
	void init(lv_obj_t *scr, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h, float ifrate);
	void load_data();
	void set_pos(int32_t offset);
	void set_marker(int marker, int32_t offset);
	void enable_marker(int marker, bool enable);
	void DrawDisplay(int noisefloor);
	void ProcessWaterfall(const IQSampleVector &input);
	void set_signal_strength(double strength);
	double get_signal_strength() { return signal_strength; }
	void SetFftParts();
	float getSuppression();
	void set_active_marker(int marker, bool active);
	void enable_second_data_series(bool enable);

	static constexpr auto draw_event_cb = EventHandler<Spectrum, &Spectrum::draw_event_cb_class>::staticHandler;
	static constexpr auto click_event_cb = EventHandler<Spectrum, &Spectrum::click_event_cb_class>::staticHandler;
	static constexpr auto pressing_event_cb = EventHandler<Spectrum, &Spectrum::pressing_event_cb_class>::staticHandler;
	
};



template <typename _Real>
static inline void rotshift(std::complex<_Real> *complexVector, const size_t count)
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
static inline void irotshift(std::complex<_Real> *complexVector, const size_t count)
{
	int center = (int)floor((float)count / 2);
	// odd: 01 234 changes to 234 01
	std::rotate(complexVector, complexVector + center, complexVector + count);
}

extern Spectrum SpectrumGraph;

IQSample::value_type rms_level_approx(const IQSampleVector &samples);
