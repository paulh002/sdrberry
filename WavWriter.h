#pragma once
#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <cstdint>
#include <cstddef>

class WavWriter
{
  public:
	WavWriter() = default;
	~WavWriter() { close(); }

	// Disable copying to prevent double-close or undefined behavior
	WavWriter(const WavWriter &) = delete;
	WavWriter &operator=(const WavWriter &) = delete;

	/**
	 * @brief Create a new WAV file with specified parameters
	 * @param filename Output path
	 * @param sampleRate Sample rate in Hz (e.g., 48000)
	 * @param numChannels Number of audio channels (1=mono, 2=stereo)
	 * @return true if file was successfully opened and header written
	 */
	bool create(const std::string &filename, uint32_t sampleRate, uint16_t numChannels);

	/**
	 * @brief Append floating-point samples to the file
	 * @param samples Vector of float samples (interleaved if multi-channel)
	 * @return true if write succeeded
	 */
	bool addSamples(const std::vector<float> &samples);

	/**
	 * @brief Append floating-point samples to the file (pointer interface)
	 * @param samples Pointer to float buffer
	 * @param count Number of samples to write
	 * @return true if write succeeded
	 */
	bool addSamples(const float *samples, size_t count);
	/**
	   * @brief Explicitly close and finalize the WAV file
	   */
	void close();

  private:
	std::ofstream file;
	uint32_t sampleRate = 0;
	uint16_t numChannels = 0;
	size_t totalSamples = 0;
	uint32_t totalDataBytes = 0;
	bool isOpen = false;

	void updateChunkSizes();
	// Little-endian writers for cross-platform compatibility
	void writeLE(std::ostream &os, uint16_t value);
	void writeLE(std::ostream &os, uint32_t value);
};
