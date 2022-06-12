#include "LMSNoisereducer.h"

LMSNoisereducer::LMSNoisereducer(){
	q = nullptr;
	q = eqlms_cccf_create(NULL, p);
	eqlms_cccf_set_bw(q, mu);
	eqlms_cccf_print(q);
	first = false;
	for (int i; i < 200; i++)
	{
		samples_training.push_back(0);
	}
}

LMSNoisereducer::~LMSNoisereducer()
{
	eqlms_cccf_destroy(q);
}

void LMSNoisereducer::Process(const IQSampleVector &samples_in, IQSampleVector &samples_out)
{
	unsigned int i = 0;

	/*if (samples_training.size() < samples_in.size())
	{
		samples_training.clear();
		for (auto con : samples_in)
		{
			samples_training.push_back(con);
		}
	}
*/
	
	
	for (auto con : samples_in)		
	{
		IQSample s;
		// push input sample
		eqlms_cccf_push(q, con);
		samples_training.push_back(con);
		// compute output sample
		eqlms_cccf_execute(q, &s);
		samples_out.push_back(s);
		// update internal weights
		s = samples_training.front();
		samples_training.pop_front();
		eqlms_cccf_step(q, con, s);
		//eqlms_cccf_step(q, samples_training[i], s);
		//eqlms_cccf_step_blind(q, s);
		//samples_training[i++] = con;
	}
}

LMSNoiseReduction::LMSNoiseReduction(int LMS_nr_strength)
{
	

	//LMS_Norm_instance.numTaps = calc_taps;
	//LMS_Norm_instance.pCoeffs = LMS_NormCoeff_f32;
	//LMS_Norm_instance.pState = LMS_StateF32;

	// Calculate "mu" (convergence rate) from user "DSP Strength" setting.  This needs to be significantly de-linearized to
	// squeeze a wide range of adjustment (e.g. several magnitudes) into a fairly small numerical range.
	mu_calc = LMS_nr_strength; // get user setting

	// New DSP NR "mu" calculation method as of 0.0.214
	mu_calc /= 2;				 // scale input value
	mu_calc += 2;				 // offset zero value
	mu_calc /= 10;				 // convert from "bels" to "deci-bels"
	mu_calc = powf(10, mu_calc); // convert to ratio
	mu_calc = 1 / mu_calc;		 // invert to fraction
	//LMS_Norm_instance.mu = mu_calc;
}

Xanr::Xanr()
{
}

Xanr::~Xanr()
{
	
}

void Xanr::Process(const SampleVector &samples_in, SampleVector &samples_out)
{
	int idx;
	float c0, c1;
	float y, error, sigma, inv_sigp;
	float nel, nev;

	for (int i = 0; i < samples_in.size(); i++)
	{
		ANR_d[ANR_in_idx] = samples_in[i];

		y = 0;
		sigma = 0;

		for (int j = 0; j < ANR_taps; j++)
		{
			idx = (ANR_in_idx + j + ANR_delay) & ANR_mask;
			y += ANR_w[j] * ANR_d[idx];
			sigma += ANR_d[idx] * ANR_d[idx];
		}
		inv_sigp = 1.0 / (sigma + 1e-10);
		error = ANR_d[ANR_in_idx] - y;

		if (ANR_notch)
		{
			//complex<float> q;
			//q.real(samples_in[i].real());
			//q.imag(error);
			samples_out.push_back(error);
		}
		else
		{
			//complex<float> q;
			//q.real(samples_in[i].real());
			//q.imag(y);
			samples_out.push_back(y);
		}

		if ((nel = error * (1.0 - ANR_two_mu * sigma * inv_sigp)) < 0.0)
			nel = -nel;
		if ((nev = ANR_d[ANR_in_idx] - (1.0 - ANR_two_mu * ANR_ngamma) * y - ANR_two_mu * error * sigma * inv_sigp) < 0.0)
			nev = -nev;
		if (nev < nel)
		{
			if ((ANR_lidx += ANR_lincr) > ANR_lidx_max)
				ANR_lidx = ANR_lidx_max;
			else if ((ANR_lidx -= ANR_ldecr) < ANR_lidx_min)
				ANR_lidx = ANR_lidx_min;
		}
		ANR_ngamma = ANR_gamma * (ANR_lidx * ANR_lidx) * (ANR_lidx * ANR_lidx) * ANR_den_mult;

		c0 = 1.0 - ANR_two_mu * ANR_ngamma;
		c1 = ANR_two_mu * error * inv_sigp;

		for (int j = 0; j < ANR_taps; j++)
		{
			idx = (ANR_in_idx + j + ANR_delay) & ANR_mask;
			ANR_w[j] = c0 * ANR_w[j] + c1 * ANR_d[idx];
		}
		ANR_in_idx = (ANR_in_idx + ANR_mask) & ANR_mask;
	}
}