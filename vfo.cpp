#include "vfo.h"
#include "Catinterface.h"
#include "CatTcpServer.h"
#include "BandFilter.h"
#include "sdrstream.h"
#include "Spectrum.h"
#include "gui_cal.h"
#include "gui_bar.h"
#include "gui_tx.h"
#include "Gui_band.h"
#include "WebServer.h"
#include "gui_top_bar.h"

CVfo	vfo;

CVfo::CVfo()
{
	memset(&vfo_setting, 0, sizeof(struct vfo_settings_struct));
	vfo_setting.frq_step = 10;
	vfo_setting.tx = false;
	vfo_setting.rx = true;
	limit_ham_band = true;
	vfo_setting.split = false;
}

void CVfo::vfo_rxtx(bool brx, bool btx, bool split)
{
	vfo_setting.tx = btx;
	vfo_setting.rx = brx;
	vfo_setting.split = split;
	if (vfo_setting.tx && !vfo_setting.rx && split && vfo_setting.active_vfo == vfo_activevfo::One)
		bpf.SetBand(vfo_setting.band[vfo_activevfo::Two], vfo_setting.rx);
	else
		bpf.SetBand(vfo_setting.band[vfo_setting.active_vfo], vfo_setting.rx);
}

void CVfo::vfo_init(long ifrate, long pcmrate, long span, SdrDeviceVector *fSdrDevices, std::string fradio, int frx_channel,int ftx_channel)
{
	int channel;
	
	SdrDevices = fSdrDevices;
	radio = fradio;
	rx_channel = frx_channel;
	tx_channel = ftx_channel;
	vfo_setting.correction_rx = Settings_file.get_int(default_radio, "correction_rx", 0);
	vfo_setting.correction_tx = Settings_file.get_int(default_radio, "correction_tx", 0);
	printf("correction rx %d, tx %d\n", vfo_setting.correction_rx, vfo_setting.correction_tx);
	vfo_setting.notxoffset = Settings_file.get_int(default_radio, "notxoffset", 0);
	vfo_setting.maxtxoffset = Settings_file.get_int(default_radio, "maxtxoffset", 0);
	ppm = ((double)Settings_file.get_int(default_radio, "cal_ppm", 0)) / 10.0;

	vfo_setting.active_vfo = 0;
	vfo_setting.span = span;
	long freq = Settings_file.get_longlong("VFO1","freq");
	string ham = Settings_file.find_radio("band");
	if (ham != "all")
		vfo.limit_ham_band = true;
	else
		vfo.limit_ham_band = false;

	channel = rx_channel;
	if (channel < 0)
		channel = tx_channel;
	SoapySDR::RangeList r = SdrDevices->get_full_frequency_range_list(radio, channel); 
	vfo_setting.vfo_low = r.front().minimum();
	vfo_setting.vfo_high = r.front().maximum();
	auto it_band = Settings_file.meters.begin();
	auto it_high = Settings_file.f_high.begin();
	auto it_mode = Settings_file.mode.begin();
	auto it_label = Settings_file.labels.begin();
	vfo_setting.bands.clear();
	for (auto col : Settings_file.f_low)
	{
		if ((col >= vfo_setting.vfo_low) && (col < vfo_setting.vfo_high))
		{
			bands_t b;
			b.f_low = col;
			if (it_high != Settings_file.f_high.end())
				b.f_high = *it_high;
			if (it_band != Settings_file.meters.end())
				b.meters = *it_band;
			if (it_label != Settings_file.labels.end())
				b.label = *it_label;
			if (it_mode != Settings_file.mode.end())
			{
				
				if (*it_mode == "usb")
					b.f_mode = mode_usb;
				if (*it_mode == "lsb")
					b.f_mode = mode_lsb;
				if (*it_mode == "ft8")
					b.f_mode = mode_ft8;
				if (*it_mode == "fm")
					b.f_mode = mode_narrowband_fm;
				if (*it_mode == "bfm")
					b.f_mode = mode_broadband_fm;
				if (*it_mode == "am")
					b.f_mode = mode_am;
				if (*it_mode == "dsb")
					b.f_mode = mode_dsb;
				b.mode = *it_mode;
				vfo_setting.bands.push_back(b);
			}
		}
		it_band++;
		it_high++;
		it_mode++;
	}
	if (freq < vfo_setting.vfo_low)
	{
		freq = vfo_setting.vfo_low;
		if (vfo.limit_ham_band)
			check_band(1, freq);
	}
	if (freq > vfo_setting.vfo_high)
	{
		freq = vfo_setting.vfo_high;
		if (vfo.limit_ham_band)
			check_band(0, freq);
	}
	
	vfo_setting.vfo_freq[1] = Settings_file.get_longlong("VFO2", "freq");
	if (vfo_setting.vfo_freq[1] < vfo_setting.vfo_low || vfo_setting.vfo_freq[1] > vfo_setting.vfo_high)
	{
		vfo_setting.vfo_freq[1] = vfo_setting.vfo_low;
	}
	vfo_setting.mode[1] = Settings_file.convert_mode(Settings_file.get_string("VFO2", "Mode"));
	
	span_offset_frequency = 0;
	vfo_setting.span = span;
	vfo_setting.pcmrate = pcmrate;
	vfo_setting.min_offset = vfo_setting.max_offset = ifrate / 2;
	vfo_setting.vfo_freq[0] = freq; // initialize the frequency to user default
	set_span(span);
	get_band(0);
	get_band(1);
	gui_band_instance.set_gui(vfo_setting.band[vfo_setting.active_vfo]);
	rx_set_sdr_freq();
	//tx_set_sdr_freq();	
	bpf.SetBand(vfo_setting.band[0], vfo_setting.rx);
	gui_vfo_inst.set_vfo_gui(0, vfo_setting.vfo_freq[0], get_rx(), get_mode_no(0), get_band_no(0), getBandIndex(get_band_no(0)));
	gui_vfo_inst.set_vfo_gui(1, vfo_setting.vfo_freq[1], get_rx(), get_mode_no(1), get_band_no(1), getBandIndex(get_band_no(0)));
	catinterface->SetBand(get_band_in_meters());
	catinterface->SetFA(vfo_setting.vfo_freq[0]);
	catinterface->SetFB(vfo_setting.vfo_freq[1]);
	catinterface.SetMDA(get_mode_no(0));
	cattcpserver.SetMDA(get_mode_no(0));
	catinterface.SetMDB(get_mode_no(1));
	cattcpserver.SetMDB(get_mode_no(1));
	
	gcal.SetCalibrationBand(getBandIndex(vfo_setting.band[vfo.vfo_setting.active_vfo]));
	//printf("Vfo init: freq %lld, sdr %lld offset %ld maxoffset %ld\n", vfo_setting.vfo_freq[0], vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo], vfo_setting.offset[vfo_setting.active_vfo], vfo_setting.max_offset);
}

void CVfo::vfo_re_init(long ifrate, long pcmrate, long span, long bandwidth)
{	
	vfo_setting.pcmrate = pcmrate;
	vfo_setting.bandwidth = bandwidth;
	if (bandwidth == 0)
		vfo_setting.bandwidth = pcmrate;
	set_span(span);
}

void CVfo::set_span(long span)
{
	span_offset_frequency = 0;
	vfo_setting.span = span;
	
	if (ifrate <= span)
	{
		vfo_setting.min_offset = vfo_setting.max_offset = ifrate / 2;
		vfo_setting.min_offset *= -1;
		span_offset_frequency = 0;
		vfo_setting.vfo_freq_sdr[0] = vfo_setting.vfo_freq[0];
		vfo_setting.offset[0] = 0;
		vfo_setting.vfo_freq_sdr[1] = vfo_setting.vfo_freq[1];
		vfo_setting.offset[1] = 0;
	}
	else if (span >= ifrate / 2)
	{
		vfo_setting.max_offset = (ifrate / 2);
		vfo_setting.min_offset = (span - ifrate / 2);
		vfo_setting.min_offset *= -1;
		span_offset_frequency = span / 2 - (span - ifrate / 2);
		vfo_setting.vfo_freq_sdr[0] = vfo_setting.vfo_freq[0] - span_offset_frequency ;
		vfo_setting.offset[0] = vfo_setting.vfo_freq[0] - vfo_setting.vfo_freq_sdr[0];
		vfo_setting.vfo_freq_sdr[1] = vfo_setting.vfo_freq[1] - span_offset_frequency;
		vfo_setting.offset[1] = vfo_setting.vfo_freq[1] - vfo_setting.vfo_freq_sdr[1];
	}
	else
	{
		vfo_setting.max_offset = span;
		span_offset_frequency = span / 2;
		vfo_setting.min_offset = 0;
		vfo_setting.vfo_freq_sdr[0] = vfo_setting.vfo_freq[0] - span_offset_frequency;
		vfo_setting.offset[0] = vfo_setting.vfo_freq[0] - vfo_setting.vfo_freq_sdr[0];
		vfo_setting.vfo_freq_sdr[1] = vfo_setting.vfo_freq[1] - vfo_setting.max_offset;
		vfo_setting.offset[1] = vfo_setting.vfo_freq[1] - vfo_setting.vfo_freq_sdr[1];
	}
	rx_set_sdr_freq();
}

void CVfo::set_frequency_to_left(long freq, int active_vfo, bool update)
{
	span_offset_frequency = 0;
	long span = vfo_setting.span;

	vfo_setting.vfo_freq_sdr[active_vfo] = freq;
	vfo_setting.offset[active_vfo] = vfo_setting.vfo_freq[active_vfo] - vfo_setting.vfo_freq_sdr[active_vfo];
	if (vfo_setting.offset[active_vfo] > vfo_setting.max_offset)
	{
		vfo_setting.offset[active_vfo] = vfo_setting.max_offset;
		vfo_setting.vfo_freq[active_vfo] = vfo_setting.vfo_freq_sdr[active_vfo] + vfo_setting.max_offset;
	}
	if (vfo_setting.offset[active_vfo] < vfo_setting.min_offset)
	{
		vfo_setting.offset[active_vfo] = vfo_setting.min_offset;
		vfo_setting.vfo_freq[active_vfo] = vfo_setting.vfo_freq_sdr[active_vfo] + vfo_setting.min_offset;
	}
	if (update)
		rx_set_sdr_freq();
	//printf("sdr freq: %lld\n", vfo_setting.vfo_freq_sdr[active_vfo]);
}

bool CVfo::compare_span()
{
	if ((ifrate - (double)vfo_setting.span) < 0.1)
		return false;
	return true;
}

std::pair<vfo_spansetting,double> CVfo::compare_span_ex()
{
	if (ifrate <= vfo_setting.span)
		return std::pair<vfo_spansetting, double>(span_is_ifrate, 0.0);
	else if (vfo_setting.span >= ifrate / 2)
		return std::pair<vfo_spansetting, double>(span_between_ifrate, ((double)ifrate - vfo_setting.span) / ifrate);
	return std::pair<vfo_spansetting, double>(span_lower_halfrate, 0.0);
}

void CVfo::rx_set_sdr_freq()
{
	if (SdrDevices && rx_channel >= 0)
	{
		SdrDevices->SdrDevices.at(radio)->setFrequency(SOAPY_SDR_RX, rx_channel, adjustFrequencyByPpm(vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo]) + vfo_setting.correction_rx);
	}
}

void CVfo::tx_set_sdr_freq()
{
	if (SdrDevices && tx_channel >= 0)
	{
		printf("TX Freq %lld\n", vfo.get_tx_frequency());
		SdrDevices->SdrDevices.at(radio)->setFrequency(SOAPY_SDR_TX, 0, adjustFrequencyByPpm(vfo.get_tx_frequency()) + vfo_setting.correction_tx);
	}
}

long CVfo::get_vfo_offset(bool rit)
{
	//unique_lock<mutex> lock(m_vfo_mutex);
	
	long offset_freq = vfo_setting.offset[vfo_setting.active_vfo] + vfo_setting.vfo_rit[vfo_setting.active_vfo];
	
	if (!rit || offset_freq > vfo_setting.max_offset || offset_freq < vfo_setting.min_offset)
	{
			offset_freq = vfo_setting.offset[vfo_setting.active_vfo];
	}
	return offset_freq;
//vfo_setting.offset[vfo_setting.active_vfo];
}

long CVfo::get_vfo_offset_tx()
{
	//unique_lock<mutex> lock(m_vfo_mutex);
	if (((abs(ifrate - ifrate_tx) > 0.1) || (abs(ifrate_tx - (double)vfo_setting.pcmrate) < 0.1) || vfo_setting.notxoffset) && vfo_setting.tx)
		return 0L;
	else
		return vfo_setting.offset[vfo_setting.active_vfo];
}

long CVfo::adjustFrequencyByPpm(long baseFreqHz)
{
	double adjusted = static_cast<double>(baseFreqHz) * (1.0 + ppm / 1'000'000.0);
	return static_cast<long>(std::round(adjusted));
}

void CVfo::set_ppm(int _ppm)
{
	ppm = ((double)_ppm) / 10.0;
}

/*
 *
 * The sdr radio wil start with the frequency selected in the center of the fft
 * When the user changes the freqency the offset to the center is calculated and the mixer
 * in the modulator will up or down mix the signal until 1/2 the sample frequency.
 * If the frequency is changed higher or lower than half the sampling frequency the sdr oscilator is
 * changed 1/4 of the sample frequency higer or lower
 * 
 * This is done to minimize the changing of the local oscilator of the sdr receiver (like Pluto)
 * ToDo: for SDR receivers without local oscilator this behaviour is not necessary, but sill used
 * to be checked if this is ok for receivers like radioberry. In case of transmit if the ifrate is the same
 * as pcmrate the 1/4 ifrate shift is not used. Keep in mind that the radioberry only has a transmit ifrate of 48 Khz. 
 * So it need to tuned differently than the receiver.
 **/

int CVfo::set_vfo(long freq, vfo_activevfo ActiveVfo)
{
	//unique_lock<mutex> lock_set_vfo(m_vfo_mutex);
	int retval{0};
	bool changeBandActiveVfo{false};

	if (pausevfo)
		return 0;
	
	if (ActiveVfo != (vfo_activevfo)vfo_setting.active_vfo && ActiveVfo != vfo_activevfo::None)
	{
		if (vfo_setting.band[vfo_setting.active_vfo] != ActiveVfo)
			changeBandActiveVfo = true;
		vfo_setting.active_vfo = (int)ActiveVfo;
	}

	if (freq == 0L)
	{
		freq = vfo_setting.vfo_freq[vfo_setting.active_vfo];
		if (vfo_setting.rx)
			rx_set_sdr_freq();
		if (vfo_setting.tx)
			tx_set_sdr_freq();
	}
	if (freq < vfo_setting.vfo_low || freq > vfo_setting.vfo_high)
		return -1;
	//vfo_setting.band[1] = band;
	if (((abs(ifrate - ifrate_tx) > 0.1) || (abs(ifrate_tx - (double)vfo_setting.pcmrate) < 0.1) || vfo_setting.notxoffset) && vfo_setting.tx)
	{  // incase of different ifrates for tx don't use offset or incase tx samplerate is equal to pcmrate
		vfo_setting.vfo_freq[vfo_setting.active_vfo] = freq;
		tx_set_sdr_freq();
		printf("tx no offset freq = %lld\n", vfo_setting.vfo_freq[vfo_setting.active_vfo]);
	}
	else
	{
		//printf("freq %lld, sdr %lld offset %lld diff %ld\n", freq, vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo], freq - vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo], freq - vfo_setting.vfo_freq[vfo_setting.active_vfo]);
		if (abs(freq - vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo]) > vfo_setting.max_offset && (freq - vfo_setting.vfo_freq[vfo_setting.active_vfo]) >0 )
		{
			vfo_setting.vfo_freq[vfo_setting.active_vfo] = freq;												  // initialize the frequency to user default
			vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo] = freq - span_offset_frequency;					  // position sdr frequency 1/4 of samplerate lower -> user frequency will be in center of fft display
			vfo_setting.offset[vfo_setting.active_vfo] = freq - vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo]; //
			if (vfo_setting.rx)
				rx_set_sdr_freq();
			if (vfo_setting.tx)
				tx_set_sdr_freq();
			tune_flag = true;
			printf("Tune pos: freq %lld, sdr %lld offset %ld maxoffset %ld\n", freq, vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo], vfo_setting.offset[vfo_setting.active_vfo], vfo_setting.max_offset);
		}
		else if (freq - vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo] < vfo_setting.min_offset && (freq - vfo_setting.vfo_freq[vfo_setting.active_vfo]) < 0)
		{
			vfo_setting.vfo_freq[vfo_setting.active_vfo] = freq;												  // initialize the frequency to user default
			vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo] = freq - span_offset_frequency;					  // position sdr frequency 1/4 of samplerate lower -> user frequency will be in center of fft display
			vfo_setting.offset[vfo_setting.active_vfo] = freq - vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo]; //
			if (vfo_setting.rx)
				rx_set_sdr_freq();
			if (vfo_setting.tx)
				tx_set_sdr_freq();
			tune_flag = true;
			//printf("Tune neg: freq %lld, sdr %lld offset %ld maxoffset %ld \n", freq, vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo], vfo_setting.offset[vfo_setting.active_vfo], vfo_setting.max_offset);
		}
		else
		{
			vfo_setting.vfo_freq[vfo_setting.active_vfo] = freq;
			vfo_setting.offset[vfo_setting.active_vfo] = freq - vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo];
			tune_flag = true;
			//printf("set offset %ld\n", vfo_setting.offset[vfo_setting.active_vfo]);
		}
	}
	//printf("freq %lld, sdr %lld offset %ld maxoffset %ld step %d\n", freq, vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo], vfo_setting.offset[vfo_setting.active_vfo], vfo_setting.max_offset, vfo_setting.frq_step);
	SpectrumGraph.set_pos(vfo_setting.offset[vfo.vfo_setting.active_vfo]);
	if (get_band(vfo_setting.active_vfo) || changeBandActiveVfo)
	{ // Band Change?
		catinterface->SetBand(get_band_in_meters());
		bpf.SetBand(vfo_setting.band[vfo.vfo_setting.active_vfo], vfo_setting.rx);
		gcal.SetCalibrationBand(getBandIndex(vfo_setting.band[vfo.vfo_setting.active_vfo]));
		gbar.set_gain_slider_band_from_config();
		printf("vfo band change\n");
		retval = 1;
	}
	gui_vfo_inst.set_vfo_gui(vfo_setting.active_vfo, freq, get_rx(), get_mode_no(vfo_setting.active_vfo), get_band_no(vfo_setting.active_vfo), getBandIndex(get_band_no(vfo_setting.active_vfo)));
	gui_band_instance.set_gui(vfo_setting.band[0]);
	if (vfo.vfo_setting.active_vfo == vfo_activevfo::One)
		catinterface->SetFA(vfo_setting.vfo_freq[0]);
	if (vfo.vfo_setting.active_vfo == vfo_activevfo::Two)
		catinterface->SetFB(vfo_setting.vfo_freq[1]);
	catinterface.InitVfo(vfo_setting.vfo_freq[0], vfo_setting.vfo_freq[1]);
	updateweb();
	return retval;
}

long CVfo::get_vfo_absolute_offset()
{
	return vfo_setting.offset[vfo.vfo_setting.active_vfo] + abs(vfo_setting.min_offset);
}

void CVfo::sync_rx_vfo()
{
	vfo_setting.vfo_freq[1] = vfo_setting.vfo_freq[0] ;
	gui_vfo_inst.set_vfo_gui(1, vfo_setting.vfo_freq[1], get_rx(), get_mode_no(1), get_band_no(1), getBandIndex(get_band_no(1)));
}
	
void CVfo::step_vfo(long icount)
{
	long freq;
	if (pausevfo)
		return;
	if (m_delay)
	{
		m_delay_counter += abs(icount);
		if (m_delay_counter < m_delay)
			return;
		m_delay_counter = 0;
	}
	//vfo_setting.vfo_freq[vfo_setting.active_vfo] += (vfo_setting.frq_step * icount);
	freq = vfo_setting.vfo_freq[vfo_setting.active_vfo] + (vfo_setting.frq_step * icount);

	if (freq < vfo_setting.vfo_low || freq > vfo_setting.vfo_high)
		return;	
	if (SdrDevices)
	{
		if (vfo.limit_ham_band)
			check_band(icount, freq);
		int i = set_vfo(freq);
		if (vfo.limit_ham_band && i == 1)
		{	// check if we need to swtich mode
			int index = getBandIndex(vfo_setting.band[vfo_setting.active_vfo]);
			if (index >= 0 && index < vfo_setting.bands.size() && vfo_setting.mode[vfo_setting.active_vfo] != vfo_setting.bands[index].f_mode)
			{
				vfo_setting.mode[vfo_setting.active_vfo] = vfo_setting.bands[index].f_mode;
				select_mode(vfo_setting.mode[vfo_setting.active_vfo], false);
				gbar.set_mode(mode);
			}
		}
	}
}

long CVfo::get_active_vfo_freq()
{
	return vfo_setting.vfo_freq[vfo_setting.active_vfo];
}


std::string CVfo::get_vfo_str()
{
	char	str[30];
	long	freq;
	
	freq = vfo_setting.vfo_freq[vfo_setting.active_vfo];
	if (freq > 10000000LU)
	{
		sprintf(str, "%3ld.%03ld,%02ld", (long)(freq / 1000000), (long)((freq / 1000) % 1000), (long)((freq / 10) % 100));
	}
	else
	{
		sprintf(str, "%3ld.%03ld,%02ld", (long)(freq / 1000000), (long)((freq / 1000) % 1000), (long)((freq / 10) % 100));
	}
	std::string s(str);
	return s;
}

long CVfo::get_tx_frequency()
{
	int active_vfo = vfo_setting.active_vfo;

	if (vfo_setting.split)
		active_vfo = Two;
	// incase of different ifrates for tx don't use offset
	if (fabs(ifrate - ifrate_tx) > 0.1 || vfo_setting.notxoffset)
		return vfo_setting.vfo_freq[active_vfo];
	else
		return vfo_setting.vfo_freq_sdr[active_vfo];
}

void CVfo::set_tuner_offset(double offset)
{
	vfo_setting.tuner_offset = offset;
}

void CVfo::set_active_vfo(int active_vfo)
{
	set_vfo(0, (vfo_activevfo)min(active_vfo, 1));
}

void CVfo::set_vfo_range(long low, long high)
{
	vfo_setting.vfo_low = low;
	vfo_setting.vfo_high = high;
}

void CVfo::set_band_freq(long freq)
{
	int band = 0;

	auto it_high = Settings_file.f_high.begin();
	auto it_band = Settings_file.meters.begin();
	for (auto &col : Settings_file.f_low)
	{
		if ((freq >= col) && (freq <= *it_high))
		{
			band = *it_band;
		}
		it_high++;
		it_band++;
	}
	if (band != 0)
		vfo.set_band(band, freq);
}

void CVfo::set_band(int band, long freq)
{
	int index = getBandIndex(band);
	if (index >= 0 && index < vfo_setting.bands.size())
	{
		if (vfo_setting.mode[vfo_setting.active_vfo] != vfo_setting.bands[index].f_mode)
		{
			vfo_setting.mode[vfo_setting.active_vfo] = vfo_setting.bands[index].f_mode;
			set_vfo(freq);
			select_mode(vfo_setting.mode[vfo_setting.active_vfo], false);
			gbar.set_mode(mode);
			return;
		}
	}
	set_vfo(freq);
}

// return 1 if band has changed
// update vfo_setting.band[active_vfo] to current band
int CVfo::get_band(int active_vfo)
{
	long freq = vfo_setting.vfo_freq[active_vfo];
	int	band = vfo_setting.band[active_vfo];
	
	auto it_high = Settings_file.f_high.begin();
	auto it_band = Settings_file.meters.begin();
	for (auto& col : Settings_file.f_low)
	{
		if ((freq >= col) && (freq <= *it_high))
			{
				vfo_setting.band[active_vfo] = *it_band;
			}
		it_high++;
		it_band++;
	}
	
	if (band == vfo_setting.band[active_vfo])
		return 0;
	else
		return 1;
}

void CVfo::check_band(int dir, long &freq)
{
		// this function let the active vfo jump to next or previous band	
	int i = 0;
	for (auto& col : vfo_setting.bands)
	{
		if (vfo_setting.band[vfo_setting.active_vfo] == col.meters)
		{
			if ((freq > vfo_setting.bands[i].f_high) && (dir > 0))
			{
				i++;
				if (i >= vfo_setting.bands.size())
				{
					i = 0;
					freq = vfo_setting.bands[i].f_low;
					break;
				}
				else
				{
					freq = vfo_setting.bands[i].f_low;
					break;					
				}
			}
			
			if ((freq < vfo_setting.bands[i].f_low) && (dir < 0))
			{
				if (freq < vfo_setting.bands[i].f_low) 
				{
					i--;
					if (i < 0)
					{
						i = vfo_setting.bands.size() - 1;
						freq = vfo_setting.bands[i].f_high;
						break;
					}
					else
					{
						freq = vfo_setting.bands[i].f_high;
						break;					
					}	
				}
			}
			break;
		}
		i++;
	}
}

void CVfo::return_bands(vector<int> &bands)
{
	bands.clear();
	for (auto& col : vfo_setting.bands)
	{
		bands.push_back(col.meters);
	}
}

int CVfo::getBandIndex(int band)
{
	int i = 0;
	for (auto& col : vfo_setting.bands)
	{
		if (band == col.meters)
			return i;
		i++;
	}
	return 0;
}

void CVfo::setVfoFrequency(int direction)
{
	long freq;

	if (direction > 0)
		freq = vfo_setting.span + vfo_setting.vfo_freq[vfo_setting.active_vfo];
	else
		freq = vfo_setting.vfo_freq[vfo_setting.active_vfo] - vfo_setting.span;

	if (freq == vfo_setting.vfo_freq[vfo_setting.active_vfo])
		return;

	if (freq < vfo_setting.vfo_low || freq > vfo_setting.vfo_high)
		return;
	if (SdrDevices)
	{
		if (vfo.limit_ham_band)
			check_band(direction, freq);
		int i = set_vfo(freq);
		if (vfo.limit_ham_band && i == 1)
		{ // check if we need to swtich mode
			int index = getBandIndex(vfo_setting.band[vfo_setting.active_vfo]);
			if (index >= 0 && index < vfo_setting.bands.size() && vfo_setting.mode[vfo_setting.active_vfo] != vfo_setting.bands[index].f_mode)
			{
				vfo_setting.mode[vfo_setting.active_vfo] = vfo_setting.bands[index].f_mode;
				select_mode(vfo_setting.mode[vfo_setting.active_vfo], false);
				gbar.set_mode(mode);
			}
		}
	}
}

int CVfo::get_current_mode()
{
	int index = getBandIndex(vfo_setting.band[vfo_setting.active_vfo]);
	return vfo_setting.mode[vfo_setting.active_vfo] = vfo_setting.bands[index].f_mode;
}

long CVfo::get_sdr_span_frequency()
{
	return vfo_setting.vfo_freq_sdr[vfo_setting.active_vfo] + vfo_setting.min_offset ; 
}

int CVfo::get_band_no(int vfo)
{
	if (vfo < 2 && vfo >= 0)
		return vfo_setting.band[vfo];
	else
		return 0;
}

int CVfo::get_mode_no(int vfo)
{
	if (vfo < 2 && vfo >= 0)
		return vfo_setting.mode[vfo];
	else
		return 0;
}

void CVfo::set_mode(int vfo, int mode)
{
	if (vfo < 2 && vfo >= 0)
		vfo_setting.mode[vfo] = mode;
}

void CVfo::set_step(int step, int delay)
{
	vfo_setting.frq_step = step;
	m_delay = delay;
}

void CVfo::setRit(int rit, int active_vfo)
{
	//unique_lock<mutex> lock(m_vfo_mutex);
	
	vfo_setting.vfo_rit[active_vfo] = rit;
	tune_flag = true;
}

std::string CVfo::getMode(int active_vfo)
{
	char str[80];

	strcpy(str, "");
	switch (vfo_setting.mode[active_vfo])
	{
	case mode_broadband_fm:
		strcpy(str, "FM");
		break;
	case mode_lsb:
		strcpy(str, "LSB");
		break;
	case mode_ft4:
	case mode_ft8:
	case mode_usb:
		strcpy(str, "USB");
		break;
	case mode_dsb:
		strcpy(str, "DSB");
	case mode_am:
		strcpy(str, "AM");
		break;
	case mode_cw:
		strcpy(str, "CW");
		break;
	}
	std::string result(str);
	return result;
}

std::string CVfo::get_band_in_text()
{
	char str[80];
	
	sprintf(str, "%d ", get_band_in_meters());

	std::string result(str);
	return result + vfo_setting.bands.at(getCurrentBandIndex()).label;
}

std::string CVfo::get_mode_in_text()
{
	return vfo_setting.bands.at(getCurrentBandIndex()).mode;
}

std::vector<int16_t> CVfo::Legend()
{
	std::vector<int16_t> spectrumLedgend;
	std::pair<vfo_spansetting, double> span_ex = compare_span_ex();
	int span = vfo_setting.span; 
	int ii;
	double offset{0}, f{0.0};

	switch (span_ex.first)
	{
	//case span_is_ifrate:
	//	f = (double)vfo.get_sdr_frequency() - (double)(span / 2.0);
	//	bins = nfft_samples;
	//	ii = span / (nfft_samples - 1);
	//	break;
	case span_is_ifrate:
	case span_between_ifrate:
		f = (double)vfo.get_sdr_frequency() - (double)vfo.get_minoffset();
		ii = span / (vert_lines - 1);
		break;
	case span_lower_halfrate:
		offset = vfo.get_vfo_offset() / span;
		f = (double)vfo.get_sdr_frequency() + offset * (double)span;
		ii = span / (vert_lines - 1);
		break;
	}

	for (int i = 0; i < vert_lines; i++)
	{
		int16_t l = (int16_t)round(f / 1000.0);
		spectrumLedgend.push_back(l);
		f = f + ii;
	}
	return spectrumLedgend;
}

void CVfo::updateweb()
{
	if (webserver.isEnabled())
	{
		json message, data;

		std::string freq = get_vfo_str();
		std::string mode = getMode(vfo.get_active_vfo());
		std::string band = get_band_in_text();
		std::string call = Settings_file.get_string("wsjtx", "call");

		message.clear();
		data.clear();
		data.emplace("frequency", freq);
		data.emplace("mode", mode);
		data.emplace("band", band);
		data.emplace("call", call);
		data.emplace("label", GuiTopBar.getLabel());
		message.emplace("type", "vfo");
		message.emplace("data",data);
		webserver.SendMessage(message);
		
		message.clear();
		message.emplace("type","ledgend");
		message.emplace("data",Legend());
		webserver.SendMessage(message);
	}
}

bool CVfo::checkVfoBandRange(long freq)
{
	bool range = false;
	for (auto col : vfo_setting.bands)
	{
		if (freq >= col.f_low && freq <= col.f_high)
		{
			range = true;
			break;
		}
	}
	return range;
}