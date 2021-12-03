#pragma once
#include <string>
#include "RtAudio.h"
#include "Audiodefs.h"
#include "DataBuffer.h"

class AudioOutput :
    public RtAudio
{
public:
	bool	init(std::string device, int pcmrate);
	bool	open(DataBuffer<Sample>	*AudioBuffer);
	bool	write(SampleVector& samples);
	void	adjust_gain(SampleVector& samples);
	void	close();
	void	stop();
	~AudioOutput();
	double	get_volume() {return m_volume;}
	void	set_volume(int vol)	{m_volume = exp(((double)vol * 6.908)/100.0) / 1000.0; } // log volume
	unsigned int get_framesize() {return bufferFrames;}
	
	operator bool() const
	{
		return (!m_zombie) && m_error.empty();
	}

protected:
	void samplesToInt16(const SampleVector& samples,
		std::vector<std::uint8_t>& bytes);

private:
	RtAudio::StreamParameters	parameters;
	DataBuffer<Sample>			*databuffer;
	unsigned int				m_sampleRate;
	unsigned int				bufferFrames;  // 256 sample frames
	double						m_volume {0.5};
	string						m_error;
	bool						m_zombie {false};
};

extern  AudioOutput *audio_output;
double get_audio_sample_rate();