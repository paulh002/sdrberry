#include "sdrberry.h"
#include "Mouse.h"
#include "HidDev.h"
#include "Catinterface.h"
#include "BandFilter.h"
#include "Demodulator.h"
#include "FMDemodulator.h"
#include "FMModulator.h"
#include "AMModulator.h"
#include "FT8Demodulator.h"
#include "gui_speech.h"
#include "EchoAudio.h"
//#include "HidThread.h"

#define BACKWARD_HAS_BFD 1
#define BACKWARD_HAS_DW 1
#include "backward.hpp"

namespace backward
{
	backward::SignalHandling sh;

} // namespace backward


DataBuffer<IQSample>	source_buffer_rx;
DataBuffer<IQSample>	source_buffer_tx;

LV_FONT_DECLARE(FreeSansOblique42);
LV_FONT_DECLARE(FreeSansOblique32);

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

// display buffer size - not sure if this size is really needed
#define DISP_BUF_SIZE 384000		// 800x480
//#define DISP_BUF_SIZE (128 * 1024)

const int screenWidth = 800;
const int screenHeight = 480;
const int bottomHeight = 40;
const int topHeight = 35;
const int tunerHeight = 100;
const int barHeight = 110; //90;
const int MorseHeight = 30;
const int nobuttons = 8;
const int bottombutton_width = (screenWidth / nobuttons) - 2;
const int bottombutton_width1 = (screenWidth / nobuttons);

lv_obj_t* scr;
lv_obj_t* bg_middle;
lv_obj_t* vfo1_button;
lv_obj_t* vfo2_button;
lv_obj_t *tabview_mid;
lv_obj_t *bar_view;
lv_obj_t *tab_buttons;
lv_indev_t *encoder_indev_t{nullptr};
lv_group_t *button_group{nullptr};

using namespace std;
std::mutex gui_mutex;

int					mode = mode_broadband_fm;
double				ifrate  = 0.53e6;  //1.0e6;//
double				ifrate_tx;
double				freq = 89800000;
//double	tuner_freq = freq + 0.25 * ifrate;
//double	tuner_offset = freq - tuner_freq;

mutex			fm_finish;
Catinterface	catinterface;
Mouse Mouse_dev;
HidDev HidDev_dev, HidDev_dev1;
BandFilter		bpf;

MidiControle	*midicontrole = nullptr;
auto			startTime = std::chrono::high_resolution_clock::now();

SdrDeviceVector		SdrDevices;
std::string			default_radio;
int					default_rx_channel = 0;
int					default_tx_channel = 0;


void encoder_read(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
	//data->enc_diff = HidDev_dev.encoder_rotate();
	//data->state = HidDev_dev.encoder_key_press();
}

std::map<string, lv_obj_t *> tab;

static void tabview_event_cb(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	int i = lv_tabview_get_tab_act(tabview_mid);
		switch (i)
		{
		case 0:
			lv_indev_set_group(encoder_indev_t, button_group);
			break;
		case 1:
			gui_band_instance.set_group();
			break;
		case 3:
			gagc.set_group();
			break;
		case 4:
			Gui_tx.set_group();
			break;
		case 5:
			gsetup.set_group();
			break;
		}
}

int main(int argc, char *argv[])
{
	const int defaultAudioSampleRate{48000};

	Settings_file.read_settings(std::string("sdrberry_settings.cfg"));
	Mouse_dev.init_mouse(Settings_file.find_input("mouse"));
	HidDev_dev.init("CONTOUR DESIGN SHUTTLEXPRESS");
	HidDev_dev1.init("GN Audio A/S Jabra Evolve2 30 Consumer Control");
	
	catinterface.begin();
	std::thread thread_catinterface(std::ref(catinterface));
	thread_catinterface.detach();

	if (AudioOutput::createAudioDevice(defaultAudioSampleRate))
		AudioInput::createAudioInputDevice(audio_output->get_samplerate(), audio_output->get_device());
	
	bpf.initFilter();
	
	std::string smode = Settings_file.find_vfo1("Mode");
	mode = Settings_file.convert_mode(smode);

	default_radio = Settings_file.find_sdr("default");
	ifrate = Settings_file.get_int(default_radio, "samplerate") * 1000;
	ifrate_tx = Settings_file.get_int(default_radio, "samplerate_tx") * 1000;
	if (ifrate_tx == 0)
		ifrate_tx = ifrate;
	printf("samperate rx %f samplerate tx %f \n", ifrate, ifrate_tx);
	
	/*LittlevGL init*/
	lv_init();

	/*Linux frame buffer device init*/
	fbdev_init();

	// Touch pointer device init
	evdev_init();
	
	/*A small buffer for LittlevGL to draw the screen's content*/
	static lv_color_t buf[DISP_BUF_SIZE];
	//static lv_color_t buf1[DISP_BUF_SIZE];

	/*Initialize a descriptor for the buffer*/
	static lv_disp_draw_buf_t disp_buf;
	lv_disp_draw_buf_init(&disp_buf, buf, NULL, DISP_BUF_SIZE);

	/*Initialize and register a display driver*/
	static lv_disp_drv_t disp_drv;
	lv_disp_drv_init(&disp_drv);
	disp_drv.draw_buf   = &disp_buf;
	disp_drv.flush_cb   = fbdev_flush;
	disp_drv.hor_res    = screenWidth;
	disp_drv.ver_res    = screenHeight;
	lv_disp_drv_register(&disp_drv);
	
	// Initialize and register a pointer device driver
	lv_indev_drv_t indev_drv;
	lv_indev_drv_init(&indev_drv);
	indev_drv.type = LV_INDEV_TYPE_POINTER;
	indev_drv.read_cb = evdev_read;       // defined in lv_drivers/indev/evdev.h
	lv_indev_drv_register(&indev_drv);

	lv_indev_drv_t indev_drv_enc;
	lv_indev_drv_init(&indev_drv_enc);
	indev_drv_enc.type = LV_INDEV_TYPE_ENCODER;
	indev_drv_enc.read_cb = encoder_read;
	encoder_indev_t = lv_indev_drv_register(&indev_drv_enc);
	button_group = lv_group_create();
	lv_indev_set_group(encoder_indev_t, button_group);
	
	lv_theme_t* th = lv_theme_default_init(NULL, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_CYAN), LV_THEME_DEFAULT_DARK, &lv_font_montserrat_14);
	lv_disp_set_theme(NULL, th);
	scr = lv_scr_act();
	
	setup_top_bar(scr);
	gui_vfo_inst.gui_vfo_init(scr);
	
	static lv_style_t background_style;
		
	lv_style_init(&background_style);
	lv_style_set_radius(&background_style, 0);
	lv_style_set_bg_color(&background_style, lv_palette_main(LV_PALETTE_RED));
	
	lv_obj_t * obj1;
	bar_view = lv_obj_create(lv_scr_act());
	lv_obj_set_style_radius(bar_view, 0,0);
	lv_obj_set_pos(bar_view, 0, topHeight + tunerHeight );
	lv_obj_set_size(bar_view, LV_HOR_RES - 3, barHeight);
	
	tabview_mid = lv_tabview_create(lv_scr_act(), LV_DIR_BOTTOM, 40);
	lv_obj_add_event_cb(tabview_mid, tabview_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

	lv_obj_set_pos(tabview_mid, 0, topHeight + tunerHeight + barHeight);
	lv_obj_set_size(tabview_mid, LV_HOR_RES - 3, screenHeight - topHeight - tunerHeight - barHeight);

	tab["spectrum"] = (lv_tabview_add_tab(tabview_mid, "Spectrum"));
	tab["band"] = (lv_tabview_add_tab(tabview_mid, "Band"));
	tab["rx"] = (lv_tabview_add_tab(tabview_mid, "RX"));
	//tab["keyboard"] = (lv_tabview_add_tab(tabview_mid, LV_SYMBOL_KEYBOARD));
	tab["agc"] = (lv_tabview_add_tab(tabview_mid, "Agc"));
	tab["speech"] = (lv_tabview_add_tab(tabview_mid, "Speech"));
	tab["tx"] = (lv_tabview_add_tab(tabview_mid, "TX"));
	tab["ft8"] = (lv_tabview_add_tab(tabview_mid, "FT8"));
	tab["settings"] = (lv_tabview_add_tab(tabview_mid, LV_SYMBOL_SETTINGS));
	
	lv_obj_clear_flag(lv_tabview_get_content(tabview_mid), LV_OBJ_FLAG_SCROLL_CHAIN | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_ONE);
	tab_buttons = lv_tabview_get_tab_btns(tabview_mid);
	Wf.init(tab["spectrum"], 0, 0, LV_HOR_RES - 3, screenHeight - topHeight - tunerHeight, ifrate);
	gft8.init(tab["ft8"], 0, 0, LV_HOR_RES - 3, screenHeight - topHeight - tunerHeight);
	gagc.init(tab["agc"], LV_HOR_RES - 3);
	gspeech.init(tab["speech"], LV_HOR_RES - 3);
	Gui_tx.gui_tx_init(tab["tx"], LV_HOR_RES - 3);
	gsetup.init(tab["settings"], LV_HOR_RES - 3, *audio_output);
	guirx.init(tab["rx"], LV_HOR_RES - 3);
	lv_btnmatrix_set_btn_ctrl(tab_buttons, 4, LV_BTNMATRIX_CTRL_HIDDEN);

	//keyb.init_keyboard(tab["keyboard"], LV_HOR_RES/2 - 3, screenHeight - topHeight - tunerHeight);
	
	std::cout << "default sdr: " << Settings_file.find_sdr("default").c_str() << std::endl;
	default_rx_channel = 0;
	default_tx_channel = 0;
		
	SoapySDR::ModuleManager mm(false);
	SoapySDR::loadModules();
	freq = Settings_file.find_vfo1_freq("freq");

	default_radio = Settings_file.find_sdr("default");
	for (auto & con : Settings_file.receivers)
	{
		std::string probe = Settings_file.find_probe((char *)con.c_str());
		SdrDevices.AddDevice(con, probe);
	}
	
	if (SdrDevices.MakeDevice(default_radio))
	{
		gbar.init(bar_view, button_group,  mode, LV_HOR_RES - 3, barHeight);
		SoapySDR::Range r = SdrDevices.get_full_frequency_range(default_radio, default_rx_channel);
		std::string start_freq = std::to_string(r.minimum() / 1.0e6);
		std::string stop_freq = std::to_string(r.maximum() / 1.0e6);
		std::string s = std::string(default_radio.c_str()) + " " + start_freq + " Mhz - " + stop_freq + " Mhz";
		lv_label_set_text(label_status, s.c_str()); 
		if (SdrDevices.get_tx_channels(default_radio) < 1) // for now assume only 1 tx channel
			default_tx_channel = -1;
		else
			default_tx_channel = 0;
		vfo.set_vfo_range(r.minimum(),r.maximum());
		vfo.vfo_init((long)ifrate, defaultAudioSampleRate, gsetup.get_span() ,&SdrDevices, default_radio, default_rx_channel, default_tx_channel);
		try
		{
			if (SdrDevices.SdrDevices[default_radio]->get_txchannels() > 0)
			{
				lv_btnmatrix_clear_btn_ctrl(tab_buttons, 4, LV_BTNMATRIX_CTRL_HIDDEN);
				Gui_tx.set_sample_rate((int)ifrate_tx);
				Gui_tx.set_drv_range();
				for (auto& col : SdrDevices.SdrDevices.at(default_radio)->get_tx_sample_rates(default_tx_channel))
				{
					int v = (int)col;
					Gui_tx.add_sample_rate(v);
				}
			}
		}
		catch (const std::exception& e)
		{
			std::cout << e.what();
		}

		gsetup.set_radio(default_radio);		
		try
		{	
			for (auto& col : SdrDevices.SdrDevices.at(default_radio)->get_rx_sample_rates(default_rx_channel))
			{
				int v = (int)col;
				gsetup.add_sample_rate(v);
			}
		}
		catch (const std::exception& e)
		{
			std::cout << e.what();
		}
		gsetup.set_sample_rate((int)ifrate);
		try
		{
			SdrDevices.SdrDevices.at(default_radio)->setSampleRate(SOAPY_SDR_RX, 0, ifrate);
		}
		catch (const std::exception& e)
		{
			std::cout << e.what();
		}
		gui_band_instance.init_button_gui(tab["band"], LV_HOR_RES - 3, SdrDevices.get_full_frequency_range_list(default_radio, default_rx_channel));
		gbar.set_vol_slider(Settings_file.volume());
		catinterface.SetAG(Settings_file.volume());
		gbar.set_if(Settings_file.if_gain(default_radio));
		gbar.set_gain_range();
		gbar.set_gain_slider(Settings_file.gain(default_radio));	
		vfo.set_vfo(freq, false);
		if (SdrDevices.SdrDevices[default_radio]->get_bandwith_count(0))
		{
			long bw = SdrDevices.SdrDevices[default_radio]->get_bandwith(0, 0);
			SdrDevices.SdrDevices[default_radio]->setBandwidth(SOAPY_SDR_RX, 0, bw);
			vfo.vfo_re_init(ifrate, defaultAudioSampleRate, gsetup.get_span(),bw);
			printf("setBandwidth %ld \n", bw);
		}
		gsetup.init_bandwidth();
		select_mode(mode); // start streaming
	}
	else
	{
		gbar.init(bar_view, button_group, mode, LV_HOR_RES - 3, barHeight); 
		lv_label_set_text(label_status, "No SDR Device Found");
		gsetup.set_radio(default_radio);
	}

	lv_group_add_obj(button_group, lv_tabview_get_tab_btns(tabview_mid));
	/*Handle LitlevGL tasks (tickless mode)*/
	auto timeLastStatus = std::chrono::high_resolution_clock::now();
	while (1)
	{
		FT8Message msg;

		gui_mutex.lock();
		lv_task_handler();
		Mouse_dev.step_vfo();
		HidDev_dev.step_vfo();
		HidDev_dev1.step_vfo();
		const auto now = std::chrono::high_resolution_clock::now();
		if (timeLastStatus + std::chrono::milliseconds(100) < now)
		{
			timeLastStatus = now;
			Fft_calc.upload_fft(Wf.data_set);
			Wf.load_data();
			double s = Fft_calc.get_signal_strength();
			set_s_meter(s);
		}
		while (FT8Queue.pull(msg))
		{			
			msg.display();
		}
		set_time_label();
		gui_mutex.unlock();
		usleep(1000);
	}
	audio_output->close();
	delete audio_output;
	audio_input->close();
	delete audio_input;
	if (midicontrole)
		delete midicontrole;
	return 0;
}

/*Set in lv_conf.h as `LV_TICK_CUSTOM_SYS_TIME_EXPR`*/
uint32_t custom_tick_get(void)
{
	static uint64_t start_ms = 0;
	if (start_ms == 0) {
		struct timeval tv_start;
		gettimeofday(&tv_start, NULL);
		start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
	}

	struct timeval tv_now;
	gettimeofday(&tv_now, NULL);
	uint64_t now_ms;
	now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;

	uint32_t time_ms = now_ms - start_ms;
	return time_ms;
}

static bool stream_rx_on{false};

void destroy_demodulators(bool all)
{
	FMDemodulator::destroy_demodulator();
	AMDemodulator::destroy_demodulator();
	AMModulator::destroy_modulator();
	FMModulator::destroy_modulator();
	FT8Demodulator::destroy_demodulator();
	EchoAudio::destroy_modulator();
	stop_fm();
	if (all)
	{
		RX_Stream::destroy_rx_streaming_thread();
		stream_rx_on = false;
	}
	TX_Stream::destroy_tx_streaming_thread();
}

extern std::chrono::high_resolution_clock::time_point starttime1;


void select_mode(int s_mode, bool bvfo)
{
	bool stereo{false}, dc{false};

	if (!SdrDevices.isValid(default_radio))
		return;
	catinterface.Pause_Cat(true);
	// wait for threads to finish
	printf("select_mode_rx stop all threads\n");
	// stop transmit
	destroy_demodulators();
	mode = s_mode;
	if (SdrDevices.get_tx_channels(default_radio) > 0)
		Gui_tx.set_tx_state(false);
	vfo.vfo_rxtx(true, false);
	vfo.set_freq_to_sdr();
	if (bvfo)
	{
		vfo.set_mode(0, mode);
		vfo.set_vfo(0, false);
	}
	printf("select_mode_rx start rx threads\n");
	switch (mode)
	{
	case mode_narrowband_fm:
		FMDemodulator::create_demodulator(ifrate, &source_buffer_rx, audio_output);
		RX_Stream::create_rx_streaming_thread(default_radio, default_rx_channel, &source_buffer_rx);
		break;
	
	case mode_broadband_fm:
		if (audio_output->get_channels() > 1)
			stereo = true;
		else
			stereo = false;
		start_fm(ifrate, audio_output->get_samplerate(), stereo, &source_buffer_rx, audio_output);
		RX_Stream::create_rx_streaming_thread(default_radio, default_rx_channel, &source_buffer_rx); 
		break;

	case mode_cw:
		guirx.set_cw(true);
	case mode_am:
	case mode_dsb:
	case mode_usb:
	case mode_lsb:
		if (mode != mode_cw)
			guirx.set_cw(false);
		vfo.set_step(10, 0);
		printf("Start AMDemodulator\n");
		AMDemodulator::create_demodulator(mode, ifrate,  &source_buffer_rx, audio_output);
		if (!stream_rx_on)
		{
			RX_Stream::create_rx_streaming_thread(default_radio, default_rx_channel, &source_buffer_rx);
			stream_rx_on = true;
		}
		else
			pause_flag = false;
		break;
	case mode_ft8:
		vfo.set_step(10, 0);
		vfo.set_vfo(Settings_file.get_ft8(vfo.getBandIndex(vfo.get_band_no(0))), false);
		FT8Demodulator::create_demodulator(ifrate, &source_buffer_rx, audio_output);
		RX_Stream::create_rx_streaming_thread(default_radio, default_rx_channel, &source_buffer_rx);
		break;
	case mode_echo:
		//EchoAudio::create_modulator(audio_output->get_samplerate(), audio_output,audio_input);
		break;
	}
	vfo.set_freq_to_sdr();
	catinterface.Pause_Cat(false);
}

void select_mode_tx(int s_mode, int tone)
{
	// Stop all threads
	if (!SdrDevices.isValid(default_radio))
		return;
	catinterface.Pause_Cat(true);
	startTime = std::chrono::high_resolution_clock::now();
	auto now = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> timePassed = now - startTime;
	printf("select_mode_tx stop all threads time %4.2f\n", (double)timePassed.count() * 1000000.0);
	destroy_demodulators();
	pause_flag = true;
	mode = s_mode;
	Gui_tx.set_tx_state(true); // set tx button
	vfo.vfo_rxtx(false, true);
	vfo.set_vfo(0, false);
	printf("select_mode_tx start tx threads\n");
	switch (mode)
	{	
	case mode_broadband_fm:
		//start_fm_tx(ifrate, audio_output->get_samplerate(), true, &source_buffer_tx, audio_output);
		//mode_running = 1;
		break;
	
	case mode_narrowband_fm:
		FMModulator::create_modulator(mode, ifrate_tx, tone, &source_buffer_tx, audio_input);
		TX_Stream::create_tx_streaming_thread(default_radio, default_rx_channel, &source_buffer_tx, ifrate_tx);
		break;
		
	case mode_cw:
	case mode_am:
	case mode_dsb:
	case mode_usb:
	case mode_lsb:
		AMModulator::create_modulator(mode, ifrate_tx, tone, &source_buffer_tx, audio_input);
		TX_Stream::create_tx_streaming_thread(default_radio, default_rx_channel, &source_buffer_tx, ifrate_tx);
		break;
	}
	catinterface.Pause_Cat(false);
}

/*
 * To switch from SDR receiver we have to do a few things:
 * 1) Stop current threats for receive and or transmit.
 * 2) Unmake the old device in SoapySDR
 * 3) Make the new device and probe device
 * 4) Set samplerate for new device from config file
 * 6) Update the buttons in the band tabview
 * 7) Update the AGC button in button bar
 * 8) Set text in top status bar to new device and frequency range
 * 9) Check device rx samplerates and add to dropdown box in setting tab
 * 10) Check if device support transmit and add transmit sample rates to dropdown and show TX tab
 * 11) Re initialize the vfo class so:
 *		- max min supported frequency is updated
 *		- 
 *
 *
 *
 **/

void	switch_sdrreceiver(std::string receiver)
{
	/// First switchoff current receiver
	destroy_demodulators();
	SdrDevices.UnMakeDevice(default_radio);
	default_radio = receiver;
	// Hide TX page
	lv_btnmatrix_set_btn_ctrl(tab_buttons, 5, LV_BTNMATRIX_CTRL_HIDDEN);
	if (SdrDevices.MakeDevice(default_radio))
	{
		ifrate = Settings_file.find_samplerate(default_radio.c_str());
		ifrate_tx = Settings_file.find_samplerate_tx(default_radio.c_str());
		if (ifrate_tx == 0)
			ifrate_tx = ifrate;
		printf("samperate rx%f sample rate tx %f\n", ifrate, ifrate_tx);
		gbar.check_agc();
		// set top line with receiver information
		SoapySDR::Range r = SdrDevices.get_full_frequency_range(default_radio, default_rx_channel);
		std::string start_freq = std::to_string(r.minimum() / 1.0e6);
		std::string stop_freq = std::to_string(r.maximum() / 1.0e6);
		std::string s = std::string(default_radio.c_str()) + " " + start_freq + " Mhz - " + stop_freq + " Mhz";
		lv_label_set_text(label_status, s.c_str());
		if (SdrDevices.get_tx_channels(default_radio) < 1) // for now assume only 1 tx channel
			default_tx_channel = -1;
		else
			default_tx_channel = 0;
		vfo.set_vfo_range(r.minimum(), r.maximum());
		vfo.vfo_init((long)ifrate, audio_output->get_samplerate(), gsetup.get_span(),&SdrDevices, default_radio, default_rx_channel, default_tx_channel);
		try
		{
			if (SdrDevices.SdrDevices[default_radio]->get_txchannels() > 0)
			{
				lv_btnmatrix_clear_btn_ctrl(tab_buttons, 5, LV_BTNMATRIX_CTRL_HIDDEN);
				Gui_tx.clear_sample_rate();
				Gui_tx.set_sample_rate((int)ifrate_tx);
				Gui_tx.set_drv_range();
				for (auto& col : SdrDevices.SdrDevices.at(default_radio)->get_tx_sample_rates(default_tx_channel))
				{
					int v = (int)col;
					Gui_tx.add_sample_rate(v);
				}
			}
		}
		catch (const std::exception& e)
		{
			std::cout << e.what();
		}
		// Rx sample rates
		gsetup.clear_sample_rate();
		try
		{	
			for (auto& col : SdrDevices.SdrDevices.at(default_radio)->get_rx_sample_rates(default_rx_channel))
			{
				int v = (int)col;
				gsetup.add_sample_rate(v);
			}
		}
		catch (const std::exception& e)
		{
			std::cout << e.what();
		}
		gsetup.set_sample_rate((int)ifrate);
		try
		{
			SdrDevices.SdrDevices.at(default_radio)->setSampleRate(SOAPY_SDR_RX, 0, ifrate);
		}
		catch (const std::exception& e)
		{
			std::cout << e.what();
		}
		gui_band_instance.init_button_gui(nullptr, LV_HOR_RES - 3, SdrDevices.get_full_frequency_range_list(default_radio, default_rx_channel));
		gbar.set_vol_slider(Settings_file.volume());
		catinterface.SetAG(Settings_file.volume());
		gbar.set_gain_range();	
		gbar.set_gain_slider(Settings_file.gain());	
		//vfo.set_vfo(freq, false);
		select_mode(mode); // start streaming
	}
}


/*
 *
 *const auto startTime = std::chrono::high_resolution_clock::now();
		const auto now = std::chrono::high_resolution_clock::now();
		const auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);			
		printf("create demodulator %lld\n", timePassed.count());
 *
 **/