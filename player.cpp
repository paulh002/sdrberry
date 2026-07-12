#include "player.h"
#include "FastFourier.h"
#include "PeakLevelDetector.h"
#include "Spectrum.h"
#include "gui_speech.h"
#include <complex.h>
#include <liquid/liquid.h>
#include <math.h>
#include <stdio.h>

static std::shared_ptr<player> sp_echo;

#define dB2mag(x) pow(10.0, (x) / 20.0)

bool player::create_player(AudioOutput *audio_ouput, AudioInput *audio_input, const std::string &_filename)
{
	if (sp_echo != nullptr)
		return false;
	
	sp_echo = std::make_shared<player>(audio_ouput, audio_input);
	sp_echo->filename = _filename;
	sp_echo->player_thread = std::thread(&player::operator(), sp_echo);
	return true;
}

void player::destroy_player()
{
	if (sp_echo == nullptr)
		return;
	sp_echo->player_thread.join();
	sp_echo.reset();
}

player::player(AudioOutput *audio_ouput, AudioInput *audio_input)
	: Demodulator(audio_ouput, audio_input)
{
}

player::~player()
{
}

void player::operator()()
{
	SampleVector audioframes;
	std::span<Sample> audiosink, audiosamples;
	
	setBandPassFilter(3000.0f, 100.0f);
	if (!wavReader.open(filename))
	{
		std::cerr << "Failed to open. Ensure 16-bit mono PCM.\n";
		return ;
	}

	std::cout << "Sample Rate: " << wavReader.getSampleRate() << " Hz\n";
	std::cout << "Total Samples: " << wavReader.getTotalSamples() << "\n";
	while (!wavReader.isEOF())
	{
		audiosink = audioInputBuffer->read();
		if (!wavReader.readChunk(audiosamples, audio_input->getbufferFrames()))
			break;
		
		calc_af_signalstrength(audiosamples);
		audio_output->adjust_gain(audiosamples);
		for (auto &col : audiosamples)
		{
			// split the stream in blocks of samples of the size framesize
			audioframes.insert(audioframes.end(), col);
			if (audioframes.size() == audio_output->get_framesize())
			{
				if ((audio_output->queued_samples() / 2) < 4096)
				{
					SampleVector audio_stereo;

					mono_to_left_right(audioframes, audio_stereo);
					audio_output->write(audio_stereo);
					audioframes.clear();
				}
				else
					audioframes.clear();
			}
		}
	}
	wavReader.close();
	printf("exit player\n");
}