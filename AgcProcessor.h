#pragma once
#include "DataBuffer.h"
#include "SdrberryTypeDefs.h"
#include <cassert>
#include <cmath>
#include <complex>
#include <concepts>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <liquid/liquid.h>
#include <span>
#include <stdexcept>
#include <variant>
#include <vector>

// C++20 Concept: Restrict to liquid-dsp supported AGC sample types
template <typename T>
concept LiquidAgcSample = std::is_same_v<T, float> || std::is_same_v<T, std::complex<float>>;

// ============================================================================
// RAII Wrapper for liquid-dsp AGC
// ============================================================================
template <LiquidAgcSample T>
class AgcProcessor
{
  public:
	explicit AgcProcessor(float bandwidth = 0.01f)
	{
		scale = 0.0000001f;
		max_gain = 1.0f;
		scale_f = 1.0;
		if constexpr (std::is_same_v<T, float>)
		{
			handle_ = agc_rrrf_create();
			if (!handle_)
				throw std::runtime_error("agc_rrrf_create() failed");
			agc_rrrf_set_bandwidth(static_cast<agc_rrrf>(handle_), bandwidth);
			agc_rrrf_set_signal_level(static_cast<agc_rrrf>(handle_), 1e-3f);
			agc_rrrf_squelch_set_threshold(static_cast<agc_rrrf>(handle_), -50); // threshold for detection [dB]
			agc_rrrf_squelch_set_timeout(static_cast<agc_rrrf>(handle_), 100);	 // timeout for hysteresis
			agc_rrrf_set_scale(static_cast<agc_rrrf>(handle_), scale);
			agc_rrrf_set_gain(static_cast<agc_rrrf>(handle_), 1.0);
		}
		else
		{
			handle_ = agc_crcf_create();
			if (!handle_)
				throw std::runtime_error("agc_crcf_create() failed");
			agc_crcf_set_bandwidth(static_cast<agc_crcf>(handle_), bandwidth);
			agc_crcf_set_signal_level(static_cast<agc_crcf>(handle_), 1e-3f);
			agc_crcf_squelch_set_threshold(static_cast<agc_crcf>(handle_), -50); // threshold for detection [dB]
			agc_crcf_squelch_set_timeout(static_cast<agc_crcf>(handle_), 100);	 // timeout for hysteresis
			agc_crcf_set_scale(static_cast<agc_crcf>(handle_), scale);
			agc_crcf_set_gain(static_cast<agc_crcf>(handle_), 1.0);
		}
	}

	~AgcProcessor()
	{
		if (!handle_)
			return;
		if constexpr (std::is_same_v<T, float>)
		{
			agc_rrrf_destroy(static_cast<agc_rrrf>(handle_));
		}
		else
		{
			agc_crcf_destroy(static_cast<agc_crcf>(handle_));
		}
	}

	// Prevent copying (C handles are unique resources)
	AgcProcessor(const AgcProcessor &) = delete;
	AgcProcessor &operator=(const AgcProcessor &) = delete;

	// Allow moving
	AgcProcessor(AgcProcessor &&other) noexcept : handle_(other.handle_)
	{
		other.handle_ = nullptr;
	}
	AgcProcessor &operator=(AgcProcessor &&other) noexcept
	{
		if (this != &other)
		{
			this->~AgcProcessor();
			handle_ = other.handle_;
			other.handle_ = nullptr;
		}
		return *this;
	}

	// In-place processing (liquid-dsp supports in-place execution)
	void process(std::span<T> samples)
	{
		if (samples.empty())
			return;
		unsigned int n = static_cast<unsigned int>(samples.size());

		if constexpr (std::is_same_v<T, float>)
		{
			agc_rrrf_execute_block(static_cast<agc_rrrf>(handle_),
								   samples.data(), n, samples.data());
		}
		else
		{
			agc_crcf_execute_block(static_cast<agc_crcf>(handle_),
								   reinterpret_cast<liquid_float_complex *>(samples.data()), n,
								   reinterpret_cast<liquid_float_complex *>(samples.data()));
		}
	}

	// Optional: Access internal gain state (available in recent liquid-dsp versions)
	float get_current_gain() const
	{
		if constexpr (std::is_same_v<T, float>)
		{
			return agc_rrrf_get_gain(static_cast<agc_rrrf>(handle_));
		}
		else
		{
			return agc_crcf_get_gain(static_cast<agc_crcf>(handle_));
		}
	}

	void set_scale(int scale)
	{
		scale_f = powf(10.0f, (float)scale / 200.0f);
		if constexpr (std::is_same_v<T, float>)
			agc_rrrf_set_scale(static_cast<agc_rrrf>(handle_), scale_f);
		else
			agc_crcf_set_scale(static_cast<agc_crcf>(handle_), scale_f);
	}

	float getRssi() const
	{
		if constexpr (std::is_same_v<T, float>)
			return agc_rrrf_get_rssi(static_cast<agc_rrrf>(handle_));
		else
			return agc_crcf_get_rssi(static_cast<agc_crcf>(handle_));
	}

	void Lock(bool lock)
	{
		if constexpr (std::is_same_v<T, float>)
		{
			if (lock)
				agc_rrrf_lock(static_cast<agc_rrrf>(handle_));

			else
				agc_rrrf_unlock(static_cast<agc_rrrf>(handle_));
		}
		else
		{
			if (lock)
				agc_crcf_lock(static_cast<agc_crcf>(handle_));

			else
				agc_crcf_unlock(static_cast<agc_crcf>(handle_));
		}
	}

	void print() const
	{
		if constexpr (std::is_same_v<T, float>)
			agc_rrrf_print(static_cast<agc_rrrf>(handle_));
		else
			agc_crcf_print(static_cast<agc_crcf>(handle_));

		printf("scale %f \n", scale_f);
	}

	bool squelch() const
	{
		if constexpr (std::is_same_v<T, float>)
		{
			if (agc_rrrf_squelch_get_status(static_cast<agc_rrrf>(handle_)) == LIQUID_AGC_SQUELCH_ENABLED)
				return true;
		}
		else
		{
			if (agc_crcf_squelch_get_status(static_cast<agc_crcf>(handle_)) == LIQUID_AGC_SQUELCH_ENABLED)
				return true;
		}
		return false;
	}

	void SetSquelch(bool squelch)
	{
		if constexpr (std::is_same_v<T, float>)
		{
			if (squelch)
				agc_rrrf_squelch_enable(static_cast<agc_rrrf>(handle_));
			else
				agc_rrrf_squelch_disable(static_cast<agc_rrrf>(handle_));
		}
		else
		{
			if (squelch)
				agc_crcf_squelch_enable(static_cast<agc_crcf>(handle_));
			else
				agc_crcf_squelch_disable(static_cast<agc_crcf>(handle_));
		}
	}

	void SetSquelchThreshold(float _threshold)
	{
		if constexpr (std::is_same_v<T, float>)
		{
			agc_rrrf_squelch_set_threshold(static_cast<agc_rrrf>(handle_), (float)_threshold / 10.0);
		}
		else
		{
			agc_crcf_squelch_set_threshold(static_cast<agc_crcf>(handle_), (float)_threshold / 10.0);
		}
	}

	void set_bandwidth(float bt)
	{
		if (bt <= 1.0 && bt > 0.0)
		{
			if constexpr (std::is_same_v<T, float>)
				agc_rrrf_set_bandwidth(static_cast<agc_rrrf>(handle_), bt);
			else
				agc_crcf_set_bandwidth(static_cast<agc_crcf>(handle_), bt);
		}
	}

  private:
	void *handle_ = nullptr; // Opaque liquid-dsp handle
	float threshold;
	float scale;
	float max_gain;
	float scale_f;
};

// ============================================================================
// 2. Demodulator (Directly embeds AGC, no Squelch)
// ============================================================================
template <LiquidAgcSample T>
class AGCMultiProcessor
{
  public:
	using sample_type = T; // Exposed for std::visit type deduction

	explicit AGCMultiProcessor(float agc_bandwidth = 0.01f) : agc_(agc_bandwidth) {}

	// Process samples through AGC (add demod math before/after if needed)
	void process(std::span<T> samples)
	{
		agc_.process(samples);
	}

	// Access AGC for metering or parameter tuning
	AgcProcessor<T> &agc() { return agc_; }
	const AgcProcessor<T> &agc() const { return agc_; }

  private:
	AgcProcessor<T> agc_;
};

// ============================================================================
// 3. Runtime Selection Wrapper (Optional, if stream type changes at runtime)
// ============================================================================
enum class StreamMode
{
	RRRF,
	CRCF
};
using RrrfDemod = AGCMultiProcessor<float>;
using CrcfDemod = AGCMultiProcessor<std::complex<float>>;
using DemodVariant = std::variant<RrrfDemod, CrcfDemod>;

class AGCUnifiedProcessor
{
  public:
	AGCUnifiedProcessor(StreamMode mode, float agc_bw)
	{
		demod_ = (mode == StreamMode::CRCF)
					 ? DemodVariant{CrcfDemod{agc_bw}}
					 : DemodVariant{RrrfDemod{agc_bw}};
	}

	// Accept raw SDR byte buffers (matches librtlsdr/SoapySDR callbacks)
	void Process(std::span<std::byte> buffer, size_t num_samples)
	{
		std::visit([&buffer, num_samples](auto &d) {
			using T = typename std::decay_t<decltype(d)>::sample_type;
			std::span<T> samples(
				reinterpret_cast<T *>(buffer.data()),
				num_samples);
			d.process(samples);
		},
				   demod_);
	}

	void set_scale(int scale)
	{
		std::visit([&](auto &d) { d.agc().set_scale(scale); }, demod_);
	}

	void Lock(bool lock)
	{
		std::visit([&](auto &d) { d.agc().Lock(lock); }, demod_);
	}

	float getRssi() const
	{
		return std::visit([](const auto &d) { return d.agc().getRssi(); }, demod_);
	}

	float get_current_gain() const
	{
		return std::visit([](const auto &d) { return d.agc().get_current_gain(); }, demod_);
	}

	void print() const
	{
		std::visit([](const auto &d) { return d.agc().print(); }, demod_);
	}

	bool squelch() const
	{
		return std::visit([](const auto &d) { return d.agc().squelch(); }, demod_);
	}

	void set_bandwidth(float bt)
	{
		std::visit([&](auto &d) { d.agc().set_bandwidth(bt); }, demod_);
	}

	void SetSquelch(bool squelch)
	{
		std::visit([&](auto &d) { d.agc().SetSquelch(squelch); }, demod_);
	}

	void SetSquelchThreshold(float _threshold)
	{
		std::visit([&](auto &d) { d.agc().SetSquelchThreshold(_threshold); }, demod_);
	}

  private:
	DemodVariant demod_;
};
