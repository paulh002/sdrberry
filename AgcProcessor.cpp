#include "AgcProcessor.h"

AgcProcessor::AgcProcessor(float bandwidth)
{

	agc_object = agc_crcf_create();	  // create object
	agc_crcf_set_bandwidth(agc_object, bandwidth); // set loop filter bandwidth
	agc_crcf_set_signal_level(agc_object, 1e-3f);
	agc_crcf_squelch_set_threshold(agc_object, -50); // threshold for detection [dB]
	agc_crcf_squelch_set_timeout(agc_object, 100);	// timeout for hysteresis
	squelch_enabled = false;
}

AgcProcessor::~AgcProcessor()
{
	if (agc_object)
	{
		agc_crcf_destroy(agc_object);
	}
}

void AgcProcessor::Process(IQSampleVector &samples_in)
{
	if (agc_object)
	{
		for (auto col : samples_in)
		{
			agc_crcf_execute(agc_object, col, &col);
		}
	}
}

void AgcProcessor::SetSquelch(bool squelch)
{
	squelch_enabled = squelch;
	if (agc_object)
	{
		if (squelch)
			agc_crcf_squelch_enable(agc_object);
		else
			agc_crcf_squelch_disable(agc_object);
	}
}

void AgcProcessor::SetSquelchThreshold(float _threshold)
{
	threshold = _threshold;
	if (agc_object)
		agc_crcf_squelch_set_threshold(agc_object, (float)threshold / 10.0);
}

float AgcProcessor::getRssi()
{
	if (agc_object)
		return agc_crcf_get_rssi(agc_object);
	else
		return -100.0;
}

void AgcProcessor::Lock(bool lock)
{
	if (agc_object)
	{
		if (lock)
			agc_crcf_lock(agc_object);
		else
			agc_crcf_unlock(agc_object);
	}
}

void AgcProcessor::print()
{
	agc_crcf_print(agc_object);
	int mode = agc_crcf_squelch_get_status(agc_object);
	char mode_str[80];
	
	switch (mode)
	{
	case LIQUID_AGC_SQUELCH_ENABLED:
		sprintf(mode_str, "squelch enabled");
		break;
	case LIQUID_AGC_SQUELCH_RISE:
		sprintf(mode_str, "signal detected");
		break;
	case LIQUID_AGC_SQUELCH_SIGNALHI:
		sprintf(mode_str, "signal high");
		break;
	case LIQUID_AGC_SQUELCH_FALL:
		sprintf(mode_str, "signal falling");
		break;
	case LIQUID_AGC_SQUELCH_SIGNALLO:
		sprintf(mode_str, "signal low");
		break;
	case LIQUID_AGC_SQUELCH_TIMEOUT:
		sprintf(mode_str, "signal timed out");
		break;
	case LIQUID_AGC_SQUELCH_DISABLED:
		sprintf(mode_str, "squelch disabled");
		break;
	default:
		sprintf(mode_str, "(unknown)");
		break;
	}
	printf("%18s\n", mode_str);
}

bool AgcProcessor::squelch()
{
	if (agc_crcf_squelch_get_status(agc_object) == LIQUID_AGC_SQUELCH_ENABLED)
		return true;
	return false;
}

void AgcProcessor::set_bandwidth(float bt)
{
	if (bt <= 1.0 && bt > 0.0)
		agc_crcf_set_bandwidth(agc_object, bt);
}
