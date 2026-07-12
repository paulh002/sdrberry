#include "WavWriter.h"

bool WavWriter::create(const std::string &filename, uint32_t sampleRate, uint16_t numChannels)
{
	if (isOpen)
		return false;

	file.open(filename, std::ios::binary);
	if (!file.is_open())
		return false;

	this->sampleRate = sampleRate;
	this->numChannels = numChannels;
	this->totalSamples = 0;
	isOpen = true;

	// Write WAV header with placeholders for chunk sizes
	file.write("RIFF", 4);
	writeLE(file, static_cast<uint32_t>(0)); // RIFF size (updated on close)
	file.write("WAVE", 4);

	file.write("fmt ", 4);
	writeLE(file, static_cast<uint32_t>(16));							// fmt chunk size
	writeLE(file, static_cast<uint16_t>(1));							// Audio format: 3 = IEEE 32-bit float
	writeLE(file, numChannels);											// Number of channels
	writeLE(file, static_cast<uint32_t>(sampleRate));											// Sample rate
	writeLE(file, static_cast<uint32_t>(sampleRate * numChannels * 4)); // Byte rate
	writeLE(file, static_cast<uint16_t>(numChannels * 4));				// Block align
	writeLE(file, static_cast<uint16_t>(16));							// Bits per sample 32

	file.write("data", 4);
	writeLE(file, static_cast<uint32_t>(0)); // Data size (updated on close)

	return file.good();
}

/**
 * @brief Append floating-point samples to the file
 * @param samples Vector of float samples (interleaved if multi-channel)
 * @return true if write succeeded
 */
bool WavWriter::addSamples(std::span<float> samples)
{
	return addSamples(samples.data(), samples.size());
}

bool WavWriter::addSamples(const float *samples, size_t count)
{
	if (!isOpen || !samples || count == 0)
		return false;

	// Convert float to int16_t with clamping
	std::vector<int16_t> pcmBuffer(count);
	for (size_t i = 0; i < count; ++i)
	{
		float s = samples[i];
		s = std::max(-1.0f, std::min(1.0f, s));
		pcmBuffer[i] = static_cast<int16_t>(s * 32767.0f);
	}

	file.write(reinterpret_cast<const char *>(pcmBuffer.data()),
			   static_cast<std::streamsize>(count * sizeof(int16_t)));
	totalDataBytes += static_cast<uint32_t>(count * sizeof(int16_t));
	return file.good();
}

/**
 * @brief Append floating-point samples to the file (pointer interface)
 * @param samples Pointer to float buffer
 * @param count Number of samples to write
 * @return true if write succeeded
 */
/*
bool WavWriter::addSamples(const float *samples, size_t count)
{
	if (!isOpen || !samples || count == 0)
		return false;

	file.write(reinterpret_cast<const char *>(samples), count * sizeof(float));
	totalSamples += count;
	return file.good();
}
*/

/**
 * @brief Explicitly close and finalize the WAV file
 */
void WavWriter::close()
{
	if (isOpen)
	{
		file.flush();
		updateChunkSizes();
		file.close();
		isOpen = false;
	}
}

void WavWriter::updateChunkSizes()
{
	if (!file.is_open() || file.bad())
		return;

	uint32_t riffSize = 36 + totalDataBytes; // 44 header bytes - 8 + data size

	file.seekp(4, std::ios::beg);
	writeLE(file, static_cast<uint32_t>(riffSize));
	file.seekp(40, std::ios::beg);
	writeLE(file, static_cast<uint32_t>(totalDataBytes));
	printf("WAV file finalized: RIFF size = %u, data size = %u\n", riffSize, totalDataBytes);
}

// Little-endian writers for cross-platform compatibility
void WavWriter::writeLE(std::ostream &os, uint16_t value)
{
	uint8_t bytes[2] = {
		static_cast<uint8_t>(value & 0xFF),
		static_cast<uint8_t>((value >> 8) & 0xFF)};
	os.write(reinterpret_cast<const char *>(bytes), 2);
}

void WavWriter::writeLE(std::ostream &os, uint32_t value)
{
	uint8_t bytes[4] = {
		static_cast<uint8_t>(value & 0xFF),
		static_cast<uint8_t>((value >> 8) & 0xFF),
		static_cast<uint8_t>((value >> 16) & 0xFF),
		static_cast<uint8_t>((value >> 24) & 0xFF)};
	os.write(reinterpret_cast<const char *>(bytes), 4);
}