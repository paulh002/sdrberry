#pragma once
#include <stdint.h>
#include <type_traits>
#include <limits>
#include <cstdint> // uint_fast16_t


template <uint8_t N, class input_t = uint32_t, class sum_t = uint32_t>
class SMA
{
  public:
	input_t operator()(input_t input)
	{
		sum -= previousInputs[index];
		sum += input;
		previousInputs[index] = input;
		if (++index == N)
			index = 0;
		return (sum + (N / 2)) / N;
	}

	static_assert(
		sum_t(0) < sum_t(-1), // Check that `sum_t` is an unsigned type
		"Error: sum data type should be an unsigned integer, otherwise, "
		"the rounding operation in the return statement is invalid.");

  private:
	uint8_t index = 0;
	input_t previousInputs[N] = {};
	sum_t sum = 0;
};

template <uint8_t K,
		  class input_t = uint_fast16_t,
		  class state_t = std::make_unsigned_t<input_t>>
class EMA
{
  public:
	/// Constructor: initialize filter to zero or optional given value.
	EMA(input_t initial = input_t(0))
		: state(zero + (state_t(initial) << K) - initial) {}

	/// Update the filter with the given input and return the filtered output.
	input_t operator()(input_t input)
	{
		state += state_t(input);
		state_t output = (state + half) >> K;
		output -= zero >> K;
		state -= output;
		return input_t(output);
	}

	constexpr static state_t
		max_state = std::numeric_limits<state_t>::max(),
		half_state = max_state / 2 + 1,
		zero = std::is_unsigned<input_t>::value ? state_t(0) : half_state,
		half = K > 0 ? state_t(1) << (K - 1) : state_t(0);

	static_assert(std::is_unsigned<state_t>::value,
				  "state type should be unsigned");

	static_assert(max_state >= std::numeric_limits<input_t>::max(),
				  "state type cannot be narrower than input type");

	/// Verify the input range to make sure it's compatible with the shift
	/// factor and the width of the state type.
	template <class T>
	constexpr static bool supports_range(T min, T max)
	{
		using sstate_t = std::make_signed_t<state_t>;
		return min <= max &&
			   min >= std::numeric_limits<input_t>::min() &&
			   max <= std::numeric_limits<input_t>::max() &&
			   (std::is_unsigned<input_t>::value
					? state_t(max) <= (max_state >> K)
					: min >= -sstate_t(max_state >> (K + 1)) - 1 &&
						  max <= sstate_t(max_state >> (K + 1)));
	}

  private:
	state_t state;
};