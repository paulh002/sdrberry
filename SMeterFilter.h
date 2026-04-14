#pragma once
#include <algorithm>
#include <cmath>
#include <stdexcept>

class SMeterFilter
{
  public:
	// Construct S-meter filter
	// update_hz:  GUI/meter refresh rate (20-30 Hz recommended)
	// attack_ms:  Rise time constant (10-40 ms)
	// release_ms: Fall time constant (150-450 ms)
	SMeterFilter(float update_hz = 30.0f,
				 float attack_ms = 25.0f,
				 float release_ms = 280.0f)
	{
		configure(update_hz, attack_ms, release_ms);
	}

	// Reconfigure time constants at runtime (e.g., on mode change)
	void configure(float update_hz, float attack_ms, float release_ms)
	{
		if (update_hz <= 0.0f || attack_ms <= 0.0f || release_ms <= 0.0f)
		{
			throw std::invalid_argument("All parameters must be > 0");
		}

		update_hz_ = update_hz;
		attack_ms_ = attack_ms;
		release_ms_ = release_ms;

		const float dt = 1.0f / update_hz_;
		const float tau_attack = attack_ms * 1e-3f;
		const float tau_release = release_ms * 1e-3f;

		// Discrete RC approximation: alpha = 1 - exp(-dt/tau)
		alpha_attack_ = 1.0f - std::exp(-dt / tau_attack);
		alpha_release_ = 1.0f - std::exp(-dt / tau_release);
	}

	// Process single raw meter sample (expects dB scale)
	float process(float input) noexcept
	{
		if (input > value_)
		{
			value_ += alpha_attack_ * (input - value_);
		}
		else
		{
			value_ += alpha_release_ * (input - value_);
		}
		return value_;
	}

	// Direct access to current smoothed value
	float current_value() const noexcept { return value_; }

	// Reset filter state (useful on band/mode change)
	void reset(float initial_value = -130.0f) noexcept { value_ = initial_value; }

  private:
	float value_ = -130.0f;
	float alpha_attack_ = 0.0f;
	float alpha_release_ = 0.0f;
	float update_hz_ = 30.0f;
	float attack_ms_ = 25.0f;
	float release_ms_ = 280.0f;
};