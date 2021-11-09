#include "sdrberry.h"
#include "Mouse.h"
#include "Catinterface.h"
#include "BandFilter.h"

AudioOutput *audio_output;
AudioInput *audio_input;
DataBuffer<IQSample>	source_buffer_rx;
DataBuffer<IQSample16>	source_buffer_tx;
DataBuffer<Sample>		audiooutput_buffer;
DataBuffer<Sample>		audioinput_buffer;

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
const int barHeight = 45;
const int nobuttons = 8;
const int bottombutton_width = (screenWidth / nobuttons) - 2;
const int bottombutton_width1 = (screenWidth / nobuttons);



lv_obj_t* scr;
lv_obj_t* bg_middle;
lv_obj_t* vfo1_button;
lv_obj_t* vfo2_button;
lv_obj_t *tabview_mid;
lv_obj_t *bar_view;

using namespace std;

atomic_bool stop_flag(false);
atomic_bool stop_tx_flag(false);
atomic_bool	stop_txmod_flag(false);

std::mutex gui_mutex;

int mode = mode_broadband_fm;
double  ifrate  = 0.53e6;  //1.0e6;//
double	ifrate_tx;
bool    stereo  = true;
int     pcmrate = 48000;
double	freq = 89800000;
volatile int		filter	= 4;
//double	tuner_freq = freq + 0.25 * ifrate;
//double	tuner_offset = freq - tuner_freq;

mutex	am_finish;
mutex	am_tx_finish;
mutex	fm_finish;
mutex	stream_finish;

Mouse			Mouse_dev;
Catinterface	catinterface;
BandFilter		bpf;

MidiControle	*midicontrole = nullptr;
auto			startTime = std::chrono::high_resolution_clock::now();

int main(int argc, char *argv[])
{
	
	Settings_file.read_settings(std::string("sdrberry_settings.cfg"));
	Mouse_dev.init_mouse(Settings_file.find_input("mouse"));

	catinterface.begin();
	std::thread thread_catinterface(std::ref(catinterface));
	thread_catinterface.detach();

	string s = Settings_file.find_audio("device");	
	audio_output = new AudioOutput();
	if (!(*audio_output)) {
		fprintf(stderr, "ERROR: AudioOutput\n");
		exit(1);
	}
	audio_output->init(s, pcmrate);


	//(char *)s.c_str(), pcmrate
	audio_input = new AudioInput();
	if (!(*audio_input)) {
		fprintf(stderr, "ERROR: AudioInput\n");
	}	
	audio_input->init(s, pcmrate, false, &audioinput_buffer);
	audio_output->set_volume(50);
	
	bpf.initFilter();
	
	std::string smode = Settings_file.find_vfo1("Mode");
	mode = Settings_file.convert_mode(smode);
	
	/*LittlevGL init*/
	lv_init();

	/*Linux frame buffer device init*/
	fbdev_init();

	// Touch pointer device init
	evdev_init();
	
	/*A small buffer for LittlevGL to draw the screen's content*/
	static lv_color_t buf[DISP_BUF_SIZE];

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
	lv_obj_set_pos(bar_view, 0, topHeight + tunerHeight );
	lv_obj_set_size(bar_view, LV_HOR_RES - 3, barHeight);
	gbar.init(bar_view, mode, LV_HOR_RES - 3, barHeight);
	
	tabview_mid = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, 40);
	lv_obj_set_pos(tabview_mid, 0, topHeight + tunerHeight + barHeight);
	lv_obj_set_size(tabview_mid, LV_HOR_RES - 3, screenHeight - topHeight - tunerHeight - barHeight);
	
	lv_obj_t *tab1 = lv_tabview_add_tab(tabview_mid, "Spectrum");
	lv_obj_t *tab2 = lv_tabview_add_tab(tabview_mid, "Band");
	lv_obj_t *tab3 = lv_tabview_add_tab(tabview_mid, "Frequency");
	lv_obj_t *tab4 = lv_tabview_add_tab(tabview_mid, "Mode");
	lv_obj_t *tab5 = lv_tabview_add_tab(tabview_mid, "Agc");
	lv_obj_t *tab6 = lv_tabview_add_tab(tabview_mid, "TX");
	lv_obj_t *tab7 = lv_tabview_add_tab(tabview_mid, "Setup");
	
	lv_obj_clear_flag(lv_tabview_get_content(tabview_mid), LV_OBJ_FLAG_SCROLL_CHAIN | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_ONE);

	Wf.init(tab1, 0, 0, LV_HOR_RES - 3, screenHeight - topHeight - tunerHeight);	
	Gui_rx.gui_rx_init(tab4, LV_HOR_RES - 3);
	Gui_tx.gui_tx_init(tab6, LV_HOR_RES - 3);
	gagc.init(tab5, LV_HOR_RES - 3);
		
 	lv_obj_t *label1 = lv_label_create(tab7);
	lv_label_set_text(label1, "Setup");
	
	
	if (Settings_file.get_mac_address() != std::string(""))
	{
		//create_ble_thread(Settings_file.get_mac_address());
		
		Ble_instance.set_mac_address(Settings_file.get_mac_address());
		std::thread thread_ble(std::ref(Ble_instance));
		thread_ble.detach();
	}
	
	
	//Ble_instance.set_mac_address(Settings_file.get_mac_address());
	//Ble_instance.setup_ble();	
	

	Gui_rx.set_gui_mode(mode);
		
	ifrate = Settings_file.find_samplerate(Settings_file.find_sdr("default").c_str());
	ifrate_tx = Settings_file.find_samplerate_tx(Settings_file.find_sdr("default").c_str());
	printf("samperate %f \n", ifrate);
	keyb.init_keyboard(tab3, LV_HOR_RES - 3, screenHeight - topHeight - tunerHeight);
	
	std::string default_radio = Settings_file.find_sdr("default");
	std::cout << "default sdr: " << Settings_file.find_sdr("default").c_str() << std::endl;
	
		
	midicontrole = new(MidiControle);
	if (midicontrole)
	{
		int ports = midicontrole->check_midi_input();
		if (ports > 1)
			midicontrole->openport(1);
	}
	
	freq = Settings_file.find_vfo1_freq("freq");
	if (discover_devices(Settings_file.find_probe((char *)default_radio.c_str())) == EXIT_SUCCESS)
	{	
		soapy_devices[0].rx_channel = 0;
		std::cout << "probe: " << Settings_file.find_probe((char *)default_radio.c_str()).c_str() << std::endl;
		std::string start_freq = std::to_string(soapy_devices[0].channel_structure_rx[soapy_devices[0].rx_channel].full_frequency_range.front().minimum() / 1.0e6);
		std::string stop_freq = std::to_string(soapy_devices[0].channel_structure_rx[soapy_devices[0].rx_channel].full_frequency_range.front().maximum() / 1.0e6);
		std::string s = std::string(soapy_devices[0].driver.c_str()) + " " + start_freq + " Mhz - " + stop_freq + " Mhz";
		lv_label_set_text(label_status, s.c_str()); 
		vfo.set_vfo_capability(&soapy_devices[0]);
		vfo.set_vfo_range(soapy_devices[0].channel_structure_rx[soapy_devices[0].rx_channel].full_frequency_range.front().minimum(),
			soapy_devices[0].channel_structure_rx[soapy_devices[0].rx_channel].full_frequency_range.front().maximum());
			
		vfo.vfo_init((long)ifrate, pcmrate, soapy_devices[0].channel_structure_rx[soapy_devices[0].rx_channel].full_frequency_range);
			for (auto& col : soapy_devices[0].channel_structure_rx[0].bandwidth_range)
		{
			int v = col.minimum();
			Gui_rx.add_sample_rate(v);
		}
		Gui_rx.set_sample_rate((int)ifrate);
		for (auto& col : soapy_devices[0].channel_structure_tx[0].bandwidth_range)
		{
			int v = col.minimum();
			Gui_tx.add_sample_rate(v);
		}
		Gui_tx.set_sample_rate((int)ifrate_tx);
		soapy_devices[0].channel_structure_tx[0].source_buffer_tx = &source_buffer_tx;
		soapy_devices[0].channel_structure_rx[0].source_buffer_rx = &source_buffer_rx;
		soapy_devices[0].sdr->setSampleRate(SOAPY_SDR_RX, 0, ifrate);
		gui_band_instance.init_button_gui(tab2, LV_HOR_RES - 3, soapy_devices[0].channel_structure_rx[soapy_devices[0].rx_channel].full_frequency_range);
		gbar.set_vol_slider(Settings_file.volume());		
		gagc.set_gain_range();
		Gui_tx.set_drv_range();
		gagc.set_gain_slider(Settings_file.gain());	
		vfo.set_vfo(freq, false);
		select_mode(mode); // start streaming
	}
	else
	{
		lv_label_set_text(label_status, "No SDR Device Found"); 
	}
	
	
	/*Handle LitlevGL tasks (tickless mode)*/
	auto timeLastStatus = std::chrono::high_resolution_clock::now();
	while (1) {
		unique_lock<mutex> gui_lock(gui_mutex); 
		lv_task_handler();
		gui_lock.unlock(); 
		Mouse_dev.step_vfo();
		const auto now = std::chrono::high_resolution_clock::now();
		if (timeLastStatus + std::chrono::milliseconds(200) < now && !stop_flag.load())
		{
			timeLastStatus = now;
			Fft_calc.upload_fft(Wf.data_set);
			Wf.load_data();
			double s = Fft_calc.get_signal_strength();
			set_s_meter(s);
		}
		
		if (midicontrole)
			midicontrole->read_midi_input();
		set_time_label();
		usleep(5000);
	}
	delete audio_output;
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

static int mode_running = 0;

void select_mode(int s_mode)
{	
	startTime = std::chrono::high_resolution_clock::now(); 
	// wait for threads to finish
	printf("select_mode_rx stop all threads\n");
	// stop transmit and close audio input
	
	printf("stop am_tx\n");
	stop_txmod_flag = true;
	unique_lock<mutex> lock_am_tx(am_tx_finish); 
	printf("stop am_finish \n");
	stop_flag = true;
	unique_lock<mutex> lock_am(am_finish); 
	printf("stop fm_finish \n");
	unique_lock<mutex> lock_fm(fm_finish); 
	lock_am.unlock();
	lock_fm.unlock();
	lock_am_tx.unlock();
	audio_input->close(); 
	audio_output->open(&audiooutput_buffer);
	
	mode_running = 0;
	stop_tx_flag = false;
	stop_flag = false;
	stop_txmod_flag = false;
	mode = s_mode;
	Gui_tx.set_tx_state(false);
	vfo.vfo_rxtx(true, false);
	vfo.set_vfo(0,false);
	vfo.set_mode(0, mode);
	printf("select_mode_rx start rx threads\n");
	switch (mode)
	{
	case mode_broadband_fm:
		start_fm(ifrate, pcmrate, true, &source_buffer_rx, audio_output);
		mode_running = 1;
		create_rx_streaming_thread(&soapy_devices[0]); 
		break;
		
	case mode_am:
	case mode_dsb:
	case mode_usb:
	case mode_lsb:
		start_dsb(mode, ifrate, pcmrate, &source_buffer_rx, audio_output);
		mode_running = 2;
		break;
	}
}

void select_filter(int ifilter)
{
	filter = ifilter;
}

void select_mode_tx(int s_mode, int tone)
{
	// Stop all threads
	startTime = std::chrono::high_resolution_clock::now();
	auto now = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> timePassed = now - startTime;
	printf("select_mode_tx stop all threads time %4.2f\n", (double)timePassed.count() * 1000000.0);
	
	//printf("select_mode_tx stop all threads\n");
	stop_flag = true;
	mode_running = 0;
	// wait for threads to finish
	unique_lock<mutex> lock_am(am_finish); 
	unique_lock<mutex> lock_fm(fm_finish); 
	unique_lock<mutex> lock_stream(stream_finish); 
	unique_lock<mutex> lock_am_tx(am_tx_finish); 
	
	lock_am.unlock();
	lock_fm.unlock();
	lock_stream.unlock();
	lock_am_tx.unlock();
	stop_flag = false;
	mode = s_mode;
	audio_output->close();
	if (tone == 0)
		audio_input->open();
	Gui_tx.set_tx_state(true); // set tx button
	vfo.vfo_rxtx(false, true);
	vfo.set_vfo(0, false);
	printf("select_mode_tx start tx threads\n");
	soapy_devices[0].sdr->setGain(SOAPY_SDR_TX, soapy_devices[0].tx_channel, (double)Gui_tx.get_drv_pos());
	switch (mode)
	{
	case mode_broadband_fm:
		//start_fm_tx(ifrate, pcmrate, true, &source_buffer_tx, audio_output);
		//mode_running = 1;
		break;
		
	case mode_am:
	case mode_dsb:
	case mode_usb:
	case mode_lsb:
		start_dsb_tx(mode, ifrate, pcmrate, tone, &source_buffer_tx, audio_input);
		mode_running = 2;
		break;
	}
}

