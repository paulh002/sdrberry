#include "SpectralNoiseReduction.h"

/************************************************************************************
	Adapted from T41-ep AC8GY and WTEE

	Noise reduction with spectral subtraction rule
    based on Romanin et al. 2009 & Schmitt et al. 2002
    and MATLAB voicebox
    and Gerkmann & Hendriks 2002
    and Yao et al. 2016

   STAND: UHSDR github 14.1.2018
************************************************************************************/


SpectralNoiseReduction::SpectralNoiseReduction(float pcmrate, tuple<float,float> bw)
{
	SampleRate = pcmrate;
	NR_first_time_2 = 1;
	NR_init_counter = 0;
	DF = pcmrate / 12000.0; // decimation factor (48000/12000)
	bandwidth = bw;
	SpectralNoiseReductionInit();
}


void SpectralNoiseReduction::SpectralNoiseReductionInit()
{
	for (int bindx = 0; bindx < NR_FFT_L / 2; bindx++)
	{
		NR_last_sample_buffer_L[bindx] = 0.1;
		NR_Hk_old[bindx] = 0.1; // old gain
		NR_Nest[bindx][0] = 0.01;
		NR_Nest[bindx][1] = 0.015;
		NR_Gts[bindx][1] = 0.1;
		//NR_M[bindx] = 500.0;
		//NR_E[bindx][0] = 0.1;
		NR_X[bindx][1] = 0.5;
		NR_SNR_post[bindx] = 2.0;
		NR_SNR_prio[bindx] = 1.0;
		//NR_first_time = 2;
		NR_long_tone_gain[bindx] = 1.0;
		NR_last_iFFT_result[bindx] = 0;
	}
}

void SpectralNoiseReduction::Process(const SampleVector &samples_in, SampleVector &samples_out)
{
	
	uint8_t VAD_low = 0;
	uint8_t VAD_high = 127;
	float lf_freq; // = (offset - width/2) / (12000 / NR_FFT_L); // bin BW is 46.9Hz [12000Hz / 256 bins] @96kHz
	float uf_freq; //= (offset + width/2) / (12000 / NR_FFT_L);

	// Need a total of 512 samples
	if (samples_in.size() != NR_FFT_L)
		return;
	
	ax = expf(-tinc / tax);
	ap = expf(-tinc / tap);
	xih1 = powf(10, (float)asnr / 10.0);
	
	xih1r = 1.0 / (1.0 + xih1) - 1.0;
	pfac = (1.0 / pspri - 1.0) * (1.0 + xih1);
	float snr_prio_min = powf(10, -(float)20 / 20.0);

	const int16_t NR_width = 4;
	const float power_threshold = 0.4;
	float ph1y[NR_FFT_L / 2];

	lf_freq = std::get<0>(bandwidth);
	uf_freq = std::get<1>(bandwidth);
	lf_freq /= ((SampleRate / DF) / NR_FFT_L); // bin BW is 46.9Hz [12000Hz / 256 bins] @96kHz
	uf_freq /= ((SampleRate / DF) / NR_FFT_L);

	// INITIALIZATION ONCE 1
	if (NR_first_time_2 == 1)
	{ // TODO: properly initialize all the variables
		for (int bindx = 0; bindx < NR_FFT_L / 2; bindx++)
		{
			NR_last_sample_buffer_L[bindx] = 0.0;
			NR_G[bindx] = 1.0;
			//xu[bindx] = 1.0;  //has to be replaced by other variable
			NR_Hk_old[bindx] = 1.0; // old gain or xu in development mode
			NR_Nest[bindx][0] = 0.0;
			NR_Nest[bindx][1] = 1.0;
			pslp[bindx] = 0.5;
		}
		NR_first_time_2 = 2; // we need to do some more a bit later down
	}

	for (int k = 0; k < 2; k++)
	{
		// NR_FFT_buffer is 512 floats big
		// interleaved r, i, r, i . . .
		// fill first half of FFT_buffer with last events audio samples
		for (int i = 0; i < NR_FFT_L / 2; i++)
		{
			NR_FFT_buffer[i].real(NR_last_sample_buffer_L[i]); // real
			NR_FFT_buffer[i].imag(0.0);					   // imaginary
		}
		// copy recent samples to last_sample_buffer for next time!
		for (int i = 0; i < NR_FFT_L / 2; i++)
		{
			NR_last_sample_buffer_L[i] = samples_in[i + k * (NR_FFT_L / 2)];
		}
		// now fill recent audio samples into second half of FFT_buffer
		for (int i = 0; i < NR_FFT_L / 2; i++)
		{
			NR_FFT_buffer[NR_FFT_L/2 + i].real(samples_in[i + k * (NR_FFT_L / 2)]); // real
			NR_FFT_buffer[NR_FFT_L/2 + i].imag(0.0);
		}

		for (int idx = 0; idx < NR_FFT_L; idx++)
		{ // sqrt Hann window
			float q = hann(idx , NR_FFT_L);
			NR_FFT_buffer[idx] *= q;
		}
		
		// execute fft 256 buckets
		fftplan plan = fft_create_plan(NR_FFT_L, NR_FFT_buffer.data(), NR_FFT_buffer1.data(), LIQUID_FFT_FORWARD, 0);
		fft_execute(plan);
		fft_destroy_plan(plan);

		std::memcpy(NR_FFT_buffer.data(), NR_FFT_buffer1.data(), NR_FFT_buffer1.size());

		for (int bindx = 0; bindx < NR_FFT_L / 2; bindx++)
		{
			// this is squared magnitude for the current frame
			NR_X[bindx][0] = (NR_FFT_buffer[bindx].real() * NR_FFT_buffer[bindx].real() + NR_FFT_buffer[bindx].imag() * NR_FFT_buffer[bindx].imag());
		}

		if (NR_first_time_2 == 2)
		{ // TODO: properly initialize all the variables
			for (int bindx = 0; bindx < NR_FFT_L / 2; bindx++)
			{
				NR_Nest[bindx][0] = NR_Nest[bindx][0] + 0.05 * NR_X[bindx][0]; // we do it 20 times to average over 20 frames for app. 100ms only on NR_on/bandswitch/modeswitch,...
				xt[bindx] = psini * NR_Nest[bindx][0];
			}
			NR_init_counter++;
			if (NR_init_counter > 19)
			{ //average over 20 frames for app. 100ms
				NR_init_counter = 0;
				NR_first_time_2 = 3; // now we did all the necessary initialization to actually start the noise reduction
			}
		}
		if (NR_first_time_2 == 3)
		{
			for (int bindx = 0; bindx < NR_FFT_L / 2; bindx++)
			{ // 1. Step of NR - calculate the SNR's
				ph1y[bindx] = 1.0 / (1.0 + pfac * expf(xih1r * NR_X[bindx][0] / xt[bindx]));
				pslp[bindx] = ap * pslp[bindx] + (1.0 - ap) * ph1y[bindx];

				if (pslp[bindx] > psthr)
				{
					ph1y[bindx] = 1.0 - pnsaf;
				}
				else
				{
					ph1y[bindx] = fmin(ph1y[bindx], 1.0);
				}
				xtr = (1.0 - ph1y[bindx]) * NR_X[bindx][0] + ph1y[bindx] * xt[bindx];
				xt[bindx] = ax * xt[bindx] + (1.0 - ax) * xtr;
			}
			for (int bindx = 0; bindx < NR_FFT_L / 2; bindx++)
			{																					   // 1. Step of NR - calculate the SNR's
				NR_SNR_post[bindx] = fmax(fmin(NR_X[bindx][0] / xt[bindx], 1000.0), snr_prio_min); // limited to +30 /-15 dB, might be still too much of reduction, let's try it?
				NR_SNR_prio[bindx] = fmax(NR_alpha * NR_Hk_old[bindx] + (1.0 - NR_alpha) * fmax(NR_SNR_post[bindx] - 1.0, 0.0), 0.0);
			}

			VAD_low = (int)lf_freq;
			VAD_high = (int)uf_freq;
			if (VAD_low == VAD_high)
			{
				VAD_high++;
			}
			if (VAD_low < 1)
			{
				VAD_low = 1;
			}
			else if (VAD_low > NR_FFT_L / 2 - 2)
			{
				VAD_low = NR_FFT_L / 2 - 2;
			}
			if (VAD_high < 1)
			{
				VAD_high = 1;
			}
			else if (VAD_high > NR_FFT_L / 2)
			{
				VAD_high = NR_FFT_L / 2;
			}

			float v;
			for (int bindx = VAD_low; bindx < VAD_high; bindx++)
			{ // maybe we should limit this to the signal containing bins (filtering!!)
				{
					v = NR_SNR_prio[bindx] * NR_SNR_post[bindx] / (1.0 + NR_SNR_prio[bindx]);
					NR_G[bindx] = 1.0 / NR_SNR_post[bindx] * sqrtf((0.7212 * v + v * v));
					NR_Hk_old[bindx] = NR_SNR_post[bindx] * NR_G[bindx] * NR_G[bindx]; //
				}

				// MUSICAL NOISE TREATMENT HERE, DL2FW

				// musical noise "artefact" reduction by dynamic averaging - depending on SNR ratio
				pre_power = 0.0;
				post_power = 0.0;
				for (int bindx = VAD_low; bindx < VAD_high; bindx++)
				{
					pre_power += NR_X[bindx][0];
					post_power += NR_G[bindx] * NR_G[bindx] * NR_X[bindx][0];
				}

				power_ratio = post_power / pre_power;
				if (power_ratio > power_threshold)
				{
					power_ratio = 1.0;
					NN = 1;
				}
				else
				{
					NN = 1 + 2 * (int)(0.5 + NR_width * (1.0 - power_ratio / power_threshold));
				}

				for (int bindx = VAD_low + NN / 2; bindx < VAD_high - NN / 2; bindx++)
				{
					NR_Nest[bindx][0] = 0.0;
					for (int m = bindx - NN / 2; m <= bindx + NN / 2; m++)
					{
						NR_Nest[bindx][0] += NR_G[m];
					}
					NR_Nest[bindx][0] /= (float)NN;
				}

				// and now the edges - only going NN steps forward and taking the average
				// lower edge
				for (int bindx = VAD_low; bindx < VAD_low + NN / 2; bindx++)
				{
					NR_Nest[bindx][0] = 0.0;
					for (int m = bindx; m < (bindx + NN); m++)
					{
						NR_Nest[bindx][0] += NR_G[m];
					}
					NR_Nest[bindx][0] /= (float)NN;
				}

				// upper edge - only going NN steps backward and taking the average
				for (int bindx = VAD_high - NN; bindx < VAD_high; bindx++)
				{
					NR_Nest[bindx][0] = 0.0;
					for (int m = bindx; m > (bindx - NN); m--)
					{
						NR_Nest[bindx][0] += NR_G[m];
					}
					NR_Nest[bindx][0] /= (float)NN;
				}

				// end of edge treatment

				for (int bindx = VAD_low + NN / 2; bindx < VAD_high - NN / 2; bindx++)
				{
					NR_G[bindx] = NR_Nest[bindx][0];
				}
				// end of musical noise reduction
			} //end of "if ts.nr_first_time == 3"

#if 1
			// FINAL SPECTRAL WEIGHTING: Multiply current FFT results with NR_FFT_buffer for 128 bins with the 128 bin-specific gain factors G
			//              for(int bindx = 0; bindx < NR_FFT_L / 2; bindx++)                 // try 128:
			for (int bindx = 0; bindx < NR_FFT_L / 2; bindx++)
			{																																		// try 128:
				NR_FFT_buffer[bindx].real(NR_FFT_buffer[bindx].real() * NR_G[bindx] * NR_long_tone_gain[bindx]);										// real part
				NR_FFT_buffer[bindx].imag(NR_FFT_buffer[bindx].imag() * NR_G[bindx] * NR_long_tone_gain[bindx]);								// imag part
				NR_FFT_buffer[NR_FFT_L - bindx -1].real(NR_FFT_buffer[NR_FFT_L - bindx -1].real() * NR_G[bindx] * NR_long_tone_gain[bindx]); // real part conjugate symmetric
				NR_FFT_buffer[NR_FFT_L - bindx -1].imag(NR_FFT_buffer[NR_FFT_L - bindx -1].imag() * NR_G[bindx] * NR_long_tone_gain[bindx]); // imag part conjugate symmetric
			}

#endif
			
			fftplan plan = fft_create_plan(NR_FFT_L, NR_FFT_buffer.data(), NR_FFT_buffer1.data(), LIQUID_FFT_BACKWARD, 0);
			fft_execute(plan);
			fft_destroy_plan(plan);
			
			for (int idx = 0; idx < NR_FFT_L; idx++)
			{ // sqrt Hann window
				float q = hann(idx, NR_FFT_L);
				NR_FFT_buffer[idx] = NR_FFT_buffer1[idx] * q ;
			
			}

			// do the overlap & add
			for (int i = 0; i < NR_FFT_L / 2; i++)
			{ // take real part of first half of current iFFT result and add to 2nd half of last iFFT_result
				samples_out.push_back(0.01 *(NR_FFT_buffer[i].real() + NR_last_iFFT_result[i]));
				//float_buffer_L[i + k * (NR_FFT_L / 2)] = NR_FFT_buffer[i].real() + NR_last_iFFT_result[i];
				//float_buffer_R[i + k * (NR_FFT_L / 2)] = float_buffer_L[i + k * (NR_FFT_L / 2)];
			}
			for (int i = 0; i < NR_FFT_L / 2; i++)
			{
				NR_last_iFFT_result[i] = NR_FFT_buffer[NR_FFT_L/2 + i].real();
			}
			// end of "for" loop which repeats the FFT_iFFT_chain two times !!!
		}
	}
}