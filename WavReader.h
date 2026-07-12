#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <cstdint>
#include <cstring>
#include <span>

class WavReader
{
  public:
	WavReader() = default;

	/**
	 * @brief Load a 16-bit mono WAV file into a float vector.
	 * @param filename Path to the WAV file
	 * @param outSamples Vector that will receive the normalized float samples
	 * @return true if file was successfully parsed, validated, and loaded
	 */
	bool open(const std::string &filename);
	bool readChunk(std::span<float> &outBuffer, size_t maxSamples);
	void close();
	bool isOpen() const { return isOpen_; }
	bool isEOF() const { return isEOF_; }
	uint32_t getSampleRate() const { return sampleRate_; }
	size_t getTotalSamples() const { return totalSamples_; }
	size_t getRemainingSamples() const { return remainingSamples_; }

  private:
	std::ifstream file_;
	uint32_t sampleRate_ = 0;
	size_t totalSamples_ = 0;
	size_t remainingSamples_ = 0;
	bool isOpen_ = false;
	bool isEOF_ = false;
	std::vector<int16_t> pcmBuffer_; // Reusable internal buffer

	template <typename T> bool readLE(T &val);
};

