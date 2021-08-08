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
};

extern struct vfo_settings_struct	vfo_setting;

void vfo_init(long long freq);
void set_vfo_capability(struct device_structure *sdr_dev);
void set_vfo(int vfo, int band, long long freq);
void step_vfo(int vfo, int icount);
long get_active_vfo();
std::string get_vfo_str(int vfo);