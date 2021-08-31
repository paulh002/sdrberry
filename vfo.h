#pragma once
#include <string>
#include <mutex>
#include <atomic>


#define	MAX_NUM_BAND	15

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
	struct				device_structure *sdr_dev;
	double				tuner_offset; 
	bool				tx;
	bool				rx;
	long				m_max_offset;
	long				m_offset[2];
};

class CVfo
{
public:
	CVfo();
	
	void vfo_init(long long freq, long ifrate);
	void set_vfo_capability(struct device_structure *sdr_dev);
	int	 set_vfo(long long freq, bool lock);
	void step_vfo(long icount, bool lock);
	long get_active_vfo();
	std::string get_vfo_str();
	void set_tuner_offset(double offset);
	void set_active_vfo(int active_vfo);
	void set_vfo_range(long long low, long long high);
	void set_band(int band, long long freq);
	void sync_rx_vfo();
	void vfo_rxtx(bool brx, bool btx);
	void set_step(int step, int delay) {vfo_setting.frq_step = step; m_delay = delay; };
	long long get_sdr_frequency()
	{
		return vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo];
	}
	
	long long get_tx_frequency()
	{
		return vfo_setting.vfo_freq[vfo_setting.active_vfo];
	}
	long	get_vfo_offset();
	
	std::atomic_bool tune_flag {false};
	
private:
	struct vfo_settings_struct	vfo_setting;
	int		m_delay_counter = 0;
	int		m_delay;
	std::mutex	m_vfo_mutex;
};

extern CVfo	vfo;			


