#pragma once
#include "RtAudio.h"

#define dB2mag(x) pow(10.0, (x) / 20.0)

template <typename Class, int (Class::*AudioCallback)(void *, void *, unsigned int, double, RtAudioStreamStatus)>
struct AudioCallbackHandler
{
	static int staticCallbackHandler(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData)
	{
		return (static_cast<Class *>(userData)->*AudioCallback)(outputBuffer, inputBuffer, nBufferFrames, streamTime, status);
	}
};