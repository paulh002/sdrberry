#pragma once
#include <complex>
#include <vector>

class Agc_class
{
public:
	void	execute(std::complex<float> in, std::complex<float> &out);
	void	execute_vector(std::vector<std::complex<float>> &vec);
	int		set_bandwidth(float b);
	float	get_signal_level();
	void	set_signal_level(float s);
	float	get_rssi();
	float	get_urssi();
	void	set_rssi(float rssi);
	float	get_gain();
	float	get_scale();
	int		set_scale(float _scale);
	int		init(std::vector<std::complex<float>> _x);
	int		set_gain_limits(float max, float min);
	int		set_enery_levels(float _e0, float _e1);
	int		reset();
	void	print();
		
private: 
	float				gain {1.0f};		// current gain value
	float				prime {1.0f};		// filtered output signal energy estimate
	float				alpha {1.0f};		// feed-back gain
	float				bandwidth {1e-2f};  // bandwidth-time constant
	float				scale {1.0f};		// output scale value
	float				min_gain {1e-6f};
	float				max_gain {1e6f};
	bool				is_locked {false};
	float				gain_unlimted {1.0f};
	float				e0 {1e-6};
	float				e1 {1.0f};
};

