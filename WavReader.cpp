#pragma once
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>
#include "WavReader.h"

bool WavReader::open(const std::string &filename)
{
	close();
	file_.open(filename, std::ios::binary);
	if (!file_.is_open())
		return false;

	char id[4];
	uint32_t chunkSize;
	file_.read(id, 4);
	if (std::strncmp(id, "RIFF", 4) != 0)
		return false;
	readLE(chunkSize); // Skip RIFF size

	file_.read(id, 4);
	if (std::strncmp(id, "WAVE", 4) != 0)
		return false;

	bool fmtFound = false;
	while (file_.good())
	{
		file_.read(id, 4);
		if (file_.gcount() < 4)
			break;
		readLE(chunkSize);

		if (std::strncmp(id, "fmt ", 4) == 0)
		{
			if (chunkSize < 16)
				return false;
			uint16_t audioFormat, channels, blockAlign, bitsPerSample;
			uint32_t byteRate;
			readLE(audioFormat);
			readLE(channels);
			readLE(sampleRate_);
			readLE(byteRate);
			readLE(blockAlign);
			readLE(bitsPerSample);

			if (audioFormat != 1 || channels != 1 || bitsPerSample != 16)
			{
				return false; // Reject non-16-bit mono PCM
			}
			fmtFound = true;
			if (chunkSize > 16)
				file_.seekg(chunkSize - 16, std::ios::cur);
		}
		else if (std::strncmp(id, "data", 4) == 0)
		{
			if (!fmtFound)
				return false;
			totalSamples_ = chunkSize / sizeof(int16_t);
			remainingSamples_ = totalSamples_;
			isEOF_ = (remainingSamples_ == 0);
			isOpen_ = true;
			return true; // file_ is now positioned exactly at start of audio data
		}
		else
		{
			file_.seekg(chunkSize, std::ios::cur);
		}

		// WAV spec: all chunks are padded to even byte boundaries
		if (chunkSize % 2 != 0)
		{
			file_.seekg(1, std::ios::cur);
		}
	}
	return false; // EOF reached without valid data chunk
}

bool WavReader::readChunk(std::vector<float> &outBuffer, size_t maxSamples)
{
	if (!isOpen_ || isEOF_ || maxSamples == 0)
	{
		isEOF_ = true;
		outBuffer.clear();
		return false;
	}

	size_t toRead = std::min(maxSamples, remainingSamples_);
	outBuffer.resize(toRead);

	// Reuse internal buffer to avoid reallocation overhead
	if (pcmBuffer_.size() < toRead)
	{
		pcmBuffer_.resize(toRead);
	}

	file_.read(reinterpret_cast<char *>(pcmBuffer_.data()), toRead * sizeof(int16_t));
	if (!file_.good() || static_cast<size_t>(file_.gcount()) != toRead * sizeof(int16_t))
	{
		close();
		return false;
	}

	// Convert int16_t -> float [-1.0, 1.0)
	for (size_t i = 0; i < toRead; ++i)
	{
		outBuffer[i] = pcmBuffer_[i] / 32768.0f;
	}

	remainingSamples_ -= toRead;
	if (remainingSamples_ == 0)
		isEOF_ = true;
	return true;
}

void WavReader::close()
{
	if (file_.is_open())
	{
		file_.close();
	}
	isOpen_ = false;
	isEOF_ = false;
	remainingSamples_ = 0;
	totalSamples_ = 0;
	sampleRate_ = 0;
}

template <typename T>
bool WavReader::readLE(T &val)
{
	uint8_t bytes[sizeof(T)];
	file_.read(reinterpret_cast<char *>(bytes), sizeof(T));
	if (!file_.good())
		return false;
	val = 0;
	for (size_t i = 0; i < sizeof(T); ++i)
	{
		val |= static_cast<T>(bytes[i]) << (i * 8);
	}
	return true;
}