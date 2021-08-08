#pragma once
#include <string>

#define	MAX_NUM_BAND	15

struct vfo_settings_struct
{
	long long			vfo_freq[2];
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
};

class CVfo
{
public:
	CVfo();
	
	void vfo_init(long long freq);
	void set_vfo_capability(struct device_structure *sdr_dev);
	int	 set_vfo(long long freq);
	void step_vfo(long icount);
	long get_active_vfo();
	std::string get_vfo_str();
	void set_tuner_offset(double offset);
	void set_active_vfo(int active_vfo);
	void set_vfo_range(long long low, long long high);
	void set_band(int band, long long freq);

private:
	struct vfo_settings_struct	vfo_setting;	
};

extern CVfo	vfo;			


