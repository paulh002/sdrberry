#include "gui_recorder.h"
#include "player.h"
#include "recorder.h"
#include "AudioInput.h"
#include "AudioOutput.h"
#include "sdrstream.h"
#include <iostream>
#include <filesystem>
#include "Spectrum.h"
#include "audiobar.h"


gui_recorder grecorder;

namespace fs = std::filesystem;
bool finalizeWav(const std::string &output_file, const std::string &target_file)
{
	std::error_code ec;

	// 1. Check if output.wav exists
	if (!fs::exists(output_file, ec))
	{
		return true; // File doesn't exist -> do nothing, success
	}

	// 2. Delete m1.wav if it exists
	if (fs::exists(target_file, ec))
	{
		fs::remove(target_file, ec);
		if (ec)
		{
			std::cerr << "Failed to remove " << target_file << ": " << ec.message() << "\n";
			return false;
		}
	}

	// 3. Move output.wav -> m1.wav
	fs::rename(output_file, target_file, ec);
	if (ec)
	{
		std::cerr << "Failed to move " << output_file << " to " << target_file << ": " << ec.message() << "\n";
		return false;
	}

	return true;
}

void gui_recorder::button_handler_class(lv_event_t *e)
{
	lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
	lv_obj_t *label = lv_obj_get_child(obj, 0L);
	char *ptr = lv_label_get_text(label);
	std::string s(ptr);
	
	if (s == "Rec")
	{
		if (lv_obj_get_state(obj) & LV_STATE_CHECKED)
		{
			destroy_demodulators(true, false);
			recorder::create_recorder(audio_output, audio_input);
		}
		else
		{
			recorder::destroy_recorder();
			select_mode(mode);
		}
	}
	if (s == "M1")
	{
		finalizeWav(std::string("output.wav"), std::string("m1.wav"));
		destroy_demodulators(true, false);
		player::create_player(audio_output, audio_input, std::string("m1.wav"));
		player::destroy_player();
		select_mode(mode);
	}
	if (s == "M2")
	{
		finalizeWav(std::string("output.wav"), std::string("m2.wav"));
		destroy_demodulators(true, false);
		player::create_player(audio_output, audio_input, std::string("m2.wav"));
		player::destroy_player();
		select_mode(mode);
	}
	if (s == "M3")
	{
		finalizeWav(std::string("output.wav"), std::string("m3.wav"));
		destroy_demodulators(true, false);
		player::create_player(audio_output, audio_input, std::string("m3.wav"));
		player::destroy_player();
		select_mode(mode);
	}
	if (s == "M4")
	{
		finalizeWav(std::string("output.wav"), std::string("m4.wav"));
		destroy_demodulators(true, false);
		player::create_player(audio_output, audio_input, std::string("m4.wav"));
		player::destroy_player();
		select_mode(mode);
	}
}

void gui_recorder::init(lv_obj_t *parent, lv_coord_t w)
{
	// To do: add buttons for recording, show recording time, show file name, show free space on sd card
	const lv_coord_t x_margin  = 10;
	const lv_coord_t y_margin  = 10;
	const int x_number_buttons = 6;
	const int y_number_buttons = 4;
	const lv_coord_t tab_margin  = 20;
	const int number_of_buttons{5};
	
	int button_width_margin = ((w - tab_margin) / x_number_buttons);
	int button_width = ((w - tab_margin) / x_number_buttons) - x_margin;

	int button_height = 40;
	int button_height_margin = button_height + y_margin;
	int ibuttons;
	int ibutton_x = 0, ibutton_y = 1;
	
	lv_obj_set_style_pad_top(parent, 10, LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(parent, 5, LV_PART_MAIN);
	lv_obj_set_style_pad_right(parent, 5, LV_PART_MAIN);
	lv_obj_set_style_pad_left(parent, 5, LV_PART_MAIN);

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
	lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);

	button_group = lv_group_create();
	ibuttons = number_of_buttons;
	for (int i = 0; i < ibuttons; i++)
	{
		char str[80];

		buttons[i] = lv_btn_create(parent);
		lv_group_add_obj(button_group, buttons[i]);
		lv_obj_add_style(buttons[i], &style_btn, 0);
		lv_obj_add_event_cb(buttons[i], button_handler, LV_EVENT_CLICKED, (void *)this);
		lv_obj_align(buttons[i], LV_ALIGN_TOP_LEFT, ibutton_x * button_width_margin, ibutton_y * button_height_margin);
		// lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
		lv_obj_set_size(buttons[i], button_width, button_height);

		lv_obj_t *lv_label = lv_label_create(buttons[i]);
		switch (i)
		{
		case 0:
			strcpy(str, "M1");
			break;
		case 1:
			strcpy(str, "M2");
			break;
		case 2:
			strcpy(str, "M3");
			break;
		case 3:
			strcpy(str, "M4");
			break;
		case 4:
			strcpy(str, "Rec");
			lv_obj_add_flag(buttons[i], LV_OBJ_FLAG_CHECKABLE);
			break;
		}
		lv_label_set_text(lv_label, str);
		lv_obj_center(lv_label);

		ibutton_x++;
		if (ibutton_x >= x_number_buttons)
		{
			ibutton_x = 0;
			ibutton_y++;
		}
	}

	lv_obj_t *mic_label = lv_label_create(parent);
	lv_label_set_text(mic_label, "Mic level: ");
	lv_obj_align(mic_label, LV_ALIGN_TOP_LEFT, x_margin,0);
	lv_obj_invalidate(parent);
	audiobar_recording.init(parent, 7 * (LV_HOR_RES / 8), 30);
	audiobar_recording.align(parent, LV_ALIGN_TOP_LEFT, 100, 0);
	audiobar_recording.SetRange(300);
	smeter_filter = std::make_unique<SMeterFilter>(20.0f, 25.0f, 280.0f); // SSB defaults

	lv_obj_t *checkbox_txbuttons = lv_checkbox_create(parent);
	lv_group_add_obj(button_group, checkbox_txbuttons);
	lv_checkbox_set_text(checkbox_txbuttons, "Show TX Buttons");
	lv_obj_add_event_cb(checkbox_txbuttons, event_handler_txbuttons, LV_EVENT_CLICKED, (void *)this);
	lv_obj_align(checkbox_txbuttons, LV_ALIGN_TOP_LEFT, VU_BAR_WIDTH + 100 + x_margin, 0);
	lv_group_add_obj(button_group, checkbox_txbuttons);

	if (Settings_file.get_int("Radio", "TXButtons", 0))
	{
		lv_obj_add_state(checkbox_txbuttons, LV_STATE_CHECKED);
		SpectrumGraph.hide_buttonbar(false);
	}
}

void gui_recorder::set_value(float value)
{
	float smoothed = smeter_filter->process(value + 350.0f);
	audiobar_recording.value(smoothed);
}

void gui_recorder::event_handler_txbuttons_class(lv_event_t *e)
{
	lv_obj_t *checkbox = (lv_obj_t *)lv_event_get_target(e);
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);

	if (code == LV_EVENT_CLICKED)
	{
		if (lv_obj_get_state(checkbox) & LV_STATE_CHECKED)
		{
			SpectrumGraph.hide_buttonbar(false);
			Settings_file.save_int("Radio", "TXButtons", 1);
		}
		else
		{
			SpectrumGraph.hide_buttonbar(true);
			Settings_file.save_int("Radio", "TXButtons", 0);
		}
		Settings_file.write_settings();
	}
}
