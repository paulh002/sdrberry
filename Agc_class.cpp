#include "Agc_class.h"
#include <complex.h>

void Agc_class::execute(std::complex<float> in, std::complex<float> &out)
{
	// apply gain to input sample
	out = in * gain;
	// compute output signal energy
	float y2 = std::real(out * std::conj(out));
	// smooth energy estimate using single-pole low-pass filter
	prime = (1.0 - alpha)* prime + alpha*y2;

	// return if locked
	if (is_locked)
		return ;
	
	// update gain according to output energy
	if (prime > e0)
		gain *= expf(-0.5f * alpha * logf(prime));
	gain_unlimted = gain;
	// clamp to 120 dB gain
	if (gain > max_gain)
		gain = max_gain;
	
	out *= scale;
}

void Agc_class::execute_vector(std::vector<std::complex<float>> &vec)
{
	std::complex<float> out;
	for (auto& col : vec)
	{
		execute(col, out);
		col = out;
	}
}

int Agc_class::set_enery_levels(float _e0, float _e1)
{
	if (_e0 < 0)
		return -1;
	if (_e0 > 1.0f)
		return -1;
	if (_e1 < 0)
		return -1;
	if (_e1 > 1.0f)
		return -1;
	
	e0 = _e0;
	e1 = _e1;
	return 0;
}

int Agc_class::set_bandwidth(float b)
{
	// check to ensure bandwidth is reasonable
	if (b < 0)
		return -1;
	if (b > 1.0f)
		return -1;
	bandwidth = b;
	alpha = b;
	return 0;
}

float Agc_class::get_signal_level()
{
	return 1.0f / gain;
}

void Agc_class::set_signal_level(float s)
{
	if (s < 0.0)
		return ;
	gain = 1.0f / s;
	prime = 1.0f;
}

float Agc_class::get_rssi()
{
	return -20*log10(gain);
}

float Agc_class::get_urssi()
{
	return -20*log10(gain_unlimted);
}

	// set estimated signal level (dB)
void Agc_class::set_rssi(float rssi)
{
	// set internal gain appropriately
	gain = powf(10.0f, rssi / 20.0f);

	// ensure resulting gain is not arbitrarily low
	if (gain < min_gain)
		gain = min_gain;

	// reset internal output signal level
	prime = 1.0f;
}

float Agc_class::get_gain()
{
	return gain;
}

float Agc_class::get_scale()
{
	return scale;
}

int Agc_class::set_scale(float _scale)
{
	// check to ensure gain is reasonable
	if (scale <= 0)
		return -1;

	// set internal gain appropriately
	scale = _scale;
	return 0;
}

//  _x      : input data vector, [size: _n x 1]
//  _n      : number of input, output samples
int Agc_class::init(std::vector<std::complex<float>> _x)
{
	// ensure number of samples is greater than zero
	if (_x.size() == 0)
		return -1;

	// compute sum squares on input
	float x2 = 0;
	for (auto col : _x)
		x2 += real(col*conj(col));

	// compute RMS level and ensure result is positive
	x2 = sqrtf(x2 / (float) _x.size()) + 1e-16f;

	// set internal gain based on estimated signal level
	set_signal_level(x2);
	return 0;
}

int	Agc_class::set_gain_limits(float max, float min)
{
	if (max < 1e-6 || max > 1e6)
		return - 1;
	if (min < 1e-6 || min > 1e6)
		return -1;
	max_gain = max;
	min_gain = min;
	return 0;
}

int Agc_class::reset()
{
	// reset gain estimate
	gain = 1.0f;

	// reset signal level estimate
	prime = 1.0f;

	return 0;
}

void Agc_class::print()
{
	printf("agc [rssi: %12.4f dB, urssi: %12.4f dB , output gain: %.3f dB, bw: %12.4e, locked: %s]:\n",
		get_rssi(),
		get_urssi(),
		scale > 0 ? 10.*log10f(scale) : -100.0f,
		bandwidth,
		is_locked ? "yes" : "no");
}