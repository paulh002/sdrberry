#pragma once
#include <atomic>
#include <math.h>
#include <mutex>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include "SdrDevice.h"
#include "Settings.h"
#include "lvgl.h"



#define MAX_NUM_BAND 15

enum vfo_activevfo
{
	One = 0,
	Two = 1,
	None = -1
} ;

enum vfo_spansetting
{
	span_is_ifrate = 0,
	span_between_ifrate = 1,
	span_lower_halfrate = 2
};

struct bands_t
{
	int meters;
	string label;
	long long f_low;
	long long f_high;
	int f_mode;
};

struct vfo_settings_struct
{
	long long vfo_freq[2];
	long long vfo_freq_sdr[2];
	long long vfo_low;
	long long vfo_high;
	int vfo_rit[2];
	int mode[2];
	int band[2];
	int frq_step;
	int active_vfo;
	unsigned long current_freq_vfo1[MAX_NUM_BAND];
	unsigned long current_freq_vfo2[MAX_NUM_BAND];
	double tuner_offset;
	bool tx;
	bool rx;
	long max_offset;
	long min_offset;
	long offset[2];
	std::vector<bands_t> bands;
	long pcmrate;
	long span;
	long bandwidth;
	int correction_tx;
	int correction_rx;
	int notxoffset;
	long maxtxoffset;
	bool split;
};

class CVfo
{
  public:
	CVfo();

	void vfo_init(long ifrate, long pcmrate, long span, SdrDeviceVector *fSdrDevices, std::string fradio, int frx_channel, int ftx_channel);
	void vfo_re_init(long ifrate, long pcmrate, long span, long bandwidth);
	void set_span(long span);
	bool compare_span();
	std::pair<vfo_spansetting, double> compare_span_ex();
	int set_vfo(long long freq, vfo_activevfo ActiveVfo = vfo_activevfo::None);
	void step_vfo(long icount);
	long get_active_vfo_freq();
	std::string get_vfo_str();
	void set_tuner_offset(double offset);
	void set_active_vfo(int active_vfo);
	void set_vfo_range(long long low, long long high);
	void set_band(int band, long long freq);
	void set_band_freq(long long freq);
	void sync_rx_vfo();
	void vfo_rxtx(bool brx, bool btx, bool split = false);
	void pause_step(bool pause) {pausevfo = pause;}
	void set_step(int step, int delay);
	void setVfoFrequency(int direction);
	void check_band(int dir, long long &freq);
	int getBandIndex(int band);
	int get_band_no(int vfo);
	int get_mode_no(int vfo);
	int get_current_mode();
	void set_mode(int vfo, int mode);
	bool get_rx() { return vfo_setting.rx; }
	bool get_tx() { return vfo_setting.tx; }
	int get_active_vfo() { return vfo_setting.active_vfo; }
	long get_maxoffset() { return vfo_setting.max_offset; }
	long get_minoffset() { return vfo_setting.min_offset * -1; }
	long long get_sdr_span_frequency();
	long long get_sdr_frequency()
	{
		return vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo];
	}
	int get_band_in_meters()
	{
		return vfo_setting.band[vfo_setting.active_vfo];
	}

	std::string get_band_in_text();
	
	int getCurrentBandIndex()
	{
		return getBandIndex(get_band_in_meters());
	}
	long long get_frequency()
	{
		return vfo_setting.vfo_freq[vfo_setting.active_vfo];
	}

	long long get_tx_frequency();
	long get_vfo_offset(bool rit = false);
	long get_vfo_offset_tx();
	long get_vfo_absolute_offset();
	void return_bands(vector<int> &bands);
	long get_span() { return vfo_setting.span;}
	void updateweb();
	bool is_vfo_limit_ham_band() { return limit_ham_band; }
	void set_vfo_limit_ham_band(bool value) { limit_ham_band = value; }
	std::atomic_bool tune_flag{false};
	
	void setRit(int rit, int active_vfo);
	std::string getMode(int active_vfo);
	std::vector<int16_t> Legend();
	void set_frequency_to_left(long long freq, int active_vfo, bool update );
	bool checkVfoBandRange(long long freq);
	
  private:
	struct vfo_settings_struct vfo_setting;
	int m_delay_counter = 0;
	int m_delay;
	std::mutex m_vfo_mutex;
	SdrDeviceVector *SdrDevices{nullptr};
	std::string radio;
	int rx_channel, tx_channel;
	long span_offset_frequency;
	bool pausevfo{false};
	bool limit_ham_band;
	
	int get_band(int active_vfo);
	void rx_set_sdr_freq();
	void tx_set_sdr_freq();
	
};

extern CVfo vfo;
