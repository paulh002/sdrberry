#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "wstring.h"
#include "lvgl/lvgl.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"
#include "devices.h"
#include "gui_right_pane.h"
#include "gui_top_bar.h"
#include "ble_interface.h"
#include "vfo.h"
#include "sdrstream.h"
#include "gui_vfo.h"
#include "Settings.h"
//#include "FMDemodulator.h"

#include <memory>
#include "SoftFM.h"
#include "FmDecode.h"
#include "AudioOutput.h"
#include "DataBuffer.h"

AudioOutput *audio_output;
	
LV_FONT_DECLARE(FreeSansOblique42);
LV_FONT_DECLARE(FreeSansOblique32);

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

// display buffer size - not sure if this size is really needed
#define DISP_BUF_SIZE 384000		// 800x480

const int screenWidth = 800;
const int screenHeight = 480;
const int bottomHeight = 40;
const int topHeight = 55;
const int tunerHeight = 70;
const int nobuttons = 8;
const int rightWidth = 200;
const int bottombutton_width = (screenWidth / nobuttons) - 2;
const int bottombutton_width1 = (screenWidth / nobuttons);

lv_obj_t* scr;
lv_obj_t* bg_middle;
lv_obj_t* vfo1_button;
lv_obj_t* vfo2_button;
lv_obj_t *tabview_mid;

static lv_style_t style_btn;

using namespace std;

atomic_bool stop_flag(false);
std::mutex gui_mutex;

int main(int argc, char *argv[])
{
	
	Settings_file.read_settings(String("sdrberry_settings.cfg"));
	
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
	setup_right_pane(scr);
	
	static lv_style_t background_style;
		
	lv_style_init(&background_style);
	lv_style_set_radius(&background_style, 0);
	lv_style_set_bg_color(&background_style, lv_palette_main(LV_PALETTE_RED));
	
	tabview_mid = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, 40);
	lv_obj_set_pos(tabview_mid, 0, topHeight + tunerHeight);
	lv_obj_set_size(tabview_mid, LV_HOR_RES - rightWidth - 3, screenHeight - topHeight - tunerHeight);
	
	lv_obj_t *tab1 = lv_tabview_add_tab(tabview_mid, "Waterfall");
	lv_obj_t *tab2 = lv_tabview_add_tab(tabview_mid, "Band");
	lv_obj_t *tab3 = lv_tabview_add_tab(tabview_mid, "RX");
	lv_obj_t *tab4 = lv_tabview_add_tab(tabview_mid, "TX");
	lv_obj_t *tab5 = lv_tabview_add_tab(tabview_mid, "Setup");
	
	lv_obj_t * label1 = lv_label_create(tab1);
	lv_label_set_text(label1, "Waterfall");
	
	label1 = lv_label_create(tab2);
	lv_label_set_text(label1, "Band");

	label1 = lv_label_create(tab3);
	lv_label_set_text(label1, "RX");

	label1 = lv_label_create(tab4);
	lv_label_set_text(label1, "TX");
	
	label1 = lv_label_create(tab5);
	lv_label_set_text(label1, "Setup");
	

	/*
	bg_middle = lv_obj_create(scr);
	lv_obj_add_style(bg_middle, &background_style, 0);
	lv_obj_set_pos(bg_middle, 0, topHeight + tunerHeight);
	lv_obj_set_size(bg_middle, LV_HOR_RES - rightWidth -3, screenHeight - topHeight - tunerHeight);
	
	
	
	lv_style_init(&style_btn);

	lv_style_set_radius(&style_btn, 10);
	lv_style_set_bg_color(&style_btn, lv_color_make(0x60, 0x60, 0x60));
	lv_style_set_bg_grad_color(&style_btn, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_bg_grad_dir(&style_btn, LV_GRAD_DIR_VER);
	lv_style_set_bg_opa(&style_btn, 255);
	lv_style_set_border_color(&style_btn, lv_color_make(0x9b, 0x36, 0x36));  // lv_color_make(0x2e, 0x44, 0xb2)
	lv_style_set_border_width(&style_btn, 2);
	lv_style_set_border_opa(&style_btn, 255);
	lv_style_set_outline_color(&style_btn, lv_color_black());
	lv_style_set_outline_opa(&style_btn, 255);
	
	vfo1_button = lv_btn_create(bg_middle);
	lv_obj_add_style(vfo1_button, &style_btn,0); 
	//lv_obj_set_event_cb(vfo1_button, mode_button_vfo);
	lv_obj_align(vfo1_button, LV_ALIGN_BOTTOM_LEFT, 0 * bottombutton_width1, 0);
	//lv_btn_set_checkable(vfo1_button, true);
	//lv_btn_toggle(vfo1_button);
	lv_obj_set_size(vfo1_button, bottombutton_width, bottomHeight);
	lv_obj_t* label = lv_label_create(vfo1_button);
	lv_label_set_text(label, "Vfo 1");
	lv_obj_center(label);
	
	vfo2_button = lv_btn_create(bg_middle);
	lv_obj_add_style(vfo2_button, &style_btn, 0); 
	//lv_obj_set_event_cb(vfo2_button, mode_button_vfo);
	lv_obj_align(vfo2_button, LV_ALIGN_BOTTOM_LEFT, 1 * bottombutton_width1, 0);
	//lv_btn_set_checkable(vfo2_button, true);
	lv_obj_set_size(vfo2_button, bottombutton_width, bottomHeight);
	label = lv_label_create(vfo2_button);
	lv_label_set_text(label, "Vfo 2");
	lv_obj_center(label);
	*/
	
	//setup_ble(String("7c:9e:bd:f8:64:92"));
	if (Settings_file.get_mac_address() != String(""))
		Ble_instance.setup_ble(Settings_file.get_mac_address());
	
	double  ifrate  = 1.0e6;
	int     pcmrate = 48000;
	bool    stereo  = true;
	
	//unique_ptr<AudioOutput> audio_output;
	//audio_output.reset(new AlsaAudioOutput("default", pcmrate, stereo));
	String s = Settings_file.find_audio("device");
	
	audio_output = new AlsaAudioOutput((char *)s.c_str(), pcmrate, stereo);
	if (!(*audio_output)) {
		fprintf(stderr,
			"ERROR: AudioOutput: %s\n",
			audio_output->error().c_str());
		exit(1);
	}
	audio_output->set_volume(50);
	
	double freq = 89950000;
	double tuner_freq = freq + 0.25 * ifrate;
	double tuner_offset = freq - tuner_freq;
	
	vfo_init((unsigned long)freq);
	
	String default_radio = Settings_file.find_sdr("default");
	std::cout << "default sdr: " << Settings_file.find_sdr("default").c_str() << std::endl;
	if (discover_devices(Settings_file.find_probe((char *)default_radio.c_str())) == EXIT_SUCCESS)
	{	
		soapy_devices[0].rx_channel = 0;
		std::cout << "probe: " << Settings_file.find_probe((char *)default_radio.c_str()).c_str() << std::endl;
		String start_freq = String(soapy_devices[0].channel_structure_rx[soapy_devices[0].rx_channel].full_frequency_range.front().minimum() / 1e6);
		String stop_freq = String(soapy_devices[0].channel_structure_rx[soapy_devices[0].rx_channel].full_frequency_range.front().maximum() / 1e6);
		String s = String(soapy_devices[0].driver.c_str()) + " " + start_freq + " Mhz - " + stop_freq + " Mhz";
		lv_label_set_text(label_status, s.c_str()); 
		set_vfo_capability(&soapy_devices[0]);
		set_vfo(0, 11, freq);

		DataBuffer<IQSample> source_buffer;
		create_rx_streaming_thread(&soapy_devices[0], &vfo_setting, ifrate, &source_buffer);
		double bandwidth_pcm = MIN(15000, 0.45 * pcmrate);
		create_fm_thread(ifrate, tuner_offset, pcmrate, true, bandwidth_pcm, 1, &source_buffer, audio_output);
		set_vol_slider(50);
		
		double gain = soapy_devices[0].sdr->getGain(SOAPY_SDR_RX, 0);
		set_gain_slider((int)gain);
	}
	else
	{
		lv_label_set_text(label_status, "No SDR Device Found"); 
	}
	
	/*Handle LitlevGL tasks (tickless mode)*/
	while (1) {
		//std::unique_lock<std::mutex> lock(gui_mutex);
		lv_task_handler();
		//std::unique_lock<std::mutex> unlock(gui_mutex);
		Ble_instance.connect();
		usleep(5000);
	}
	delete audio_output;
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

