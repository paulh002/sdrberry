#pragma once
#include "RtAudio.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include "Audiodefs.h"
#include "DataBuffer.h"

class AudioInput : public RtAudio
{
public:
	bool init(std::string device, int pcmrate);
	bool open(DataBuffer<Sample>	*AudioBuffer);
	void adjust_gain(SampleVector& samples);
	bool read(SampleVector& samples);
	void close();
	~AudioInput();
	double	get_volume() {return m_volume;}
	void	set_volume(double vol)	{m_volume = vol; }
	
	operator bool() const
	{
		return (!m_zombie) && m_error.empty();
	}
	
private:
	RtAudio::StreamParameters	parameters;
	unsigned int				sampleRate;
	unsigned int				bufferFrames;
	double						m_volume {0.5};
	DataBuffer<Sample>			*databuffer;
	string						m_error;
	bool						m_zombie {false}
	;
};

extern  AudioInput  *audio_input;