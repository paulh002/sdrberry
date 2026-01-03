#include "AMDemodulator.h"
#include "AMModulator.h"
#include "BandFilter.h"
#include "Catinterface.h"
#include "Demodulator.h"
#include "EchoAudio.h"
#include "FMDemodulator.h"
#include "FMModulator.h"
#include "FT8Demodulator.h"
#include "HidDev.h"
#include "Mouse.h"
#include "SharedQueue.h"
#include "gui_speech.h"
#include "GuiFt8Setting.h"
#include "gui_ft8bar.h"
#include "Keyboard.h"
#include "Spectrum.h"
#include "FreeDVTab.h"
#include "gui_cal.h"
#include "wsjtx_lib.h"
#include "gui_agc.h"
#include "gui_rx.h"
#include "gui_bar.h"
#include "gui_tx.h"
#include "gui_setup.h"
#include "gui_ft8.h"
#include "gui_bottom_bar.h"
#include "Gui_band.h"
#include "gui_sdr.h"
#include "gui_squelch.h"
#include "gui_preset.h"
#include "AudioInput.h"
#include "AudioOutput.h"
#include "Catinterface.h"
#include "CatTcpServer.h"
#include "DataBuffer.h"
#include "Filter.h"
#include "FmDecode.h"
#include "MidiControle.h"
#include "Modes.h"
#include "RtAudio.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"
#include "Settings.h"
#include "gui_top_bar.h"
#include "sdrstream.h"
#include "sdrberry.h"
#include "CustomEvents.h"
#include "WebServer.h"
#include <nlohmann/json.hpp>
#include <sys/file.h>
#include "i2cinput.h"
#include "i2coutput.h"
#include "gui_gain.h"
#include "strlib.h"
#include "tz.h"
#include "Shuttle.h"

using json = nlohmann::json;
// test
//#include "quick_arg_parser.hpp"

//#include "HidThread.h"
/*
struct Args : MainArguments<Args>
{
	bool verbose = option("verbose", 'v');
	std::filesystem::path file = argument(0);
	std::string sdrRadio = option("SdrRadio", 'r').validator([](std::string sdrRadio) { return sdrRadio == "radioberry" || sdrRadio == "hifiberry" || sdrRadio == "pluto" || sdrRadio == "hackrf" || sdrRadio == "rtlsdr" || sdrRadio == "sdrplay"; });
};
*/

/* print stacktrace
 	StackTrace st;
	st.load_here(32);
	Printer p;
	p.object = true;
	p.color_mode = ColorMode::always;
	p.address = true;
	p.print(st, stdout);
*/
#define LOCK_FILE "/tmp/sdrberry.lock"
int fd_lock_file = 0;

bool isAlreadyRunning()
{
	fd_lock_file = open(LOCK_FILE, O_CREAT | O_RDWR, 0666);
	if (fd_lock_file == -1)
	{
		std::cerr << "Failed to open lock file.\n";
		return true;
	}

	if (flock(fd_lock_file, LOCK_EX | LOCK_NB) == -1)
	{
		std::cerr << "Another instance of sdrberry is already running.\n";
		std::cerr << "Reboot and start sdrberry again\n";
		if (fd_lock_file > 0)
			close(fd_lock_file);
		return true;
	}

	// Keep the lock file open to maintain the lock
	return false;
}

void handle_signal(int signal)
{
	if (signal == SIGINT)
	{
		if (fd_lock_file > 0)
			close(fd_lock_file);
		remove(LOCK_FILE);
		std::cout << "\nCaught Ctrl+C (SIGINT), exiting gracefully..." << std::endl;
		// You can perform cleanup or any other actions here
		audio_input->close();
		audio_output->close();
		exit(0); // Exit program
	}
}

#define BACKWARD_HAS_BFD 1
#define BACKWARD_HAS_DW 1
#include "backward.hpp"

namespace backward
{
	backward::SignalHandling sh;

} // namespace backward

DataBuffer<IQSample> source_buffer_rx;
DataBuffer<IQSample> source_buffer_tx;

LV_FONT_DECLARE(FreeSansOblique42);
LV_FONT_DECLARE(FreeSansOblique32);

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

// display buffer size - not sure if this size is really needed
#define DISP_BUF_SIZE 29491200 //384000 // 800x480
//#define DISP_BUF_SIZE (128 * 1024)

//const std::array<int, 10> screenResolutionsWidth =  {3840, 3200, 2560, 2048, 1920, 1600, 1280, 1024, 800, 800};
//const std::array<int, 10> screenResolutionsHeight = {2160, 1800, 1440, 1080, 1080, 1200, 720 ,  768, 600, 480};
const std::array<int, 11> screenResolutionsWidth = {800, 800, 1024, 1024, 1280, 1600, 1920, 2048, 2560, 3200, 3840};
const std::array<int, 11> screenResolutionsHeight = {480, 600, 600, 768, 720, 1200, 1080, 1080, 1440, 1800, 2160};

int screenSelect = 9;
int screenRotate = 0;
int screenWidth = 800;
int screenHeight = 480;
const int bottomHeight = 40;
const int topHeight = 35;
const int tunerHeight = 100;
const int barHeight = 110; //90;
int barHeightft8 = 125; //90;
const int MorseHeight = 30;
const int nobuttons = 8;
const int bottombutton_width = (screenWidth / nobuttons) - 2;
const int bottombutton_width1 = (screenWidth / nobuttons);
const int buttonHeight = 40;
int tabHeight = screenHeight - topHeight - tunerHeight - barHeight;
const int defaultAudioSampleRate{48000};
const int hidetx{4};
//const int hidespeech{5};

std::mutex gui_mutex;

lv_color_t *display_buf;
lv_obj_t *scr;
lv_obj_t *bg_middle;
lv_obj_t *vfo1_button;
lv_obj_t *vfo2_button;
lv_obj_t *tabview_mid;
lv_obj_t *bar_view, *ft8bar_view, *calbar_view, *bottom_bar_view;
lv_obj_t *tab_buttons;
lv_indev_t *encoder_indev_t{nullptr};
lv_group_t *button_group{nullptr};
lv_group_t *keyboard_group{nullptr};
extern lv_img_dsc_t mouse_cursor_icon;

using namespace std;

int mode = mode_broadband_fm;
double ifrate = 0.53e6; //1.0e6;//
double ifrate_tx;
int mode_state_rxtx = 0;
int default_rx_channel = 0;
int default_tx_channel = 0;

//double freq = 89800000;
//double	tuner_freq = freq + 0.25 * ifrate;
//double	tuner_offset = freq - tuner_freq;

mutex fm_finish;
Catinterface catinterface;
CatTcpServer cattcpserver;
Keyboard KeyboardDevice;
Mouse Mouse_dev;
HidDev HidDev_dev, HidDev_dev1, HidDev_dev2;
Shuttle shuttle;
BandFilter bpf;
SharedQueue<GuiMessage> guiQueue;
unique_ptr<wsjtx_lib> wsjtx;
WebServer webserver;
i2coutput i2c_output;

MidiControle *midicontrole = nullptr;
auto startTime = std::chrono::high_resolution_clock::now();

SdrDeviceVector SdrDevices;
std::string default_radio;


static std::string keysRed;

int IsScreenRotated() 
{ 
	return screenRotate; 
}

void keyboard_read(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
	static bool dummy_read = false;

	keysRed += KeyboardDevice.GetKeys();
	/*Send a release manually*/
	if (dummy_read)
	{
		dummy_read = false;
		data->state = LV_INDEV_STATE_RELEASED;
		data->continue_reading = keysRed.length() > 0;
	}
	/*Send the pressed character*/
	else if (keysRed.length() > 0)
	{
		dummy_read = true;
		data->state = LV_INDEV_STATE_PRESSED;
		data->key = keysRed.at(0);
		keysRed.erase(0, 1);
		data->continue_reading = true;
	}
}

int rotary_rotation{};
bool rotations = false;

void mouse_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
	MouseState state;
	/*Get the current x and y coordinates*/
	state = Mouse_dev.GetMouseState();
	data->point.x = state.x;
	data->point.y = state.y;
	rotary_rotation = state.Rotated;
	data->btn_id = state.btn_id;
	
	/*Get whether the mouse button is pressed or released*/
	if (state.pressed)
	{
		data->state = LV_INDEV_STATE_PR;
	}
	else
	{
		data->state = LV_INDEV_STATE_REL;
	}
}

void mouse_keyboard_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
	MouseState state;
	/*Get the current x and y coordinates*/
	state = KeyboardDevice.GetMouseState();
	data->point.x = state.x;
	data->point.y = state.y;
	rotary_rotation = state.Rotated;

	/*Get whether the mouse button is pressed or released*/
	if (state.pressed && state.btn_id == BTN_LEFT)
	{
		data->state = LV_INDEV_STATE_PR;
	}
	else
	{
		data->state = LV_INDEV_STATE_REL;
	}
}

void encoder_read(lv_indev_drv_t *drv, lv_indev_data_t *data){
	//data->enc_diff = HidDev_dev.encoder_rotate();
	//data->state = HidDev_dev.encoder_key_press();

	if (rotations)
		data->enc_diff = rotary_rotation;
	//data->state = HidDev_dev.encoder_key_press();
	rotary_rotation = 0;
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
		if (gsetup.get_calibration())
		{
			gcal.hide(false);
		}
		else
		{
			gbar.hide(false);
			guift8bar.hide(true);
		}		
		lv_indev_set_group(encoder_indev_t, button_group);
		break;
	case 1:
		gcal.hide(true);
		gbar.hide(false);
		guift8bar.hide(true);
		gui_band_instance.set_group();
		break;
	case 2:
		gcal.hide(true);
		gbar.hide(false);
		guift8bar.hide(true);
		break;
	case 3:
		gcal.hide(true);
		gbar.hide(false);
		guift8bar.hide(true);
		guisquelch.set_group();
		break;
	case 4:
		gcal.hide(true);
		gbar.hide(false);
		guift8bar.hide(true);
		Gui_tx.set_group();
		break;
	case 5:
		if (gsetup.get_calibration())
		{
			gcal.hide(false);
			gbar.hide(true);
		}
		else
		{
			gcal.hide(true);
			gbar.hide(false);
		}
		guift8bar.hide(true);
		Gui_tx.set_group();
		break;
	case 6:
		gcal.hide(true);
		gbar.hide(true);
		guift8bar.hide(false);
		lv_indev_set_group(encoder_indev_t, button_group);
		break;
	case 7:
		gcal.hide(true);
		gbar.hide(false);
		guift8bar.hide(true);
		guisdr.set_group();
		break;
	case 8:
		if (gsetup.get_calibration())
		{
			gcal.hide(false);
			gbar.hide(true);
		}
		else
		{
			gcal.hide(true);
			gbar.hide(false);
		}
		guift8bar.hide(true);
		guift8setting.set_group();
		break;
	}
}

void process_vfo_message(GuiMessage msg)
{
	float freqf;
	long freq;
	std::string buf;

	// printf("Message %s \n", msg.text.c_str());
	buf = msg.text;
	if (msg.text.find(" ") != string::npos)
	{
		buf = msg.text.substr(0, msg.text.find(" "));
		if (buf.length() == 0)
			buf = msg.text.substr(0, msg.text.find("M"));
		if (buf.length() == 0)
			buf = msg.text.substr(0, msg.text.find("m"));
		if (buf.length() == 0)
			buf = msg.text.substr(0, msg.text.find("K"));
		if (buf.length() == 0)
			buf = msg.text.substr(0, msg.text.find("k"));
	}
	if (buf.length() > 0 && strlib::has_any_digits(buf))
	{
		freqf = std::stof(buf);
		if (msg.text.find("M") != string::npos)
			freqf = freqf * 1000000;
		if (msg.text.find("K") != string::npos)
			freqf = freqf * 1000;
		if (msg.text.find("m") != string::npos)
			freqf = freqf * 1000000;
		if (msg.text.find("k") != string::npos)
			freqf = freqf * 1000;
		freq = freqf;
		if (vfo.is_vfo_limit_ham_band())
		{
			if (vfo.checkVfoBandRange(freq))
			{
				vfo.set_band_freq(freq);
			}
			else
			{
				static const char *btns[] = {""};
				lv_obj_t *mbox1 = lv_msgbox_create(NULL, "VFO", "Out of Ham band range", btns, true);
				lv_obj_center(mbox1);
			}
		}
		else
		{
			vfo.set_vfo(freq);
		}
	}
}

int main(int argc, char *argv[])
{
	if (isAlreadyRunning())
		return 0;

	Settings_file.read_settings(std::string("sdrberry_settings.cfg"));
	std::string timezone = Settings_file.get_string("Radio", "timezone");
	auto t = make_zoned(date::current_zone(), date::floor<std::chrono::seconds>(std::chrono::system_clock::now()));
	if (timezone.size())
	{
		try
		{
			auto zone = date::locate_zone(timezone);
			t = make_zoned(zone, date::floor<std::chrono::seconds>(std::chrono::system_clock::now()));
		}
		catch (const date::nonexistent_local_time &e)
		{
			std::cout << e.what() << '\n';
		}
	}
	std::cout << t << '\n'; // 2016-05-14 18:33:24.205 EDT

	signal(SIGINT, handle_signal); // Catch Ctrl+C (SIGINT)
	
	default_radio = Settings_file.find_sdr("default");
	screenSelect = Settings_file.get_int("screen","resolution", 0);
	screenRotate = Settings_file.get_int("screen", "rotation", 0);
	screenWidth = screenResolutionsWidth.at(screenSelect);
	screenHeight = screenResolutionsHeight.at(screenSelect);

	tabHeight = screenHeight - topHeight - tunerHeight - barHeight;

	int touchswapxy = Settings_file.get_int("input", "touch_swap_xy", 0);
	int debugswapxy = Settings_file.get_int("input", "touch_debug", 0);
	std::string touchscreen = Settings_file.get_string("input", "touchscreen");
	evdev_touch_swap(touchswapxy, debugswapxy);
	evdev_touch_driver(touchscreen.c_str());
	printf("Screen resolution %d x %d screenRotate %d touch_swap setting %d\n", screenWidth, screenHeight, screenRotate, touchswapxy);

	wsjtx = make_unique<wsjtx_lib>();
	KeyboardDevice.init_keyboard();
	Mouse_dev.init_mouse();
	//HidDev_dev.init("CONTOUR DESIGN SHUTTLEXPRESS");
	shuttle.start();
	HidDev_dev1.init("GN Audio A/S Jabra Evolve2 30 Consumer Control");
	std::string hid_device = "HID " + Settings_file.get_string("input", "hiddevice");
	if (hid_device.length())
		HidDev_dev2.init(hid_device);
	else
		HidDev_dev2.init("HID 413d:553a");

	catinterface.begin();
	std::thread thread_catinterface(std::ref(catinterface));
	thread_catinterface.detach();

	cattcpserver.StartServer();
	std::thread thread_cattcpserver(std::ref(cattcpserver));
	thread_cattcpserver.detach();
	
	int audiodevID;
	audiodevID = AudioInput::createAudioInputDevice(defaultAudioSampleRate, 2048);
	AudioOutput::createAudioDevice(defaultAudioSampleRate, 1024, audiodevID);
	
	bpf.initFilter();

	std::string smode = Settings_file.get_string("VFO1","Mode");
	mode = Settings_file.convert_mode(smode);

	std::string i2c_output_device = Settings_file.get_string("i2c", "output_device");
	std::string i2c_output_address = Settings_file.get_string("i2c", "output_address");
	if (i2c_output_device.size() > 0 && i2c_output_address.size() > 0)
	{
		i2c_output.initI2Cdevice(i2c_output_device, i2c_output_address);
		i2c_output.clear();
	}

	/*LittlevGL init*/
	lv_init();

#if USE_FBDEV
	/*Linux frame buffer device init*/
	fbdev_init();

	// Touch pointer device init
	evdev_init();
	
	/*A small buffer for LittlevGL to draw the screen's content*/
	const int dispbuffersize = LV_COLOR_DEPTH * screenWidth * screenHeight;
	//static lv_color_t buf[dispbuffersize];
	//static lv_color_t buf1[DISP_BUF_SIZE];
	display_buf = (lv_color_t *)malloc(dispbuffersize * sizeof(lv_color_t));

	/*Initialize a descriptor for the buffer*/
	static lv_disp_draw_buf_t disp_buf;
	lv_disp_draw_buf_init(&disp_buf, display_buf, NULL, dispbuffersize);

	/*Initialize and register a display driver*/
	static lv_disp_drv_t disp_drv;
	lv_disp_drv_init(&disp_drv);
	disp_drv.draw_buf = &disp_buf;
	disp_drv.flush_cb = fbdev_flush;
	if (screenRotate == 1 || screenRotate == 3)
	{
		disp_drv.hor_res = screenHeight;
		disp_drv.ver_res = screenWidth;
	}
	else
	{
		disp_drv.hor_res = screenWidth;
		disp_drv.ver_res = screenHeight;
	}
	lv_disp_t *disp = lv_disp_drv_register(&disp_drv);
	if (screenRotate == 1)
	{
		disp_drv.sw_rotate = 1;
		lv_disp_set_rotation(disp, LV_DISP_ROT_270);
	}
	else if (screenRotate == 2)
	{
		disp_drv.sw_rotate = 1;
		lv_disp_set_rotation(disp, LV_DISP_ROT_180);
	}
	else if (screenRotate == 3)
	{
		disp_drv.sw_rotate = 1;
		lv_disp_set_rotation(disp, LV_DISP_ROT_90);
	}
	// Initialize and register a pointer device driver
	static lv_indev_drv_t indev_drv;
	lv_indev_drv_init(&indev_drv);
	indev_drv.type = LV_INDEV_TYPE_POINTER;
	indev_drv.read_cb = evdev_read; // defined in lv_drivers/indev/evdev.h
	lv_indev_drv_register(&indev_drv);

	static lv_indev_drv_t indev_drv_enc;
	lv_indev_drv_init(&indev_drv_enc);
	indev_drv_enc.type = LV_INDEV_TYPE_ENCODER;
	indev_drv_enc.read_cb = encoder_read;
	encoder_indev_t = lv_indev_drv_register(&indev_drv_enc);
	button_group = lv_group_create();
	lv_indev_set_group(encoder_indev_t, button_group);
	
#endif

#if USE_WAYLAND
	//-lwayland-client -lwayland-cursor -lxkbcommon
	lv_wayland_init();
	lv_disp_t *disp = lv_wayland_create_window(screenWidth, screenHeight,"sdrberry",NULL);
#endif

	static lv_indev_drv_t indev_drv_mouse;
	lv_indev_t *indev_mouse;
	if (Mouse_dev.GetMouseAttached())
	{
		lv_indev_drv_init(&indev_drv_mouse);
		indev_drv_mouse.type = LV_INDEV_TYPE_POINTER;
		indev_drv_mouse.read_cb = mouse_read;
		indev_mouse = lv_indev_drv_register(&indev_drv_mouse);
		// Set cursor. For simplicity set a HOME symbol now.
		lv_obj_t *mouse_cursor = lv_img_create(lv_scr_act());
		lv_img_set_src(mouse_cursor, &mouse_cursor_icon);
		lv_indev_set_cursor(indev_mouse, mouse_cursor);
	}
	else
	{
		if (KeyboardDevice.MouseAttached())
		{
			lv_indev_drv_init(&indev_drv_mouse);
			indev_drv_mouse.type = LV_INDEV_TYPE_POINTER;
			indev_drv_mouse.read_cb = mouse_keyboard_read;
			indev_mouse = lv_indev_drv_register(&indev_drv_mouse);
			// Set cursor. For simplicity set a HOME symbol now.
			lv_obj_t *mouse_cursor = lv_img_create(lv_scr_act());
			lv_img_set_src(mouse_cursor, &mouse_cursor_icon);
			lv_indev_set_cursor(indev_mouse, mouse_cursor);
		}
	}

	static lv_indev_drv_t indev_drv_keyboard;
	if (KeyboardDevice.Attached())
	{
		lv_indev_drv_init(&indev_drv_keyboard); /*Basic initialization*/
		indev_drv_keyboard.type = LV_INDEV_TYPE_KEYPAD;
		indev_drv_keyboard.read_cb = keyboard_read;
		lv_indev_t *kb_indev = lv_indev_drv_register(&indev_drv_keyboard);
		keyboard_group = lv_group_create();
		lv_indev_set_group(kb_indev, keyboard_group);
	}
	
	lv_theme_t *th = lv_theme_default_init(NULL, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_CYAN), LV_THEME_DEFAULT_DARK, &lv_font_montserrat_14);
	lv_disp_set_theme(NULL, th);
	scr = lv_scr_act();

	GuiTopBar.setup_top_bar(scr);
	gui_vfo_inst.gui_vfo_init(scr, keyboard_group);

	static lv_style_t background_style;

	lv_style_init(&background_style);
	lv_style_set_radius(&background_style, 0);
	lv_style_set_bg_color(&background_style, lv_palette_main(LV_PALETTE_RED));

	lv_obj_t *obj1;
	bar_view = lv_obj_create(lv_scr_act());
	lv_obj_set_style_radius(bar_view, 0, 0);
	lv_obj_set_pos(bar_view, 0, topHeight + tunerHeight);
	lv_obj_set_size(bar_view, LV_HOR_RES - 3, barHeight);

	ft8bar_view = lv_obj_create(lv_scr_act());
	lv_obj_set_style_radius(ft8bar_view, 0, 0);
	lv_obj_set_pos(ft8bar_view, 0, topHeight);
	lv_obj_set_size(ft8bar_view, LV_HOR_RES - 3, barHeight + tunerHeight);
	lv_obj_add_flag(ft8bar_view, LV_OBJ_FLAG_HIDDEN);
	guift8bar.init(ft8bar_view, button_group, keyboard_group, mode, LV_HOR_RES - 3, barHeightft8);

	calbar_view = lv_obj_create(lv_scr_act());
	lv_obj_set_style_radius(calbar_view, 0, 0);
	lv_obj_set_pos(calbar_view, 0, topHeight + tunerHeight);
	lv_obj_set_size(calbar_view, LV_HOR_RES - 3, barHeight);
	lv_obj_add_flag(calbar_view, LV_OBJ_FLAG_HIDDEN);
	gcal.init(calbar_view, button_group, keyboard_group, LV_HOR_RES - 3, barHeight);

	tabview_mid = lv_tabview_create(lv_scr_act(), LV_DIR_BOTTOM, buttonHeight);
	lv_obj_add_event_cb(tabview_mid, tabview_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	lv_obj_set_pos(tabview_mid, 0, topHeight + tunerHeight + barHeight);
	lv_obj_set_size(tabview_mid, LV_HOR_RES - 3, tabHeight);
	
	tab["spectrum"] = (lv_tabview_add_tab(tabview_mid, "Spectrum"));
	tab["band"] = (lv_tabview_add_tab(tabview_mid, "Band"));
	tab["Preset"] = (lv_tabview_add_tab(tabview_mid, "Presets"));
	tab["rx"] = (lv_tabview_add_tab(tabview_mid, "RX"));
	//tab["keyboard"] = (lv_tabview_add_tab(tabview_mid, LV_SYMBOL_KEYBOARD));
	tab["squelch"] = (lv_tabview_add_tab(tabview_mid, "Squelch"));
	//tab["agc"] = (lv_tabview_add_tab(tabview_mid, "Agc"));
	tab["tx"] = (lv_tabview_add_tab(tabview_mid, "TX"));
	//tab["speech"] = (lv_tabview_add_tab(tabview_mid, "Speech"));
	tab["wsjtx"] = (lv_tabview_add_tab(tabview_mid, "Wsjtx"));
	//tab["FreeDV"] = (lv_tabview_add_tab(tabview_mid, "FreeDV"));
	tab["sdr"] = (lv_tabview_add_tab(tabview_mid, "Sdr"));
	tab["settings"] = (lv_tabview_add_tab(tabview_mid, LV_SYMBOL_SETTINGS));
	
	lv_obj_clear_flag(lv_tabview_get_content(tabview_mid), LV_OBJ_FLAG_SCROLL_CHAIN | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_ONE);
	tab_buttons = lv_tabview_get_tab_btns(tabview_mid);
	gsetup.init(tab["settings"], keyboard_group, LV_HOR_RES - 3, tabHeight - buttonHeight);
	SpectrumGraph.init(tab["spectrum"], 0, 0, LV_HOR_RES - 3, tabHeight - buttonHeight, ifrate);
	gft8.init(tab["wsjtx"], keyboard_group, 0, 0, LV_HOR_RES - 3, tabHeight - buttonHeight);
	guirx.init(tab["rx"], LV_HOR_RES - 3);
	guisdr.init(tab["sdr"], LV_HOR_RES - 3, tabHeight - buttonHeight);
	guipreset.init(tab["Preset"], LV_HOR_RES - 3, tabHeight - buttonHeight, button_group, keyboard_group);

	//freeDVTab.init(tab["FreeDV"], 0, 0, LV_HOR_RES - 3, tabHeight - buttonHeight);
	lv_btnmatrix_set_btn_ctrl(tab_buttons, hidetx, LV_BTNMATRIX_CTRL_DISABLED);
	guisquelch.init(tab["squelch"], lv_tabview_get_tab_btns(tabview_mid), LV_HOR_RES - 3);

	static lv_style_t style_btn;
	lv_style_init(&style_btn);
	lv_style_set_radius(&style_btn, 0);
	lv_style_set_border_width(&style_btn, 1);
	lv_style_set_border_opa(&style_btn, LV_OPA_50);
	lv_style_set_border_color(&style_btn, lv_color_black());
	lv_style_set_border_side(&style_btn, LV_BORDER_SIDE_INTERNAL);
	lv_style_set_radius(&style_btn, 0);
	lv_obj_add_style(tab_buttons, &style_btn, LV_PART_ITEMS);
	

	//keyb.init_keyboard(tab["keyboard"], LV_HOR_RES/2 - 3, screenHeight - topHeight - tunerHeight);
	float decimate = pow(2, Settings_file.get_int(default_radio, "decimate", 0));
	int rx_rate = Settings_file.get_int(default_radio, "samplerate");
	int tx_rate = Settings_file.get_int(default_radio, "samplerate_tx");
	if (tx_rate == 0)
		tx_rate = rx_rate;
	
	ifrate = rx_rate * 1000.0 / decimate;
	ifrate_tx = tx_rate * 1000 / decimate;
	printf("samperate rx %d samplerate tx %d decimation %f ifrate %f\n", rx_rate, tx_rate, decimate, ifrate);
	if (rx_rate == 0)
	{
		if (fd_lock_file > 0)
			close(fd_lock_file);
		remove(LOCK_FILE);
		std::cout << "\n samplerate not set,please update sdrberry_settings.cfg exiting gracefully...\n";
		exit(0); // Exit program
	}

	std::cout << "default sdr: " << Settings_file.find_sdr("default").c_str() << std::endl;
	SoapySDR::ModuleManager mm(false);
	SoapySDR::loadModules();
	SoapySDR::setLogLevel(SOAPY_SDR_TRACE);

	default_radio = Settings_file.find_sdr("default");
	for (auto &con : Settings_file.receivers)
	{
		std::string probe = Settings_file.find_probe((char *)con.c_str());
		if (!probe.length())
			probe = "driver=" + con;
		SdrDevices.AddDevice(con, probe);
	}
	
	i2cinput::create_i2c_input_thread();

	if (SdrDevices.MakeDevice(default_radio))
	{
		guift8bar.SetTxButtons();
		if (SdrDevices.get_tx_channels(default_radio) < 1) // for now assume only 1 tx channel
			default_tx_channel = -1;
		else
		{
			default_tx_channel = 0;
		}
		Gui_tx.gui_tx_init(tab["tx"], LV_HOR_RES - 3, default_tx_channel < 0);
		gbar.init(bar_view, button_group, mode, LV_HOR_RES - 3, barHeight);
		SoapySDR::Range r;
		if (SdrDevices.get_rx_channels(default_radio) < 1)
		{
			r = SdrDevices.get_full_tx_frequency_range(default_radio, default_tx_channel);
			default_rx_channel = -1;
		}
		else
		{
			r = SdrDevices.get_full_frequency_range(default_radio, default_rx_channel);
		}

		char str[80];

		sprintf(str, "%0.2f", r.minimum() / 1.0e6);
		std::string	start_freq(str);
		sprintf(str, "%0.2f", r.maximum() / 1.0e6);
		std::string stop_freq(str);
		std::string s = std::string(default_radio.c_str()) + " " + start_freq + " Mhz - " + stop_freq + " Mhz";
		GuiTopBar.set_label_status(s.c_str());
		//if (SdrDevices.get_tx_channels(default_radio) < 1) // for now assume only 1 tx channel
		//	default_tx_channel = -1;
		//else
		//	default_tx_channel = 0;
		vfo.set_vfo_range(r.minimum(), r.maximum());
		vfo.vfo_init((long)ifrate, defaultAudioSampleRate, guisdr.get_span(), &SdrDevices, default_radio, default_rx_channel, default_tx_channel);
		vfo.set_step(gbar.get_step_value(), 0);
		try
		{
			if (SdrDevices.SdrDevices[default_radio]->get_txchannels() > 0)
			{
				lv_btnmatrix_clear_btn_ctrl(tab_buttons, hidetx, LV_BTNMATRIX_CTRL_DISABLED);
				Gui_tx.set_drv_range();
				for (auto &col : SdrDevices.SdrDevices.at(default_radio)->get_tx_sample_rates(default_tx_channel))
				{
					int v = (int)col;
					guisdr.add_tx_sample_rate(v);
				}
				guisdr.set_tx_sample_rate((int)tx_rate * 1000);
			}
		}
		catch (const std::exception &e)
		{
			std::cout << e.what();
		}

		guisdr.set_radio(default_radio);
		try
		{
			if (default_rx_channel >= 0)
			{
				for (auto &col : SdrDevices.SdrDevices.at(default_radio)->get_rx_sample_rates(default_rx_channel))
				{
					int v = (int)col;
					guisdr.add_sample_rate(v);
				}
			}
			else
			{
				if (default_tx_channel >= 0)
				{
					for (auto &col : SdrDevices.SdrDevices.at(default_radio)->get_tx_sample_rates(default_tx_channel))
					{
						int v = (int)col;
						guisdr.add_sample_rate(v);
					}
				}
			}
		}
		catch (const std::exception &e)
		{
			std::cout << e.what();
		}
		guisdr.set_sample_rate(ifrate * decimate);
		try
		{
			SdrDevices.SdrDevices.at(default_radio)->setSampleRate(SOAPY_SDR_RX, 0, ifrate * decimate);
		}
		catch (const std::exception &e)
		{
			std::cout << e.what();
		}
		gui_band_instance.init_button_gui(tab["band"], keyboard_group, LV_HOR_RES - 3, tabHeight - buttonHeight, SdrDevices.get_full_frequency_range_list(default_radio, max(default_rx_channel, default_tx_channel)));
		gbar.set_vol_slider(Settings_file.volume());
		catinterface->SetAG(Settings_file.volume());
		gbar.set_gain_range();
		gbar.set_gain_slider_band_from_config();
		vfo.set_vfo(0LL, vfo_activevfo::One);
		try
		{
			if (SdrDevices.SdrDevices.at(default_radio)->get_bandwith_count(0))
			{
				long bw = SdrDevices.SdrDevices[default_radio]->get_bandwith(0, 0);
				SdrDevices.SdrDevices[default_radio]->setBandwidth(SOAPY_SDR_RX, 0, bw);
				printf("setBandwidth %ld \n", bw);
			}
		}
		catch (const std::exception &e)
		{
			std::cout << e.what();
		}
		guisdr.set_span_range(ifrate);
		int span = Settings_file.get_int(default_radio, "span", (int)ifrate / 1000) * 1000;
		guisdr.set_span_value(span);
		guisdr.init_bandwidth();
		guisdr.init_antenna();
		guisdr.init_settings();
		guisquelch.set_sdr_state();
		gbar.set_mode(mode);
		guigain.reset_gains();
		select_mode(mode); // start streaming
	}
	else
	{
		SoapySDR::Range r1(35000000, 40000000);
		SoapySDR::RangeList RangeList;
		RangeList.push_back(r1);
		Gui_tx.gui_tx_init(tab["tx"], LV_HOR_RES - 3, true);
		gui_band_instance.init_button_gui(tab["band"], keyboard_group, LV_HOR_RES - 3, tabHeight - buttonHeight, RangeList);
		gbar.init(bar_view, button_group, mode, LV_HOR_RES - 3, barHeight);
		GuiTopBar.set_label_status("No SDR Device Found");
		guisdr.set_radio(default_radio);
	}

	lv_group_add_obj(button_group, lv_tabview_get_tab_btns(tabview_mid));
	/*Handle LitlevGL tasks (tickless mode)*/
	auto timeLastStatus = std::chrono::high_resolution_clock::now();
	int wsjtxWaterfallGain = Settings_file.get_int("wsjtx", "waterfallgain", 20);

	if (Settings_file.get_int("web", "enabled", 0))
		webserver.StartServer();
	int refreshSpeed = Settings_file.get_int("Radio", "refresh", 50);

	while (1)
	{
		WsjtxMessage msg;

		gui_mutex.lock();
		lv_task_handler();
		gui_mutex.unlock();
		
		//Mouse_dev.step_vfo();
		//HidDev_dev.step_vfo();
		shuttle.step_vfo();
		HidDev_dev1.step_vfo();
		HidDev_dev2.step_vfo();
		if (mode == mode_ft8 || mode == mode_ft4 || mode == mode_wspr)
			DigitalTransmission::WaitForTimeSlot();
		const auto now = std::chrono::high_resolution_clock::now();
		if (timeLastStatus + std::chrono::milliseconds(refreshSpeed) < now)
		{
			timeLastStatus = now;
			double s = SpectrumGraph.get_signal_strength();
			gui_vfo_inst.set_s_meter(s);
			catinterface->SetSM((uint8_t)s);
			if (mode == mode_freedv)
				freeDVTab.DrawWaterfall();
			if (mode == mode_ft8 || mode == mode_ft4 || mode == mode_wspr)
				guift8bar.DrawWaterfall(guirx.get_waterfallgain() + (float)wsjtxWaterfallGain);
			SpectrumGraph.DrawDisplay();
			if (gsetup.get_calibration())
			{
				gcal.SetErrorCorrelation(errorMeasurement,correlationMeasurement);
			}
			Gui_tx.get_measurements();
		}

		if (!IsDigtalMode(mode) && i2cinput::connected() && SdrDevices.get_tx_channels(default_radio) > 0 && audio_input->isStreamOpen())
		{
			if (i2cinput::get_pin(0) == 0)
			{
				if (!mode_state_rxtx && default_tx_channel >= 0)
				{
					select_mode_tx(mode, audioTone::NoTone, 0);
					mode_state_rxtx = 1;
				}
			}
			else
			{
				if (mode_state_rxtx)
				{
					select_mode(mode);
					mode_state_rxtx = 0;
				}
			}
		}
		
		while (wsjtx->pullMessage(msg))
		{
			gft8.add_line(msg.hh,
						  msg.min,
						  msg.sec,
						  msg.snr,
						  0,
						  msg.dt,
						  msg.freq,
						  msg.msg.c_str());
		}
		GuiTopBar.set_time_label();
		while (guiQueue.size() > 0)
		{
			GuiMessage msg = guiQueue.front();
			switch (msg.message)
			{
			case GuiMessage::setvfo_a: {
				gbar.set_vfo(vfo_activevfo::One);
				process_vfo_message(msg);
				break;
			}
			case GuiMessage::setvfo_b: {
				gbar.set_vfo(vfo_activevfo::Two);
				process_vfo_message(msg);
				break;
			}
				
			case GuiMessage::setvfo: {
				process_vfo_message(msg);
				break;
			}
			
			case GuiMessage::change_step:
				gbar.change_step(-1);
				break;
			
			case GuiMessage::step:
				vfo.step_vfo(msg.data);
				break;
			
			case GuiMessage::blink:
				if (msg.data)
					gbar.setIfGainOverflow(true);
				else
					gbar.setIfGainOverflow(false);
				break;
				
			case GuiMessage::setpos:
				SpectrumGraph.set_pos(vfo.get_vfo_offset());
				break;
			
			case GuiMessage::setmode_vfo_a:
				if (!IsDigtalMode(mode) && mode != msg.data)
				{
					gbar.set_vfo(vfo_activevfo::One);
					gbar.set_mode(msg.data);
					select_mode(msg.data);
				}
				break;
				
			case GuiMessage::setmode_vfo_b:
				if (!IsDigtalMode(mode) && mode != msg.data)
				{
					gbar.set_vfo(vfo_activevfo::Two);
					gbar.set_mode(msg.data);
					select_mode(msg.data);
				}
				break;

			case GuiMessage::rit_onoff:
				if (!IsDigtalMode(mode))
				{
					if (!msg.data)
					{
						vfo.setRit(0, vfo.get_active_vfo());
						gbar.set_rit_button(false, 0);
					}
				}
				break;

			case GuiMessage::rit_delta:
				if (!IsDigtalMode(mode))
				{
					vfo.setRit(msg.data, vfo.get_active_vfo());
					if (msg.data == 0)
						gbar.set_rit_button(false, msg.data);
					else
						gbar.set_rit_button(true, msg.data);
				}
				break;

			case GuiMessage::setband:
				{
					gui_band_instance.set_gui(msg.data);
					int index = getIndex(Settings_file.meters, msg.data);
					if (index >= 0)
					{
						long f_low = Settings_file.f_low.at(index);
						int f_band = Settings_file.meters.at(index);
						vfo.set_band(f_band, f_low);
						gbar.set_mode(mode);
					}
				}
				break;
			case GuiMessage::setifgain:
				gbar.set_if(msg.data);
				break;
			case GuiMessage::setvol:
				gbar.set_vol_slider(msg.data);
				break;
			case GuiMessage::setgain:
				gbar.set_gain_slider(msg.data);
				break;
			case GuiMessage::setled:
				gbar.set_led(msg.data);
				break;
			case GuiMessage::setwpm:
				gbar.set_cw_wpm(msg.data);
				break;
			case GuiMessage::displayline:
				gbar.set_cw_message(msg.text);
				break;
			case GuiMessage::filter:
				gbar.set_filter_number(msg.data);
				break;
			case GuiMessage::clearWsjtx:
				break;
			case GuiMessage::scrollWsjtx:
				gft8.tableScrollLastItem();
				break;
			case GuiMessage::wsjtxMessage: {
					json message = json::parse(msg.text);
					//printf("%s\n", message.dump().c_str());
					if (message.at("type") == "wsjtxbar")
					{
						if (message.at("button") == "Monitor")
							guift8bar.MonitorButton();
						if (message.at("button") == "CQ")
							guift8bar.CQButton();
						if (message.at("button") == "TX")
							guift8bar.TXButton();
						if (message.at("button") == "Clear")
							guift8bar.ClearButton();
						if (message.at("button") == "Log")
							guift8bar.LogButton();
						if (message.at("button") == "frequency")
						{
							guift8bar.set_frequency(message);
						}
						if (message.at("type") == "spectrumbar")
						{
							
						}
						guift8bar.get_buttons();
					}
					if (IsDigtalMode(mode))
					{
						try
						{
							if (message.at("type") == "selecttxmessage")
							{
								guift8bar.MessageNo(message.at("no"));
							}
							if (message.at("type") == "selectmessage")
							{
								gft8.SelectMessage(message.at("data"));
							}
						}
						catch (const exception &e)
						{
							std::string err = e.what();
							printf("%s\n", err.c_str());
						}
					}
				}
				break;

			case GuiMessage::TranceiverMessage: {
					//printf("%s\n", msg.text.c_str());
					json message;
					try
					{
						message = json::parse(msg.text);
					}
					catch (const exception &e)
					{
						std::string err = e.what();
						printf("%s\n", err.c_str());
						break;
					}
					
					//printf("%s\n", message.dump().c_str());
					if (message.find("setfilter") != message.end())
					{
						gbar.websetfilter(message.at("setfilter"));
					}
					if (message.find("setvolume") != message.end())
					{
						if (message.at("setvolume").is_number())
							gbar.set_vol_slider(message.at("setvolume"), false);
					}
					if (message.find("setrfvalue") != message.end())
					{
						if (message.at("setrfvalue").is_number())
						{
							int max_gain, min_gain;

							gbar.get_gain_range(max_gain, min_gain);
							gbar.set_gain_slider((int)message.at("setrfvalue") + min_gain, false);
						}
					}
					if (message.find("setifvalue") != message.end())
					{
						if (message.at("setifvalue").is_number())
							gbar.set_if(message.at("setifvalue"), false);
					}
					if (message.find("tx") != message.end())
					{
						// do volume
					}
					if (message.find("tune") != message.end())
					{
						// do volume
					}
					if (message.find("mode") != message.end())
					{
						// do volume
					}
					if (message.find("band") != message.end())
					{
						// do volume
					}

				}
				break;
			}
			guiQueue.pop_front();
		}
		usleep(500);
		fflush(stdout);
	}
	audio_output->close();
	delete audio_output;
	audio_input->close();
	delete audio_input;
	free(display_buf);
	if (midicontrole)
		delete midicontrole;
	return 0;
}

/*Set in lv_conf.h as `LV_TICK_CUSTOM_SYS_TIME_EXPR`*/

/*uint32_t custom_tick_get(void)
{
	static uint64_t start_ms = 0;
	if (start_ms == 0)
	{
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
*/

uint32_t custom_tick_get(void)
{
	struct timespec period;
	clock_gettime(CLOCK_MONOTONIC, &period);
	uint64_t ticks_ms = period.tv_sec * 1000 + period.tv_nsec / 1000000;

	static uint64_t start_ms = 0;
	if (start_ms == 0)
	{
		start_ms = ticks_ms;
	}

	uint32_t time_ms = ticks_ms - start_ms;
	return time_ms;
}

static bool stream_rx_on{false};

void destroy_demodulators(bool all, bool close_stream)
{
	FMDemodulator::destroy_demodulator();
	AMDemodulator::destroy_demodulator();
	AMModulator::destroy_modulator();
	FMModulator::destroy_modulator();
	FT8Demodulator::destroy_demodulator();
	EchoAudio::destroy_modulator();
	FMBroadBandDemodulator::destroy_demodulator();
	if (all)
	{
		RX_Stream::destroy_rx_streaming_thread();
		stream_rx_on = false;
		RX_Stream::pause_rx_stream(false);
	}
	TX_Stream::destroy_tx_streaming_thread(close_stream);
}

void update_filter(int bandwidth)
{
	FMDemodulator::setLowPassAudioFilterCutOffFrequency(bandwidth);
	AMDemodulator::setLowPassAudioFilterCutOffFrequency(bandwidth);
	AMModulator::setLowPassAudioFilterCutOffFrequency(bandwidth);
	FT8Demodulator::setLowPassAudioFilterCutOffFrequency(bandwidth);
}

extern std::chrono::high_resolution_clock::time_point starttime1;

bool IsDigtalMode(int mode)
{
	if (mode == mode_ft8)
		return true;
	if (mode == mode_ft4)
		return true;
	if (mode == mode_wspr)
		return true;
	return false;
}

void set_tx_buttons()
{
	guift8bar.SetTxButtons();
	gbar.setTxButtons();

	if (SdrDevices.get_tx_channels(default_radio) == 0 || !audio_input->isStreamOpen())
	{
		lv_btnmatrix_set_btn_ctrl(tab_buttons, hidetx, LV_BTNMATRIX_CTRL_DISABLED);
		Gui_tx.enable_tx(false);
	}
	else
	{
		lv_btnmatrix_clear_btn_ctrl(tab_buttons, hidetx, LV_BTNMATRIX_CTRL_DISABLED);
		Gui_tx.enable_tx(true);	
	}
}

void select_mode(int s_mode, bool bvfo, int channel)
{
	bool stereo{false}, dc{false};
	std::vector<long> ftx_freq;

	if (!SdrDevices.isValid(default_radio))
		return;
	set_tx_buttons();
	catinterface->SetTX(TX_OFF);
	catinterface.Pause_Cat(true);
	catinterface.MuteFA(false);
	i2c_output.set_rxtx(false);
	vfo.pause_step(false);
	// wait for threads to finish
	printf("select_mode_rx stop all threads\n");
	// stop transmit
	destroy_demodulators();
	mode = s_mode;
	if (SdrDevices.get_tx_channels(default_radio) > 0)
		Gui_tx.set_tx_state(false);
	if (SdrDevices.get_rx_channels(default_radio) < 1)
	{
		catinterface.Pause_Cat(false);
		return;
	}
	vfo.vfo_rxtx(true, false);
	if (bvfo)
	{
		vfo.set_mode(vfo.get_active_vfo(), mode);
		vfo.set_vfo(0);
	}
	printf("select_mode_rx start rx threads\n");
	switch (mode)
	{
	case mode_narrowband_fm:
		guift8bar.setmonitor(false);
		gbar.set_filter_dropdown(4000);
		FMDemodulator::create_demodulator(ifrate, &source_buffer_rx, audio_output);
		if (!stream_rx_on)
		{
			RX_Stream::create_rx_streaming_thread(ifrate, default_radio, channel, &source_buffer_rx, guisdr.get_decimation());
			stream_rx_on = true;
		}
		else
			RX_Stream::pause_rx_stream(false);
		break;

	case mode_broadband_fm:
		guift8bar.setmonitor(false);
		if (audio_output->get_channels() > 1)
			stereo = true;
		else
			stereo = false;
		FMBroadBandDemodulator::create_demodulator(ifrate, &source_buffer_rx, audio_output, stereo);
		if (!stream_rx_on)
		{
			RX_Stream::create_rx_streaming_thread(ifrate, default_radio, channel, &source_buffer_rx, guisdr.get_decimation());
			stream_rx_on = true;
		}
		else
			RX_Stream::pause_rx_stream(false);
		break;
		
	case mode_cw:
		guirx.set_cw(true);
	case mode_am:
	case mode_dsb:
	case mode_usb:
	case mode_lsb:
	case mode_freedv:
		if (mode != mode_cw)
			guirx.set_cw(false);
		guift8bar.setmonitor(false);
		vfo.set_step(gbar.get_step_value(), 0);
		printf("Start AMDemodulator\n");
		AMDemodulator::create_demodulator(mode, ifrate, &source_buffer_rx, audio_output);
		if (!stream_rx_on)
		{
			RX_Stream::create_rx_streaming_thread(ifrate, default_radio, channel, &source_buffer_rx, guisdr.get_decimation());
			stream_rx_on = true;
		}
		else
			RX_Stream::pause_rx_stream(false);
		break;
	case mode_ft8:
	case mode_ft4:
	case mode_wspr:
		catinterface.MuteFA(true);
		vfo.pause_step(true);
		guift8bar.setmonitor(true);
		vfo.set_step(gbar.get_step_value(), 0);
		FT8Demodulator::create_demodulator(ifrate, &source_buffer_rx, audio_output, mode);
		RX_Stream::create_rx_streaming_thread(ifrate, default_radio, channel, &source_buffer_rx, guisdr.get_decimation());
		break;
	//case mode_am:
	case mode_echo:
		EchoAudio::create_modulator(audio_output,audio_input);
		break;
	}
	catinterface.Pause_Cat(false);
}

bool select_mode_tx(int s_mode, audioTone tone, int cattx, int channel)
{
	ModulatorParameters param{};
	
	// Stop all threads
	if (!SdrDevices.isValid(default_radio))
		return false;
	if (mode == mode_ft8 || mode == mode_ft4 || mode == mode_wspr)
		return false;
	if (SdrDevices.get_tx_channels(default_radio) == 0 || !audio_input->isStreamOpen())
		return false;
	catinterface->SetTX(cattx);
	catinterface.Pause_Cat(true);
	catinterface.MuteFA(false);
	i2c_output.set_rxtx(true);
	vfo.pause_step(false);
	startTime = std::chrono::high_resolution_clock::now();
	auto now = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> timePassed = now - startTime;
	printf("select_mode_tx stop all threads time %4.2f\n", (double)timePassed.count() * 1000000.0);
	int halfduplex = Settings_file.get_int(default_radio, "halfduplex", 0);
	if (halfduplex)
	{
		destroy_demodulators(true, false);
	}
	else
	{
		destroy_demodulators(false, false);
	}
	RX_Stream::pause_rx_stream(true);
	mode = s_mode;
	Gui_tx.set_tx_state(true); // set tx button
	vfo.vfo_rxtx(false, true, gui_vfo_inst.get_split());
	vfo.set_vfo(0, vfo_activevfo::None);
	int tx_rate = Settings_file.get_int(default_radio, "samplerate_tx");
	float decimate = (int)pow(2.0, (float)Settings_file.get_int(default_radio, "decimate"));
	ifrate_tx = tx_rate * 1000 / decimate;

	bool restart = (bool)Settings_file.get_int(default_radio, "restart_tx", 1);
	printf("select_mode_tx start tx threads samplerate %f10.0 decimation %f2.0 restart %d\n", ifrate_tx, decimate, restart);
	switch (mode)
	{
	case mode_broadband_fm:
		//start_fm_tx(ifrate, audio_output->get_samplerate(), true, &source_buffer_tx, audio_output);
		//mode_running = 1;
		break;

	case mode_narrowband_fm:
		FMModulator::create_modulator(mode, ifrate_tx, tone, &source_buffer_tx, audio_input);
		TX_Stream::create_tx_streaming_thread(ifrate, default_radio, channel, &source_buffer_tx, ifrate_tx, guisdr.get_decimation(), restart);
		break;

	case mode_cw:
	case mode_am:
	case mode_dsb:
	case mode_usb:
	case mode_lsb:
	case mode_freedv:
		param.mode = mode;
		param.tone = tone;
		param.ifrate = ifrate_tx;
		AMModulator::create_modulator(param, &source_buffer_tx, audio_input);
		TX_Stream::create_tx_streaming_thread(ifrate, default_radio, channel, &source_buffer_tx, ifrate_tx, guisdr.get_decimation(), restart);
		break;
	
	case mode_echo:
		EchoAudio::create_modulator(audio_output, audio_input);
		break;
	
	default:
		break;
	}
	catinterface.Pause_Cat(false);
	return true;
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

void switch_sdrreceiver(std::string receiver)
{
	SoapySDR::Range r;
	default_rx_channel = 0;
	default_tx_channel = 0;
	
	/// First switchoff current receiver
	destroy_demodulators(true, true);
	SdrDevices.UnMakeDevice(default_radio);
	default_radio = receiver;
	Settings_file.save_string("SDR Receivers", "default", default_radio);
	Settings_file.write_settings();
	
	// Hide TX page
	lv_btnmatrix_set_btn_ctrl(tab_buttons, hidetx, LV_BTNMATRIX_CTRL_DISABLED);
	if (SdrDevices.MakeDevice(default_radio))
	{
		float decimate = pow(2, Settings_file.get_int(default_radio, "decimate", 0));
		int rx_rate = Settings_file.get_int(default_radio, "samplerate");
		int tx_rate = Settings_file.get_int(default_radio, "samplerate_tx");
		if (tx_rate == 0)
			tx_rate = rx_rate;

		ifrate = rx_rate * 1000.0 / decimate;
		ifrate_tx = tx_rate * 1000 / decimate;
		printf("samperate rx %d samplerate tx %d decimation %f ifrate %f\n", rx_rate, tx_rate, decimate, ifrate);
		// set top line with receiver information
		if (SdrDevices.get_rx_channels(default_radio) < 1)
		{
			r = SdrDevices.get_full_tx_frequency_range(default_radio, default_tx_channel);
			default_rx_channel = -1;
		}
		else
		{
			r = SdrDevices.get_full_frequency_range(default_radio, default_rx_channel);
		}
		std::string start_freq = std::to_string(r.minimum() / 1.0e6);
		std::string stop_freq = std::to_string(r.maximum() / 1.0e6);
		std::string s = std::string(default_radio.c_str()) + " " + start_freq + " Mhz - " + stop_freq + " Mhz";
		GuiTopBar.set_label_status(s.c_str());
		if (SdrDevices.get_tx_channels(default_radio) < 1) // for now assume only 1 tx channel
			default_tx_channel = -1;
		else
			default_tx_channel = 0;
		Gui_tx.enable_tx(default_tx_channel > 0);
		guift8bar.SetTxButtons();
		vfo.set_vfo_range(r.minimum(), r.maximum());
		vfo.vfo_init((long)ifrate, audio_output->get_samplerate(), guisdr.get_span(), &SdrDevices, default_radio, default_rx_channel, default_tx_channel);
		guisdr.clear_sample_rate();
		try
		{
			if (SdrDevices.SdrDevices[default_radio]->get_txchannels() > 0)
			{
				lv_btnmatrix_clear_btn_ctrl(tab_buttons, hidetx, LV_BTNMATRIX_CTRL_DISABLED);
				guisdr.clear_sample_rate();
				Gui_tx.set_drv_range();
				Gui_tx.set_mic_slider(Settings_file.get_int("Radio", "micgain", 85));
				for (auto &col : SdrDevices.SdrDevices.at(default_radio)->get_tx_sample_rates(default_tx_channel))
				{
					int v = (int)col;
					guisdr.add_tx_sample_rate(v);
				}
				guisdr.set_tx_sample_rate((int)tx_rate * 1000);
			}
		}
		catch (const std::exception &e)
		{
			std::cout << e.what();
		}
		// Rx sample rates
		try
		{
			if (default_rx_channel >= 0)
			{
				for (auto &col : SdrDevices.SdrDevices.at(default_radio)->get_rx_sample_rates(default_rx_channel))
				{
					int v = (int)col;
					guisdr.add_sample_rate(v);
				}
			}
			else
			{
				if (default_tx_channel >= 0)
				{
					for (auto &col : SdrDevices.SdrDevices.at(default_radio)->get_tx_sample_rates(default_tx_channel))
					{
						int v = (int)col;
						guisdr.add_sample_rate(v);
					}
				}
			}
		}
		catch (const std::exception &e)
		{
			std::cout << e.what();
		}

		guisdr.set_sample_rate((int)ifrate * decimate);
		try
		{
			SdrDevices.SdrDevices.at(default_radio)->setSampleRate(SOAPY_SDR_RX, 0, ifrate * decimate);
		}
		catch (const std::exception &e)
		{
			std::cout << e.what();
		}
		gui_band_instance.init_button_gui(nullptr, keyboard_group, LV_HOR_RES - 3, tabHeight - buttonHeight, SdrDevices.get_full_frequency_range_list(default_radio, max(default_rx_channel, default_tx_channel)));
		gbar.set_vol_slider(Settings_file.volume());
		catinterface->SetAG(Settings_file.volume());
		gbar.set_gain_range();
		gbar.set_gain_slider_band_from_config();
		guift8bar.SetTxButtons();
		gbar.setTxButtons();
		guigain.reset_gains();
		//vfo.set_vfo(freq, false);
		if (SdrDevices.SdrDevices[default_radio]->get_bandwith_count(0))
		{
			long bw = SdrDevices.SdrDevices[default_radio]->get_bandwith(0, 0);
			bw = Settings_file.get_int(default_radio, "bandwidth");
			SdrDevices.SdrDevices[default_radio]->setBandwidth(SOAPY_SDR_RX, 0, bw);
			vfo.vfo_re_init(ifrate, defaultAudioSampleRate, guisdr.get_span(), bw);
			printf("setBandwidth %ld \n", bw);
		}
		guisdr.init_bandwidth();
		guisdr.init_antenna();
		guisquelch.set_sdr_state();
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

void my_timer(lv_timer_t *timer)
{
	/*Use the user_data*/
	/*Do something with LVGL*/
	lv_tabview_set_act(tabview_mid, 0, LV_ANIM_ON);
	lv_timer_del(timer);
	timer = NULL;
}

void create_spectrum_page_time()
{
	int time = Settings_file.get_int("Radio", "gui_timer", 0);
	if (time)
		lv_timer_t *timer = lv_timer_create(my_timer, time, NULL);
}

