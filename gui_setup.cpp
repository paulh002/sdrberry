#include "gui_setup.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <atomic>
#include "Spectrum.h"
#include "gui_cal.h"
#include "gui_ft8bar.h"
#include "gui_bar.h"
#include "gui_i2csetup.h"
#include "gui_i2c_input.h"
#include "gui_i2c_output.h"
#include "Demodulator.h"
#include "screen.h"
#include "SecondScreen.h"
#include "WebServer.h"
#include <unistd.h>
#include <sys/reboot.h>
#include <linux/reboot.h>

gui_setup	gsetup;
extern 		void switch_sdrreceiver(std::string receiver);

static const char *cal_opts = "None\n"
							  "Manual\n"
							  "Auto\n"
							  "Both";
#include <spawn.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <iostream>
#include <cstring>

extern char **environ; // POSIX standard environment pointer

bool open_cli_and_run_script(const std::string &script_path)
{
	// Terminals that support Wayland natively and accept `-e` to run commands
	const std::vector<std::string> terminals = {"foot", "alacritty", "kitty", "lxterminal"};
	std::string term_cmd;

	// Pick the first available terminal
	for (const auto &t : terminals)
	{
		// posix_spawnp will search PATH, so we just need the name
		term_cmd = t;
		break;
	}

	if (term_cmd.empty())
	{
		std::cerr << "[SDR] No supported terminal emulator found in PATH.\n";
		return false;
	}

	// Wrap script in `sh -c` to handle non-executable .sh files safely
	// `exec` replaces the shell process, avoiding orphaned sh instances
	std::string full_cmd = "exec " + script_path;

	std::vector<char *> argv = {
		const_cast<char *>(term_cmd.c_str()),
		const_cast<char *>("-e"),
		const_cast<char *>("sh"),
		const_cast<char *>("-c"),
		const_cast<char *>(full_cmd.c_str()),
		nullptr};

	pid_t pid;
	int ret = posix_spawnp(&pid, term_cmd.c_str(), nullptr, nullptr, argv.data(), environ);
	if (ret != 0)
	{
		std::cerr << "[SDR] posix_spawnp failed: " << strerror(ret) << " (code " << ret << ")\n";
		std::cerr << "[SDR] PATH: " << (getenv("PATH") ? getenv("PATH") : "NULL") << "\n";
		std::cerr << "[SDR] WAYLAND_DISPLAY: " << (getenv("WAYLAND_DISPLAY") ? getenv("WAYLAND_DISPLAY") : "NULL") << "\n";
		std::cerr << "[SDR] XDG_RUNTIME_DIR: " << (getenv("XDG_RUNTIME_DIR") ? getenv("XDG_RUNTIME_DIR") : "NULL") << "\n";
		std::cerr << "[SDR] Failed to spawn terminal: " << strerror(ret) << "\n";
		return false;
	}

	std::cout << "[SDR] Terminal spawned (PID: " << pid << ") running: " << script_path << "\n";
	return true;
}


void gui_setup::webbox_event_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
	if (code == LV_EVENT_VALUE_CHANGED)
	{
		if (lv_obj_get_state(obj) & LV_STATE_CHECKED)
		{
			Settings_file.save_int("web", "enabled", 1);
			webserver.StartServer();
		}
		else
		{
			Settings_file.save_int("web", "enabled", 0);
			webserver.StopServer();
		}
	Settings_file.save();
	}
}
	
void gui_setup::calbox_event_cb_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
	if (code == LV_EVENT_VALUE_CHANGED)
	{
		if (lv_obj_get_state(obj) & LV_STATE_CHECKED)
		{
			gbar.hide(true);
			guift8bar.hide(true);
			gcal.hide(false);
		}
		else
		{
			Settings_file.save();
			gbar.hide(false);
			guift8bar.hide(true);
			gcal.hide(true);
		}
	}
}

void gui_setup::cal_button_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
	if (code == LV_EVENT_VALUE_CHANGED)
	{
		int i = lv_dropdown_get_selected(obj);
		Demodulator::set_autocorrection(i);
		Settings_file.save_int(default_radio, "correction", i);
		Settings_file.write_settings();
	}
}

void gui_setup::dcbox_event_cb_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
	if (code == LV_EVENT_VALUE_CHANGED)
	{
		if (lv_obj_get_state(obj) & LV_STATE_CHECKED)
		{
			Demodulator::set_dc_filter(true);
			Settings_file.save_int(default_radio, "dc", 1);
		}
		else
		{
			Demodulator::set_dc_filter(false);
			Settings_file.save_int(default_radio, "dc", 0);
		}
		Settings_file.write_settings();
	}
}

void gui_setup::brightness_slider_event_cb_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e); 

	int i = lv_slider_get_value(obj);
	if (i > 0)
	{		
		set_brightness(i);
	}
}

void gui_setup::main_display_event_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
	if (code == LV_EVENT_VALUE_CHANGED)
	{
		std::string sel_main_display;
		std::string Second_display_port = Settings_file.get_string("screen", "SecondDisplay", "None");
		
		int item = lv_dropdown_get_selected(obj);
		switch (item)
		{
		case 0:
			sel_main_display = "HDMI-A-1";
			break;
		case 1:
			sel_main_display = "HDMI-A-2";
			break;
		case 2:
			sel_main_display = "DSI-1";
			break;
		case 3:
			sel_main_display = "DSI-2";
			break;
		}
		if (Second_display_port == sel_main_display)
		{
			lv_dropdown_set_selected(second_disp, 0);
			Settings_file.save_string("screen", "SecondDisplay", "None");
			delete_second_display();
		}
		Settings_file.save_string("screen", "Display", sel_main_display);
		Settings_file.write_settings();
	}
}

void gui_setup::second_display_event_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
	if (code == LV_EVENT_VALUE_CHANGED)
	{
		std::string sel_main_display;
		std::string display_port = Settings_file.get_string("screen", "Display", "DSI-1");
		int item = lv_dropdown_get_selected(obj);
		switch (item)
		{
		case 0:
			sel_main_display = "None";
			break;
		case 1:
			sel_main_display = "HDMI-A-1";
			break;
		case 2:
			sel_main_display = "HDMI-A-2";
			break;
		case 3:
			sel_main_display = "DSI-1";
			break;
		case 4:
			sel_main_display = "DSI-2";
			break;
		}
		if (display_port != sel_main_display)
		{
			delete_second_display();
			if (secondscreen)
				secondscreen->deinit();
			Settings_file.save_string("screen", "SecondDisplay", sel_main_display);
			Settings_file.write_settings();
			if (sel_main_display != "None")
			{
				second_screen = create_display(&disp_1, sel_main_display);
				secondscreen = std::make_unique<SecondScreen>();
				secondscreen->init(second_screen, keyboard_group);
				secondscreen->init_vfo();
			}
		}
		else
		{
			lv_dropdown_set_selected(obj, 0);
			delete_second_display();
		}
	}
}

void gui_setup::audio_button_handler_class(lv_event_t * e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e); 
	if (code == LV_EVENT_VALUE_CHANGED) 
	{	char buf[80];
		
		int item = lv_dropdown_get_selected(obj);
		lv_dropdown_get_selected_str(obj,buf, sizeof(buf));
		destroy_demodulators();
		audio_input->close();
		audio_output->close();
		audio_input->open(audio_input->getAudioDevice(std::string(buf)));
		audio_output->open(audio_output->getAudioDevice(std::string(buf)));
		select_mode(mode, false);
		Settings_file.save_string("Audio", "device", std::string(buf));
		Settings_file.write_settings();
	}
}

void gui_setup::do_shutdown_button_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = (lv_obj_t *)lv_event_get_current_target(e);

	if (code == LV_EVENT_CLICKED)
	{
		// const char *ptr = lv_msgbox_get_active_button_text(obj);

		lv_obj_t *btn = lv_event_get_target_obj(e);
		lv_obj_t *label = lv_obj_get_child(btn, 0);
		const char *ptr = lv_label_get_text(label);
		if (strcmp(ptr, "Shutdown") == 0 || strcmp(ptr, "Reboot") == 0)
		{
			vfo.vfo_store_freq_mode();
			sync(); // flush filesystem buffers
			system("systemctl reboot");
		}
		else if (strcmp(ptr, "exit") == 0)
		{
			vfo.vfo_store_freq_mode();
			stop_sdrberry();
		}
		else
		{
			lv_msgbox_close(mbox1);
		}
	}
}

void gui_setup::do_update_button_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = (lv_obj_t *)lv_event_get_current_target(e);

	if (code == LV_EVENT_CLICKED)
	{
		// const char *ptr = lv_msgbox_get_active_button_text(obj);

		lv_obj_t *btn = lv_event_get_target_obj(e);
		lv_obj_t *label = lv_obj_get_child(btn, 0);
		const char *ptr = lv_label_get_text(label);
		if (strcmp(ptr, "Update") == 0)
		{
			open_cli_and_run_script("/usr/local/bin/build_sdrberry");
			lv_msgbox_close(mbox1);

			mbox1 = lv_msgbox_create(NULL);
			lv_msgbox_add_title(mbox1, "Sdrberry");
			lv_msgbox_add_text(mbox1, "Please reboot?");
			lv_msgbox_add_close_button(mbox1);
			lv_obj_t *btn;
			btn = lv_msgbox_add_footer_button(mbox1, "Reboot");
			lv_obj_add_event_cb(btn, do_shutdown_button_handler, LV_EVENT_CLICKED, (void *)this);
			btn = lv_msgbox_add_footer_button(mbox1, "exit");
			lv_obj_add_event_cb(btn, do_shutdown_button_handler, LV_EVENT_CLICKED, (void *)this);
			lv_obj_center(mbox1);
		}
		else
		{
			lv_msgbox_close(mbox1);
		}
	}
}

void gui_setup::shutdown_button_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
	if (code == LV_EVENT_CLICKED)
	{
		mbox1 = lv_msgbox_create(NULL);
		lv_msgbox_add_title(mbox1, "Sdrberry");
		lv_msgbox_add_text(mbox1, "Do you want to shutdown or exit?");
		lv_msgbox_add_close_button(mbox1);
		lv_obj_t *btn;
		btn = lv_msgbox_add_footer_button(mbox1, "Shutdown");
		lv_obj_add_event_cb(btn, do_shutdown_button_handler, LV_EVENT_CLICKED, (void *)this);
		btn = lv_msgbox_add_footer_button(mbox1, "exit");
		lv_obj_add_event_cb(btn, do_shutdown_button_handler, LV_EVENT_CLICKED, (void *)this);
		btn = lv_msgbox_add_footer_button(mbox1, "Cancel");
		lv_obj_add_event_cb(btn, do_shutdown_button_handler, LV_EVENT_CLICKED, (void *)this);
		lv_obj_center(mbox1);
	}
}

void gui_setup::update_button_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
	
	if (code == LV_EVENT_CLICKED)
	{
		mbox1 = lv_msgbox_create(NULL);
		lv_msgbox_add_title(mbox1, "Sdrberry");
		lv_msgbox_add_text(mbox1, "Update Sdrberry ?");
		lv_msgbox_add_close_button(mbox1);
		lv_obj_t *btn;
		btn = lv_msgbox_add_footer_button(mbox1, "Update");
		lv_obj_add_event_cb(btn, do_update_button_handler, LV_EVENT_CLICKED, (void *)this);
		btn = lv_msgbox_add_footer_button(mbox1, "Cancel");
		lv_obj_add_event_cb(btn, do_update_button_handler, LV_EVENT_CLICKED, (void *)this);
		lv_obj_center(mbox1);
	}
}

void gui_setup::init(lv_obj_t *o_tab, lv_group_t *keyboard_group_, lv_coord_t w, lv_coord_t h)
{

	const lv_coord_t x_page_margin = 5;
	const lv_coord_t x_margin  = 10;
	const lv_coord_t y_margin  = 5;
	const int y_number_buttons = 4;
	const lv_coord_t tab_margin  = 20;

	int x_number_buttons = 5;
	if (w > 800)
		x_number_buttons = 8;

	int button_width_margin = ((w - tab_margin) / x_number_buttons);
	int button_width = ((w - tab_margin) / x_number_buttons) - x_margin;
	int button_height = 50;
	int button_height_margin = button_height + y_margin;
	int	ibutton_x = 0, ibutton_y = 0;

	lv_point_t font_size;
	lv_txt_get_size(&font_size, "7074", LV_FONT_DEFAULT, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);

	keyboard_group = keyboard_group_;
	button_group = lv_group_create();
	lv_obj_set_style_pad_top(o_tab, 5, LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(o_tab, 5, LV_PART_MAIN);
	lv_obj_set_style_pad_right(o_tab, 5, LV_PART_MAIN);

	tileview = lv_tileview_create(o_tab);
	lv_obj_clear_flag(tileview, LV_OBJ_FLAG_SCROLL_ELASTIC);

	settings_main = lv_tileview_add_tile(tileview, 0, 0, (lv_dir_t)(LV_DIR_BOTTOM | LV_DIR_TOP));

	if (screenWidth <= small_res)
		settings_cal = lv_tileview_add_tile(tileview, 0, 2, (lv_dir_t)(LV_DIR_BOTTOM | LV_DIR_TOP));
	else
		settings_cal = settings_main;

	settings_i2c = lv_tileview_add_tile(tileview, 0, 3, (lv_dir_t)(LV_DIR_BOTTOM | LV_DIR_TOP));
	i2csetup.init(settings_i2c, w, h, button_group);

	settings_i2c_input = lv_tileview_add_tile(tileview, 0, 4, (lv_dir_t)(LV_DIR_BOTTOM | LV_DIR_TOP));
	gui_i2cinput.init(settings_i2c_input, w, h, button_group);

	settings_i2c_output = lv_tileview_add_tile(tileview, 0, 5, (lv_dir_t)(LV_DIR_BOTTOM | LV_DIR_TOP));
	gui_i2coutput.init(settings_i2c_output, w, h, button_group);

	lv_obj_set_tile_id(tileview, 0, 0, LV_ANIM_OFF);
	
	lv_style_init(&text_style);
	lv_style_set_radius(&text_style, 0);
	lv_style_set_bg_color(&text_style, lv_color_black());
	
	lv_style_init(&style_btn);
	lv_style_set_radius(&style_btn, 10);
	lv_style_set_bg_color(&style_btn, lv_color_make(0x60, 0x60, 0x60));
	lv_style_set_bg_grad_color(&style_btn, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_bg_grad_dir(&style_btn, LV_GRAD_DIR_VER);
	lv_style_set_bg_opa(&style_btn, 255);
	lv_style_set_border_color(&style_btn, lv_color_make(0x9b, 0x36, 0x36)); // lv_color_make(0x2e, 0x44, 0xb2)
	lv_style_set_border_width(&style_btn, 2);
	lv_style_set_border_opa(&style_btn, 255);
	lv_style_set_outline_color(&style_btn, lv_color_black());
	lv_style_set_outline_opa(&style_btn, 255);
	lv_obj_clear_flag(settings_main, LV_OBJ_FLAG_SCROLLABLE);
	
	int xpos = 0;

	d_audio = lv_dropdown_create(settings_main);
	lv_group_add_obj(button_group, d_audio);
	lv_obj_align(d_audio, LV_ALIGN_TOP_LEFT, x_margin, y_margin + ibutton_y * button_height_margin + button_height_margin / 2);
	lv_obj_set_width(d_audio, 2 * button_width); // 2*
	lv_dropdown_clear_options(d_audio);
	lv_obj_add_event_cb(d_audio, audio_button_handler, LV_EVENT_VALUE_CHANGED, (void *)this);
	// std::vector<std::string> devices = audio_output->getDeviceNames();
	std::vector<std::string> devices;
	audio_input->listDevices(devices);
	std::string s = Settings_file.find_audio("device");
	for (int i = 0; i < devices.size(); i++)
	{
		if (devices[i].length() > 0)
		{
			lv_dropdown_add_option(d_audio, devices[i].c_str(), LV_DROPDOWN_POS_LAST);
			if (s.length() > 0 && devices[i].find(s) != std::string::npos)
			{
				lv_dropdown_set_selected(d_audio, i);
			}
		}
	}
	
	audio_label = lv_label_create(settings_main);
	lv_label_set_text(audio_label, "Audio device");
	lv_obj_align_to(audio_label, d_audio, LV_ALIGN_OUT_TOP_LEFT, 0, -10);

	int y_cal;
	if (screenWidth <= small_res)
		y_cal = y_margin + ibutton_y * button_height_margin + button_height_margin / 2;
	else
		y_cal = y_margin + (ibutton_y + 2) * button_height_margin + button_height_margin / 2;

	int x_cal = x_margin;
	calibration_dropdown = lv_dropdown_create(settings_cal);
	lv_group_add_obj(button_group, calibration_dropdown);
	// lv_obj_align(calibration_dropdown, LV_ALIGN_TOP_LEFT, w / 2 + w / 4, y_cal);
	lv_obj_align(calibration_dropdown, LV_ALIGN_TOP_LEFT, x_cal, y_cal);
	lv_obj_set_width(calibration_dropdown, button_width); // 2*
	lv_dropdown_clear_options(calibration_dropdown);
	lv_obj_add_event_cb(calibration_dropdown, cal_button_handler, LV_EVENT_VALUE_CHANGED, (void *)this);
	lv_dropdown_set_options_static(calibration_dropdown, cal_opts);
	cal_label = lv_label_create(settings_cal);
	lv_label_set_text(cal_label, "calibration");
	lv_obj_align_to(cal_label, calibration_dropdown, LV_ALIGN_OUT_TOP_LEFT, 0, -10);
	lv_dropdown_set_selected(calibration_dropdown, Settings_file.get_int(default_radio, "correction", 0));
	Demodulator::set_autocorrection(Settings_file.get_int(default_radio, "correction", 0));

	//shutdown button
	shutdownbutton = lv_btn_create(settings_main);
	//lv_group_add_obj(m_button_group, shutdownbutton);
	lv_obj_add_style(shutdownbutton, &style_btn, 0);
	lv_obj_set_size(shutdownbutton, button_width * 0.8, button_height * 0.8);
	lv_obj_add_event_cb(shutdownbutton, shutdown_button_handler, LV_EVENT_CLICKED, (void *)this);
	x_cal = xpos + 4 * (button_width + x_margin);
	y_cal = y_margin + ibutton_y * button_height_margin + button_height_margin / 2;
	lv_obj_align(shutdownbutton, LV_ALIGN_TOP_LEFT, x_cal, y_cal);
	lv_obj_t *lv_label = lv_label_create(shutdownbutton);
	lv_label_set_text(lv_label, "Exit");
	lv_obj_center(lv_label);

	y_cal = y_margin + button_height_margin / 2;
	x_cal = w / 2 + w / 4 + w / 16;
	webbox = lv_checkbox_create(settings_cal);
	lv_obj_align_to(webbox, settings_cal, LV_ALIGN_TOP_LEFT, x_cal, y_cal);
	lv_checkbox_set_text(webbox, "web");
	lv_obj_add_event_cb(webbox, webbox_event_cb, LV_EVENT_VALUE_CHANGED, (void *)this);
	if (Settings_file.get_int("web", "enabled"))
	{
		lv_obj_add_state(webbox, LV_STATE_CHECKED);
	}

	y_cal += button_height_margin ;
	x_cal = w / 2 + w / 4 + w / 16;
	calbox = lv_checkbox_create(settings_cal);
	lv_obj_align_to(calbox, settings_cal, LV_ALIGN_TOP_LEFT, x_cal, y_cal);
	lv_checkbox_set_text(calbox, "cal");
	lv_obj_add_event_cb(calbox, calbox_event_cb, LV_EVENT_VALUE_CHANGED, (void *)this);

	dcbox = lv_checkbox_create(settings_cal);
	lv_obj_align_to(dcbox, settings_cal, LV_ALIGN_TOP_LEFT, x_cal, y_cal + 50);
	lv_checkbox_set_text(dcbox, "dc filter");
	lv_obj_add_event_cb(dcbox, dcbox_event_cb, LV_EVENT_VALUE_CHANGED, (void *)this);
	if (Settings_file.get_int(default_radio, "dc"))
	{
		lv_obj_add_state(dcbox, LV_STATE_CHECKED);
		Demodulator::set_dc_filter(true);
	}
	else
	{
		Demodulator::set_dc_filter(false);
	}

	// Main Display
	int y_disp = y_margin + ibutton_y * button_height_margin + button_height_margin / 2;
	int x_disp = xpos + 2 * (button_width + x_margin);

	main_display = lv_dropdown_create(settings_main);
	lv_dropdown_clear_options(main_display);
	lv_dropdown_set_options(main_display, "HDMI-A-1\nHDMI-A-2\nDSI-1\nDSI-2");
	lv_obj_add_event_cb(main_display, main_display_event_cb, LV_EVENT_VALUE_CHANGED, (void *)this);
	lv_obj_align(main_display, LV_ALIGN_TOP_LEFT, x_disp, y_disp);
	std::string display_port = Settings_file.get_string("screen", "Display", "DSI-1");
	int option = 0;
	if (display_port == "HDMI-A-1")
		option = 0;
	if (display_port == "HDMI-A-2")
		option = 1;
	if (display_port == "DSI-1")
		option = 2;
	if (display_port == "DSI-2")
		option = 3;
	lv_dropdown_set_selected(main_display, option);
	lv_group_add_obj(button_group, main_display);
	
	lv_obj_t *main_display_label = lv_label_create(settings_main);
	lv_label_set_text(main_display_label, "Main display");
	lv_obj_align_to(main_display_label, main_display, LV_ALIGN_OUT_TOP_LEFT, 0, -10);

	// Second Display
	x_disp = xpos + 3 * (button_width + x_margin);
	second_disp = lv_dropdown_create(settings_main);
	lv_obj_align(second_disp, LV_ALIGN_TOP_LEFT, x_disp, y_disp);
	lv_dropdown_clear_options(second_disp);
	lv_dropdown_set_options(second_disp, "None\nHDMI-A-1\nHDMI-A-2\nDSI-1");
	lv_obj_add_event_cb(second_disp, second_display_event_cb, LV_EVENT_VALUE_CHANGED, (void *)this);
	display_port = Settings_file.get_string("screen", "SecondDisplay", "None");
	option = 0;
	if (display_port == "None")
		option = 0;
	if (display_port == "HDMI-A-1")
		option = 1;
	if (display_port == "HDMI-A-2")
		option = 2;
	if (display_port == "DSI-1")
		option = 3;
	if (display_port == "DSI-1")
		option = 4;
	lv_dropdown_set_selected(second_disp, option);
	lv_group_add_obj(button_group, second_disp);

	lv_obj_t *second_display_label = lv_label_create(settings_main);
	lv_label_set_text(second_display_label, "Second Display");
	lv_obj_align_to(second_display_label, second_disp, LV_ALIGN_OUT_TOP_LEFT, 0, -10);
	
	ibutton_y++;
	
	y_cal = y_margin + ibutton_y * button_height_margin + button_height_margin / 2;
	x_cal = xpos + 4 * (button_width + x_margin);
	update_button = lv_btn_create(settings_main);
	// lv_group_add_obj(m_button_group, shutdownbutton);
	lv_obj_add_style(update_button, &style_btn, 0);
	lv_obj_set_size(update_button, button_width * 0.8, button_height * 0.8);
	lv_obj_add_event_cb(update_button, update_button_handler, LV_EVENT_CLICKED, (void *)this);
	lv_obj_align(update_button, LV_ALIGN_TOP_LEFT, x_cal, y_cal);
	lv_label = lv_label_create(update_button);
	lv_label_set_text(lv_label, "Update");
	lv_obj_center(lv_label);
	
	int hh = lv_obj_get_height(main_display);
	int y_span = ibutton_y * font_size.y + ibutton_y * y_margin + ibutton_y * button_height_margin;
	brightness_slider_label = lv_label_create(settings_main);
	lv_label_set_text(brightness_slider_label, "brightness");
	int x_pos = 0;
	lv_obj_align(brightness_slider_label, LV_ALIGN_TOP_LEFT, x_margin, y_span);

	brightness_slider = lv_slider_create(settings_main);
	lv_group_add_obj(button_group, brightness_slider);
	lv_obj_set_width(brightness_slider, w / 2 - 50 - y_margin);
	lv_obj_align_to(brightness_slider, brightness_slider_label, LV_ALIGN_BOTTOM_LEFT, x_margin, y_margin + font_size.y);
	lv_obj_add_event_cb(brightness_slider, brightness_slider_event_cb, LV_EVENT_VALUE_CHANGED, (void *)this);
	lv_slider_set_range(brightness_slider, 0, get_maxbrightness());
	lv_slider_set_value(brightness_slider, get_brightness(), LV_ANIM_ON);
	lv_group_add_obj(button_group, lv_tabview_get_tab_btns(tabview_mid));
	lv_group_add_obj(button_group, brightness_slider);
}


void gui_setup::set_group()
{
	lv_indev_set_group(encoder_indev_t, button_group);
	lv_group_focus_obj(d_audio);
}

void gui_setup::set_brightness(int brightness)
{
	std::string			f{"/sys/class/backlight/10-0045/brightness"};
	std::ofstream		myfile;
		
	myfile.open(f);
	if (!myfile.is_open())
	{
		f = "/sys/class/backlight/rpi_backlight/brightness";
		myfile.open(f);	
	}
	if (!myfile.is_open())
	{
		f = "/sys/class/backlight/11-0045/brightness";
		myfile.open(f);
	}
	std::string buf = strlib::sprintf("%d", brightness);
	myfile << buf;
}

int gui_setup::get_brightness()
{
	int brightness;
	std::string		f{"/sys/class/backlight/10-0045/actual_brightness"};
	std::string		s;
	std::ifstream	myfile;
	
	myfile.open(f);
	if (!myfile.is_open())
	{
		f = "/sys/class/backlight/rpi_backlight/actual_brightness";
		myfile.open(f);	
	}
	if (!myfile.is_open())
	{
		f = "/sys/class/backlight/11-0045/actual_brightness";
		myfile.open(f);
	}
	if (myfile.is_open())
	{
		myfile >> s;
		brightness = atoi(s.c_str());
	}
	return brightness;
}

int gui_setup::get_maxbrightness()
{
	int brightness = 50;
	std::string f{"/sys/class/backlight/10-0045/max_brightness"};
	std::string s;
	std::ifstream myfile;

	myfile.open(f);
	if (!myfile.is_open())
	{
		f = "/sys/class/backlight/rpi_backlight/max_brightness";
		myfile.open(f);
	}
	if (!myfile.is_open())
	{
		f = "/sys/class/backlight/11-0045/max_brightness";
		myfile.open(f);
	}
	if (myfile.is_open())
	{
		myfile >> s;
		brightness = atoi(s.c_str());
	}
	return brightness;
}