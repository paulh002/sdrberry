#include "FreeDVTab.h"
#include "AudioOutput.h"

FreeDVTab freeDVTab;

void FreeDVTab::init(lv_obj_t *scr, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h)
{
	float resampleRate = 4000.0 / (float)audio_output->get_samplerate();
	waterfall = std::make_unique<Waterfall>(scr, x, y, w / 2, h, resampleRate);
}

void FreeDVTab::DrawWaterfall()
{
	waterfall->Draw();
}

void FreeDVTab::Process(const IQSampleVector &input)
{
	waterfall->Process(input);
}

void FreeDVTab::SetMode(int mode) 
{
	waterfall->SetMode(mode);
}