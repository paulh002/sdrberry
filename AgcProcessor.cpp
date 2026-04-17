#include "AgcProcessor.h"

AgcProcessor::AgcProcessor()
{
	scale = 0.0000001f;
	max_gain = 1.0f;
	scale_f = 1.0;
	agc_object = agc_rrrf_create();	  // create object
	agc_rrrf_set_bandwidth(agc_object, 0.01); // set loop filter bandwidth
	agc_rrrf_set_signal_level(agc_object, 1e-3f);
	agc_rrrf_squelch_set_threshold(agc_object, -50); // threshold for detection [dB]
	agc_rrrf_squelch_set_timeout(agc_object, 100);	// timeout for hysteresis
	agc_rrrf_set_scale(agc_object, scale);
	agc_rrrf_set_gain(agc_object, 1.0);

	squelch_enabled = false;
}

AgcProcessor::~AgcProcessor()
{
	if (agc_object)
	{
		agc_rrrf_destroy(agc_object);
	}
}

void AgcProcessor::Process(SampleVector &samples_in)
{
	if (agc_object && (agc_rrrf_squelch_get_status(agc_object) == LIQUID_AGC_SQUELCH_DISABLED))
	{
		for (auto &col : samples_in)
		{
			agc_rrrf_execute(agc_object, col, &col);
		}
	}
	else if (agc_object)
	{
		for (auto col : samples_in)
		{
			agc_rrrf_execute(agc_object, col, &col);
		}
	}
}

void AgcProcessor::SetSquelch(bool squelch)
{
	squelch_enabled = squelch;
	if (agc_object)
	{
		if (squelch)
			agc_rrrf_squelch_enable(agc_object);
		else
			agc_rrrf_squelch_disable(agc_object);
	}
}

void AgcProcessor::SetSquelchThreshold(float _threshold)
{
	threshold = _threshold;
	if (agc_object)
		agc_rrrf_squelch_set_threshold(agc_object, (float)threshold / 10.0);
}

float AgcProcessor::getRssi()
{
	if (agc_object)
		return agc_rrrf_get_rssi(agc_object);
	else
		return -100.0;
}

void AgcProcessor::Lock(bool lock)
{
	if (agc_object)
	{
		if (lock)
			agc_rrrf_lock(agc_object);
		else
			agc_rrrf_unlock(agc_object);
	}
}

void AgcProcessor::print()
{
	agc_rrrf_print(agc_object);
	printf("scale %f \n", scale_f);
}

bool AgcProcessor::squelch()
{
	if (agc_rrrf_squelch_get_status(agc_object) == LIQUID_AGC_SQUELCH_ENABLED)
		return true;
	return false;
}

void AgcProcessor::set_bandwidth(float bt)
{
	if (bt <= 1.0 && bt > 0.0)
		agc_rrrf_set_bandwidth(agc_object, bt);
}

void AgcProcessor::set_scale(int gain)
{
	scale_f = powf(10.0f, (float)gain / 200.0f);
	agc_rrrf_set_scale(agc_object, scale_f);
}
