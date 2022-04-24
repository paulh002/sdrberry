#pragma once
#include <string>
#include <mutex>
#include <atomic>
#include <unistd.h>
#include <mutex>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "lvgl/lvgl.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"
#include "Settings.h"
#include "gui_top_bar.h"
#include "sdrstream.h"
#include "gui_vfo.h"
#include "SdrDevice.h"

#define	MAX_NUM_BAND	15
struct bands
{
	int			meters;
	string		labels;
	long		f_low;
	long		f_high;
	int			f_mode;
} ;

struct vfo_settings_struct
{
	long long			vfo_freq[2];
	long long			vfo_freq_sdr[2];
	long long			vfo_low;
	long long			vfo_high;
	int					mode[2];
	int					band[2];
	int					frq_step;
	int					active_vfo;
	unsigned long		current_freq_vfo1[MAX_NUM_BAND];
	unsigned long		current_freq_vfo2[MAX_NUM_BAND];
	double				tuner_offset; 
	bool				tx;
	bool				rx;
	long				m_max_offset;
	long				m_offset[2];
	vector<bands>		m_bands;
	long				pcmrate;
};

class CVfo
{
public:
	CVfo();
	
	void vfo_init(long ifrate, long pcmrate, SdrDeviceVector *fSdrDevices, std::string fradio, int frx_channel, int ftx_channel);
	void vfo_re_init(long ifrate, long pcmrate);
	int	 set_vfo(long long freq, bool lock);
	void set_freq_to_sdr();
	void step_vfo(long icount, bool lock);
	long get_active_vfo_freq();
	std::string get_vfo_str();
	void set_tuner_offset(double offset);
	void set_active_vfo(int active_vfo);
	void set_vfo_range(long long low, long long high);
	void set_band(int band, long long freq);
	void sync_rx_vfo();
	void vfo_rxtx(bool brx, bool btx);
	void set_step(int step, int delay) {vfo_setting.frq_step = step; m_delay = delay; };
	void check_band(int dir, long long& freq);
	int	 getBandIndex(int band);
	int get_band_no(int vfo) {if (vfo < 2 && vfo >= 0) return vfo_setting.band[vfo]; else return 0;}
	int get_mode_no(int vfo){if (vfo < 2 && vfo >= 0) return vfo_setting.mode[vfo]; else return 0;}
	void set_mode(int vfo, int mode) {if (vfo < 2 && vfo >= 0) vfo_setting.mode[vfo] = mode;}
	bool get_rx() {return vfo_setting.rx;}
	bool get_tx() {return vfo_setting.tx;}
	int get_active_vfo() { return vfo_setting.active_vfo; }
	long long get_sdr_frequency()
	{
		return vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo];
	}
	int		get_band_in_meters()
	{
		return vfo_setting.band[vfo_setting.active_vfo];
	}
		
	long long get_tx_frequency();
	long	get_vfo_offset();
	void	return_bands(vector<int> &bands);
		
	std::atomic_bool tune_flag {false};
	bool	limit_ham_band;
	

  private:
	struct vfo_settings_struct	vfo_setting;
	int							m_delay_counter = 0;
	int							m_delay;
	std::mutex					m_vfo_mutex;
	SdrDeviceVector				*SdrDevices {nullptr};
	std::string					radio;
	int							rx_channel, tx_channel;
	long						offset_frequency;

	int							get_band(int active_vfo);
	void						rx_set_sdr_freq();
	void						tx_set_sdr_freq();
};

extern CVfo	vfo;			


