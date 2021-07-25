#pragma once


#define	MAX_NUM_BAND	15

struct vfo_settings_struct
{
	unsigned long		vfo_freq1;
	unsigned long		vfo_freq2;
	int					mode[2];
	int					band[2];
	int					frq_step;
	unsigned long		current_freq_vfo1[MAX_NUM_BAND];
	unsigned long		current_freq_vfo2[MAX_NUM_BAND];
	struct				device_structure *sdr_dev;
};

extern struct vfo_settings_struct	vfo_setting;

void vfo_init(unsigned long freq);
void set_vfo_capability(struct device_structure *sdr_dev);
void set_vfo(int vfo, int band, unsigned long freq);
void step_vfo(int vfo, int icount);