#include "gui_bar.h"
#include "Catinterface.h"
#include "CustomEvents.h"
#include "gui_ft8bar.h"
#include "gui_rx.h"
#include "gui_setup.h"
#include "gui_sdr.h"
#include "Spectrum.h"
#include <memory>
#include "screen.h"
#include "WebServer.h"
#include "gui_gain.h"
#include "vfo.h"

const int buttontx = 0;
const int buttontune = 1;
const int buttonmode = 2;
const int buttonvfo = 3;
const int buttonsteps = 4;
const int buttonatt = 5;
const int buttonnoise = 6;
const int buttonrit = 7;
const int buttonmarker = 8;
const int button_filter = 9;


static const char *opts = "0.5 Kc\n"
						  "1.0 Kc\n"
						  "1.5 Kc\n"
						  "2.0 Kc\n"
						  "2.5 Kc\n"
						  "3.0 Kc\n"
						  "3.5 Kc\n"
						  "4.0 Kc\n"
						  "11.0 Kc\n"
						  "16.0 Kc";

std::vector<std::string> stepsTypes{"1 Hz", "10 Hz", "50 Hz", "100 Hz", "250 Hz", "500 Hz", "1 kHz",
									"2.5 kHz", "3.125 kHz", "5 kHz", "6.25 kHz", "7.5 kHz", "8.33 kHz",
									"9 kHz", "10 kHz", "12.5 kHz", "15 kHz", "20 kHz", "25 kHz", "50 kHz", "100 kHz", "200 kHz"};
std::vector<int> stepsValues{1, 10, 50, 100, 250, 500, 1000,2500, 3125, 5000, 6250, 7500, 8330, 9000, 10000, 12500, 15000, 20000, 25000, 50000, 100000, 200000};
std::vector<std::string> FilterTypes{"0.5 Khz", "1.0 Khz", "1.5 Khz", "2.0 Khz", "2.5 Khz", "3.0 Khz", "3.5 Khz", "4.0 Khz", "11.0 Khz", "16.0 Khz"};
std::vector<int> FilterValues{500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 11000, 16000};
std::vector<std::string> ModesTypes{"USB", "LSB", "CW", "DSB", "AM", "FM", "bFM"};
std::vector<int> ModesCodes{mode_usb, mode_lsb, mode_cw, mode_dsb, mode_am, mode_narrowband_fm, mode_broadband_fm};
std::vector<std::string> preamTypes{"off", "5db", "10db", "15db"};
std::vector<std::string> attnTypes{"off", "-10db", "-20db", "-30db", "-40db"};
std::vector<std::string> MarkerTypes{"off", "M 1", "M 2"};
std::map<int, int> ModesMap{{mode_usb, 0}, {mode_lsb, 1}, {mode_cw, 2}, {mode_dsb, 3}, {mode_am, 4}, {mode_narrowband_fm, 5}, {mode_broadband_fm, 6}};

gui_bar gbar;

std::string mode_string(int mode)
{
	if (!IsDigtalMode(mode))
		return ModesTypes.at(ModesMap.at(mode));
	else
		return ModesTypes.at(ModesMap.at(mode_usb));
}

int gui_bar::get_step_value() 	
{ 
	return stepsValues[steps_value]; 
}

void gui_bar::change_step(int i)
{
	if (steps_value > 0)
		steps_value--;
	update_step_button(steps_value);
}


void gui_bar::web_filterfreq()
{
	json result = json::array();
	json message;
	
	message.emplace("frequency", "0.5 Khz");
	result.push_back(message);
	message.clear();
	message.emplace("frequency", "1.0 Khz");
	result.push_back(message);
	message.clear();
	message.emplace("frequency", "1.5 Khz");
	result.push_back(message);
	message.clear();
	message.emplace("frequency", "2.0 Khz");
	result.push_back(message);
	message.clear();
	message.emplace("frequency", "2.5 Khz");
	result.push_back(message);
	message.clear();
	message.emplace("frequency", "3.0 Khz");
	result.push_back(message);
	message.clear();
	message.emplace("frequency", "3.5 Khz");
	result.push_back(message);
	message.clear();
	message.emplace("frequency", "4.0 Khz");
	result.push_back(message);
	message.clear();

	message.clear();
	message.emplace("type", "filterfrq");
	message.emplace("data", result);
	webserver.SendMessage(message);
}

void gui_bar::set_tx(bool tx)
{
	if (tx)
		lv_obj_add_state(button[buttontune], LV_STATE_CHECKED);
	else
		lv_obj_clear_state(button[buttontune], LV_STATE_CHECKED);
}

void gui_bar::set_mode(int mode)
{
	int i = 0;

	switch (mode)
	{
	case mode_wspr:
	case mode_usb:
	case mode_ft8:
	case mode_ft4:
		i = 0;
		break;
	case mode_lsb:
		i = 1;
		break;
	case mode_am:
		i = 4;
		break;
	case mode_narrowband_fm:
		i = 5;
		break;
	case mode_cw:
		i = 3;
		break;
	case mode_broadband_fm:
		i = 6;
		break;
	}
	std::string txt = std::string("Mode") + std::string("\n#0fff0f ") + ModesTypes[i] + std::string("#");
	lv_label_set_text(label[buttonmode], txt.c_str());
}

void gui_bar::bar_button_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);

	if (code == customLVevents.getCustomEvent(LV_BUTTON_EVENT_CUSTOM) || code == customLVevents.getCustomEvent(LV_SLIDER_EVENT_CUSTOM) || code == customLVevents.getCustomEvent(LV_SLIDER_EVENT_CUSTOM_OK) || code == customLVevents.getCustomEvent(LV_EVENT_STEPS_CUSTOM_OK) || code == customLVevents.getCustomEvent(LV_EVENT_STEPS_CUSTOM))
	{
		if (attenuatorWindow != nullptr)
		{
			attenuatorWindow.reset();
			attenuatorWindow = nullptr;
			lv_obj_clear_state(get_button_obj(buttonatt), LV_STATE_CHECKED);
		}
		if (modeWindow != nullptr)
		{
			modeWindow.reset();
			modeWindow = nullptr;
			lv_obj_clear_state(get_button_obj(buttonmode), LV_STATE_CHECKED);
		}
		if (stepsWindow != nullptr)
		{
			stepsWindow.reset();
			stepsWindow = nullptr;
			lv_obj_clear_state(get_button_obj(buttonsteps), LV_STATE_CHECKED);
		}
		if (MarkerWindow != nullptr)
		{
			MarkerWindow.reset();
			MarkerWindow = nullptr;
			lv_obj_clear_state(get_button_obj(buttonmarker), LV_STATE_CHECKED);
		}
		if (ritWindow != nullptr)
		{
			ritWindow.reset();
			ritWindow = nullptr;
			if (code == customLVevents.getCustomEvent(LV_SLIDER_EVENT_CUSTOM))
			{
				lv_obj_clear_state(get_button_obj(buttonrit), LV_STATE_CHECKED);
				rit_value = 0;
				vfo.setRit(rit_value, vfo.get_active_vfo());
			}
			else
			{
				lv_obj_add_state(get_button_obj(buttonrit), LV_STATE_CHECKED);
			}
		}
		return;
	}

	if (code == customLVevents.getCustomEvent(LV_EVENT_MARKER_CLICKED))
	{
		MarkerWindow.reset();
		MarkerWindow = nullptr;
		long i = (long)e->param;
		switch (i)
		{
		case 0:
			SpectrumGraph.enable_marker(0, false);
			SpectrumGraph.enable_marker(1, false);
			lv_obj_clear_state(get_button_obj(buttonmarker), LV_STATE_CHECKED);
			break;
		case 1:
			SpectrumGraph.enable_marker(0, true);
			break;
		case 2:
			SpectrumGraph.enable_marker(1, true);
			break;
		}
		return;
	}
	
	if (code == customLVevents.getCustomEvent(LV_EVENT_MODE_CLICKED))
	{
		modeWindow.reset();
		modeWindow = nullptr;
		long i = (long)e->param;
		lv_obj_clear_state(get_button_obj(buttonmode), LV_STATE_CHECKED);
		std::string txt = std::string("Mode") + std::string("\n#0fff0f ") + ModesTypes[i] + std::string("#");
		lv_label_set_text(label[buttonmode], txt.c_str());
		select_mode((int)ModesCodes[i], true);
		return;
	}

	if (code == customLVevents.getCustomEvent(LV_EVENT_STEPS_CLICKED))
	{
		long i = (long)e->param;
		update_step_button(i);
		return;
	}

	if (code == customLVevents.getCustomEvent(LV_EVENT_ATT_CLICKED))
	{
		attenuatorWindow.reset();
		attenuatorWindow = nullptr;
		long i = (long)e->param;
		std::string txt = std::string("ATT");
		if (i)
		{
			txt += std::string("\n#0fff0f ") + attnTypes[i] + std::string("#");
		}
		lv_label_set_text(label[buttonatt], txt.c_str());
		lv_obj_clear_state(get_button_obj(buttonatt), LV_STATE_CHECKED);
		return;
	}

	if (code == customLVevents.getCustomEvent(LV_EVENT_RIT_VALUE_CHANGED))
	{
		rit_value = (long)e->param;
		vfo.setRit(rit_value, vfo.get_active_vfo());
		return;
	}

	if (code == LV_EVENT_CLICKED)
	{

		for (int i = 0; i < gbar.getbuttons(); i++)
		{
			if (obj == gbar.get_button_obj(i))
			{
				switch (i)
				{
				case buttontx:
					if (!IsDigtalMode(mode))
					{
						if (lv_obj_get_state(obj) & LV_STATE_CHECKED)
						{
							if (!select_mode_tx(mode))
								lv_obj_clear_state(obj, LV_STATE_CHECKED);
						}
						else
							select_mode(mode);
					}
					else
					{
						lv_obj_clear_state(obj, LV_STATE_CHECKED);
					}
					break;
				case buttontune:
					if (!IsDigtalMode(mode))
					{
						if (lv_obj_get_state(obj) & LV_STATE_CHECKED)
						{
							if (!select_mode_tx(mode, audioTone::SingleTone))
								lv_obj_clear_state(obj, LV_STATE_CHECKED);
						}
						else
							select_mode(mode);
					}
					else
					{
						lv_obj_clear_state(obj, LV_STATE_CHECKED);
					}
					break;
				case buttonmode:
					if (modeWindow == nullptr && !IsDigtalMode(mode))
					{
						modeWindow = std::make_unique<guiButtonWindows>(obj, (void *)this, "Mode", ModesTypes, ModesMap.at(mode), customLVevents.getCustomEvent(LV_EVENT_MODE_CLICKED), 300, 200);
					}
					lv_obj_clear_state(obj, LV_STATE_CHECKED);
					break;
				case buttonvfo:
					if (!IsDigtalMode(mode))
					{
						int m = vfo.get_mode_no(vfo.get_active_vfo());
						if (lv_obj_get_state(obj) & LV_STATE_CHECKED)
						{
							vfo.set_active_vfo(vfo_activevfo::Two);
							std::string txt = std::string("VFO 2");
							lv_label_set_text(label[buttonvfo], txt.c_str());
							
						}
						else
						{
							vfo.set_active_vfo(vfo_activevfo::One);
							std::string txt = std::string("VFO 1");
							lv_label_set_text(label[buttonvfo], txt.c_str());
						}
						if (m != vfo.get_mode_no(vfo.get_active_vfo()))
						{
							select_mode(vfo.get_mode_no(vfo.get_active_vfo()), true);
						}
					}
					else
					{
						lv_obj_clear_state(obj, LV_STATE_CHECKED);
					}
					break;
				case buttonsteps:
					if (stepsWindow == nullptr)
					{
						stepsWindow = std::make_unique<guiListWindows>(obj, (void *)this, "Steps", stepsTypes, steps_value, customLVevents.getCustomEvent(LV_EVENT_STEPS_CLICKED), 200, 300);
					}
					break;
				case buttonatt:
					if (attenuatorWindow == nullptr)
					{
						attenuatorWindow = std::make_unique<guiButtonWindows>(obj, (void *)this, "Attenuator", attnTypes, -1, customLVevents.getCustomEvent(LV_EVENT_ATT_CLICKED), 300, 200);
					}
					break;
				case buttonnoise:
					// Noise
					if (!IsDigtalMode(mode))
					{
						if (lv_obj_get_state(obj) & LV_STATE_CHECKED)
						{
							Demodulator::set_noise_filter(guirx.get_noise() + 1);
						}
						else
						{
							Demodulator::set_noise_filter(0);
						}
					}
					else
					{
						lv_obj_clear_state(obj, LV_STATE_CHECKED);
					}
					break;
				case buttonrit:
					if (!IsDigtalMode(mode))
					{
						if (ritWindow == nullptr)
						{
							ritWindow = std::make_unique<guiSliderWindows>(obj, (void *)this, "Rit", std::vector<std::string>(), rit_value, customLVevents.getCustomEvent(LV_EVENT_RIT_VALUE_CHANGED), 180, 200);
							lv_obj_add_state(get_button_obj(buttonrit), LV_STATE_CHECKED);
						}
					}
					break;
				case buttonmarker:
					if (MarkerWindow == nullptr)
					{
						MarkerWindow = std::make_unique<guiButtonWindows>(obj, (void *)this, "Marker", MarkerTypes, -1, customLVevents.getCustomEvent(LV_EVENT_MARKER_CLICKED), 300, 200);
						lv_obj_add_state(get_button_obj(buttonmarker), LV_STATE_CHECKED);
					}
					break;
				}
			}
		}
	}
}

void gui_bar::update_step_button(int step)
{
	steps_value = step;
	std::string txt = std::string("Steps");
	txt += std::string("\n#0fff0f ") + stepsTypes[step] + std::string("#");
	lv_label_set_text(label[buttonsteps], txt.c_str());
	lv_obj_clear_state(get_button_obj(buttonsteps), LV_STATE_CHECKED);
	if (steps_value < stepsValues.size())
		vfo.set_step(stepsValues.at(steps_value), 0);
	Settings_file.save_int("Radio", "steps", steps_value);
	Settings_file.write_settings();
}

void gui_bar::vol_slider_event_class(lv_event_t *e)
{
	lv_obj_t *slider = lv_event_get_target(e);
	lv_label_set_text_fmt(gbar.get_vol_slider_label(), "vol %d", lv_slider_get_value(slider));
	audio_output->set_volume(lv_slider_get_value(slider));
	catinterface->SetAG(lv_slider_get_value(slider));
	Settings_file.save_vol(lv_slider_get_value(slider));
	updateweb();
}

void gui_bar::if_slider_event_class(lv_event_t *e)
{
	lv_obj_t *slider = lv_event_get_target(e);
	lv_label_set_text_fmt(get_if_slider_label(), "if %d db", lv_slider_get_value(slider));
	int sl = lv_slider_get_value(slider);
	ifgain.store(std::pow(10.0, (float)sl / 20.0));
	catinterface->SetIG(lv_slider_get_value(slider));
	if_gain[std::to_string(vfo.get_band_in_meters()) + "m"] = lv_slider_get_value(slider);
	Settings_file.set_map_string(default_radio, "if-gain-map", if_gain);
	guift8bar.set_if(sl);
	updateweb();
}

void gui_bar::gain_slider_event_class(lv_event_t *e)
{
	lv_obj_t *slider = lv_event_get_target(e);

	lv_label_set_text_fmt(get_gain_slider_label(), "rf %d db", lv_slider_get_value(slider));
	catinterface->SetRG(lv_slider_get_value(slider));
	guigain.set_gains();
	rf_gain[std::to_string(vfo.get_band_in_meters()) + "m"] = lv_slider_get_value(slider);
	Settings_file.set_map_string(default_radio, "rf-gain-map", rf_gain);
	guigain.clear_store_class();
	updateweb();
	try
	{
		SdrDevices.SdrDevices.at(default_radio)->setGain(SOAPY_SDR_RX, guisdr.get_current_rx_channel(), lv_slider_get_value(slider));
	}
	catch (const std::exception &e)
	{
		std::cout << e.what();
	}
}

void gui_bar::step_gain_slider(int step)
{
	set_gain_slider(lv_slider_get_value(gain_slider) + step);
}

gui_bar::gui_bar()
	: rit_value(0)
{
}

gui_bar::~gui_bar()
{
}

void gui_bar::set_gain_slider_band_from_config(bool web)
{
	int gain = 10;

	rf_gain = Settings_file.get_map_string(default_radio, "rf-gain-map");
	gain = rf_gain[to_string(vfo.get_band_in_meters()) + "m"];
	set_gain_slider(gain, web);

	if_gain = Settings_file.get_map_string(default_radio, "if-gain-map");
	gain = if_gain[to_string(vfo.get_band_in_meters()) + "m"];
	set_if(gain);
}

void gui_bar::set_gain_slider(int gain, bool web)
{
	double max_gain{0.0};
	double min_gain{0.0};

	try
	{
		max_gain = (double)SdrDevices.SdrDevices.at(default_radio)->rx_channels.at(guisdr.get_current_rx_channel())->get_full_gain_range().maximum();
		min_gain = (double)SdrDevices.SdrDevices.at(default_radio)->rx_channels.at(guisdr.get_current_rx_channel())->get_full_gain_range().minimum();
	}
	catch (const std::exception &e)
	{
		std::cout << e.what();
		return;
	}
	if (gain > max_gain)
		gain = max_gain;
	if (gain < min_gain)
		gain = min_gain;
	lv_label_set_text_fmt(gain_slider_label, "rf %d db", gain);
	lv_slider_set_value(gain_slider, gain, LV_ANIM_ON);
	rf_gain[std::to_string(vfo.get_band_in_meters()) + "m"] = gain;
	Settings_file.set_map_string(default_radio, "rf-gain-map", rf_gain);
	guigain.set_gains();
	catinterface->SetRG(gain);
	if (web)
		updateweb();
	try
	{
		SdrDevices.SdrDevices.at(default_radio)->setGain(SOAPY_SDR_RX, guisdr.get_current_rx_channel(), (double)gain);
	}
	catch (const std::exception &e)
	{
		std::cout << e.what();
	}
}

void gui_bar::filter_slider_event_class(lv_event_t *e)
{
	lv_obj_t *obj = lv_event_get_target(e);
	lv_event_code_t code = lv_event_get_code(e);

	if (code == LV_EVENT_VALUE_CHANGED)
	{
		if (!IsDigtalMode(mode))
		{

			int sel = lv_dropdown_get_selected(obj);
			catinterface->SetNA(sel);
			update_filter(ifilters.at(sel));
			filter_to_mode_cutoff_frequencies[ModesTypes.at(ModesMap.at(mode))] = ifilters.at(sel);
			Settings_file.set_map_string("Radio", "Audiofilter", filter_to_mode_cutoff_frequencies);
			catinterface->SetNA(sel);
			updateweb();
		}
		else
		{
			lv_dropdown_set_selected(obj,7);
		}
	}
}

void gui_bar::set_gain_range()
{
	int max_gain = 100, min_gain = 0;

	try
	{
		max_gain = (int)SdrDevices.SdrDevices.at(default_radio)->rx_channels.at(guisdr.get_current_rx_channel())->get_full_gain_range().maximum();
		min_gain = (int)SdrDevices.SdrDevices.at(default_radio)->rx_channels.at(guisdr.get_current_rx_channel())->get_full_gain_range().minimum();
	}
	catch (const std::exception &e)
	{
		std::cout << e.what();
	}
	lv_slider_set_range(gain_slider, min_gain, max_gain);
}

void gui_bar::init(lv_obj_t *o_parent, lv_group_t *button_group, int mode, lv_coord_t w, lv_coord_t h)
{
	const lv_coord_t x_margin = 2;
	const lv_coord_t y_margin = 2; // 5;
	const int x_number_buttons = 5;
	const int y_number_buttons = 2;
	const int max_rows = 2;
	const int slide_max_rows = 3;
	const lv_coord_t tab_margin = w / 3;
	const int cw_margin = 20;

	int button_width_margin = ((w - tab_margin) / (x_number_buttons + 1));
	int button_width = ((w - tab_margin) / (x_number_buttons + 1)) - x_margin;
	int button_height = h / max_rows - y_margin - y_margin;
	int button_height_margin = button_height + y_margin;
	int ibutton_x = 0, ibutton_y = 0;
	int i = 0;
	int slider_height_margin = h / slide_max_rows - y_margin;
	int button_width_dropdown = button_width;

	ifilters.push_back(500);
	ifilters.push_back(1000);
	ifilters.push_back(1500);
	ifilters.push_back(2000);
	ifilters.push_back(2500);
	ifilters.push_back(3000);
	ifilters.push_back(3500);
	ifilters.push_back(4000);
	ifilters.push_back(4500);
	ifilters.push_back(5000);

	filter_to_mode_cutoff_frequencies = Settings_file.get_map_string("Radio", "Audiofilter");
	steps_value = Settings_file.get_int("Radio", "steps", 0);
	barview = o_parent;
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

	lv_style_init(&ifGainStyleKnob);
	lv_style_set_bg_color(&ifGainStyleKnob, lv_palette_main(LV_PALETTE_RED));

	lv_style_init(&ifGainStyleIndicator);
	lv_style_set_bg_opa(&ifGainStyleIndicator, LV_OPA_COVER);
	lv_style_set_bg_color(&ifGainStyleIndicator, lv_palette_main(LV_PALETTE_RED));

	lv_obj_set_style_pad_hor(o_parent, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(o_parent, 0, LV_PART_MAIN);
	lv_obj_clear_flag(o_parent, LV_OBJ_FLAG_SCROLLABLE);
	buttongroup = button_group;

	ibuttons = number_of_buttons;
	for (i = 0; i < ibuttons; i++)
	{
		char str[80];

		if (i < number_of_buttons - 1)
		{
			button[i] = lv_btn_create(o_parent);
			lv_obj_add_style(button[i], &style_btn, 0);
			lv_obj_add_event_cb(button[i], bar_button_handler, LV_EVENT_CLICKED, (void *)this);
			lv_obj_align(button[i], LV_ALIGN_TOP_LEFT, ibutton_x * button_width_margin, y_margin + ibutton_y * button_height_margin);
			lv_obj_set_size(button[i], button_width, button_height);
			lv_group_add_obj(button_group, button[i]);

			label[i] = lv_label_create(button[i]);
			switch (i)
			{
			case buttontx:
				lv_obj_add_flag(button[buttontx], LV_OBJ_FLAG_CHECKABLE);
				strcpy(str, "TX");
				if (SdrDevices.get_tx_channels(default_radio) == 0)
					lv_obj_add_state(button[buttontx], LV_STATE_DISABLED);
				break;
			case buttontune:
				lv_obj_add_flag(button[buttontune], LV_OBJ_FLAG_CHECKABLE);
				strcpy(str, "Tune");
				if (SdrDevices.get_tx_channels(default_radio) == 0)
					lv_obj_add_state(button[buttontune], LV_STATE_DISABLED);
				break;
			case buttonmode:
				strcpy(str, "Mode");
				lv_obj_add_event_cb(button[buttonmode], bar_button_handler, customLVevents.getCustomEvent(LV_EVENT_MODE_CLICKED), (void *)this);
				lv_obj_add_event_cb(button[buttonmode], bar_button_handler, customLVevents.getCustomEvent(LV_BUTTON_EVENT_CUSTOM), (void *)this);
				lv_obj_add_flag(button[buttonmode], LV_OBJ_FLAG_CHECKABLE);
				break;
			case buttonvfo:
				lv_obj_add_flag(button[buttonvfo], LV_OBJ_FLAG_CHECKABLE);
				strcpy(str, "VFO 1");
				break;
			case buttonsteps: 
				{
					lv_obj_add_flag(button[buttonsteps], LV_OBJ_FLAG_CHECKABLE);
					std::string txt = std::string("Steps");
					txt += std::string("\n#0fff0f ") + stepsTypes[steps_value] + std::string("#");
					strcpy(str, txt.c_str());
					lv_obj_add_event_cb(button[buttonsteps], bar_button_handler, customLVevents.getCustomEvent(LV_EVENT_STEPS_CLICKED), (void *)this);
					lv_obj_add_event_cb(button[buttonsteps], bar_button_handler, customLVevents.getCustomEvent(LV_EVENT_STEPS_CUSTOM_OK), (void *)this);
					lv_obj_add_event_cb(button[buttonsteps], bar_button_handler, customLVevents.getCustomEvent(LV_EVENT_STEPS_CUSTOM), (void *)this);
				}
				break;
			case buttonatt:
				lv_obj_add_flag(button[buttonatt], LV_OBJ_FLAG_CHECKABLE);
				strcpy(str, "ATT");
				lv_obj_add_event_cb(button[buttonatt], bar_button_handler, customLVevents.getCustomEvent(LV_EVENT_ATT_CLICKED), (void *)this);
				lv_obj_add_event_cb(button[buttonatt], bar_button_handler, customLVevents.getCustomEvent(LV_BUTTON_EVENT_CUSTOM), (void *)this);
				break;
			case buttonnoise:
				lv_obj_add_flag(button[buttonnoise], LV_OBJ_FLAG_CHECKABLE);
				strcpy(str, "Noise");
				break;
			case buttonrit:
				lv_obj_add_flag(button[buttonrit], LV_OBJ_FLAG_CHECKABLE);
				strcpy(str, "Rit");
				lv_obj_add_event_cb(button[buttonrit], bar_button_handler, customLVevents.getCustomEvent(LV_SLIDER_EVENT_CUSTOM), (void *)this);
				lv_obj_add_event_cb(button[buttonrit], bar_button_handler, customLVevents.getCustomEvent(LV_SLIDER_EVENT_CUSTOM_OK), (void *)this);
				lv_obj_add_event_cb(button[buttonrit], bar_button_handler, customLVevents.getCustomEvent(LV_EVENT_RIT_VALUE_CHANGED), (void *)this);
				break;
			case buttonmarker:
				lv_obj_add_flag(button[buttonmarker], LV_OBJ_FLAG_CHECKABLE);
				strcpy(str, "Marker");
				lv_obj_add_event_cb(button[buttonmarker], bar_button_handler, customLVevents.getCustomEvent(LV_BUTTON_EVENT_CUSTOM), (void *)this);
				lv_obj_add_event_cb(button[buttonmarker], bar_button_handler, customLVevents.getCustomEvent(LV_EVENT_MARKER_CLICKED), (void *)this);
				break;
			}
			lv_label_set_recolor(label[i], true);
			lv_label_set_text(label[i], str);
			lv_obj_center(label[i]);
		}
		else
		{
			filter = i;
			button[i] = lv_dropdown_create(o_parent);
			lv_dropdown_set_options_static(button[i], opts);
			lv_obj_align(button[i], LV_ALIGN_TOP_LEFT, ibutton_x * button_width_margin, y_margin + ibutton_y * button_height_margin);
			lv_obj_set_size(button[i], button_width_dropdown, button_height);
			lv_obj_add_style(button[i], &style_btn, 0);
			lv_obj_add_event_cb(button[i], filter_slider_event_cb, LV_EVENT_VALUE_CHANGED, (void *)this);
			lv_group_add_obj(button_group, button[i]);
		}

		ibutton_x++;
		if (ibutton_x >= x_number_buttons)
		{
			ibutton_x = 0;
			ibutton_y++;
		}
	}

	int vol_x = x_number_buttons * button_width_margin + 10;
	int vol_width = (w / 3); //	-30;
	vol_slider_label = lv_label_create(o_parent);
	lv_label_set_text(vol_slider_label, "vol");
	lv_obj_align(vol_slider_label, LV_ALIGN_TOP_LEFT, vol_x + vol_width + 5, 15);
	vol_slider = lv_slider_create(o_parent);
	lv_slider_set_range(vol_slider, 0, 100);
	lv_obj_set_width(vol_slider, vol_width);
	lv_obj_align(vol_slider, LV_ALIGN_TOP_LEFT, vol_x, 15);
	lv_obj_add_event_cb(vol_slider, vol_slider_event_cb, LV_EVENT_VALUE_CHANGED, (void *)this);

	int gain_y = 15 + slider_height_margin;
	if_slider_label = lv_label_create(o_parent);
	lv_label_set_text(if_slider_label, "if 60 db");
	lv_obj_align(if_slider_label, LV_ALIGN_TOP_LEFT, vol_x + vol_width + 5, gain_y);
	if_slider = lv_slider_create(o_parent);
	lv_slider_set_range(if_slider, 0, maxifgain);
	lv_obj_set_width(if_slider, vol_width);
	lv_obj_align(if_slider, LV_ALIGN_TOP_LEFT, vol_x, gain_y);
	lv_obj_add_event_cb(if_slider, if_slider_event_cb, LV_EVENT_VALUE_CHANGED, (void *)this);
	lv_slider_set_value(if_slider, 60, LV_ANIM_OFF);

	gain_y += (slider_height_margin);
	gain_slider_label = lv_label_create(o_parent);
	lv_label_set_text(gain_slider_label, "gain");
	lv_obj_align(gain_slider_label, LV_ALIGN_TOP_LEFT, vol_x + vol_width + 5, gain_y);
	gain_slider = lv_slider_create(o_parent);
	lv_slider_set_range(gain_slider, 0, 100);
	lv_obj_set_width(gain_slider, vol_width);
	lv_obj_align(gain_slider, LV_ALIGN_TOP_LEFT, vol_x, gain_y);
	lv_obj_add_event_cb(gain_slider, gain_slider_event_cb, LV_EVENT_VALUE_CHANGED, (void *)this);

	lv_group_add_obj(button_group, vol_slider);
	lv_group_add_obj(button_group, if_slider);
	lv_group_add_obj(button_group, gain_slider);

	try
	{
		if (SdrDevices.SdrDevices.at(default_radio)->rx_channels.at(guisdr.get_current_rx_channel())->get_agc())
		{
			string sagc = Settings_file.get_string(default_radio, "AGC");
			if (sagc == "off")
				SdrDevices.SdrDevices.at(default_radio)->setGainMode(SOAPY_SDR_RX, guisdr.get_current_rx_channel(), false);
			else
			{
				bool bAgc = SdrDevices.SdrDevices.at(default_radio)->getGainMode(SOAPY_SDR_RX, guisdr.get_current_rx_channel());
				if (bAgc)
					lv_obj_add_state(button[9], LV_STATE_CHECKED);
			}
		}
	}
	catch (const std::exception &e)
	{
		std::cout << e.what();
	}

	int cw_y = y_margin + max_rows * button_height_margin;

	lv_style_init(&cw_style);
	lv_style_set_radius(&cw_style, 0);
	lv_style_set_bg_color(&cw_style, lv_color_black());

	cw_box = lv_obj_create(o_parent);
	lv_obj_add_style(cw_box, &cw_style, 0);
	lv_obj_set_pos(cw_box, 2, cw_y);
	lv_obj_set_size(cw_box, 5 * button_width_margin, 30);
	lv_obj_clear_flag(cw_box, LV_OBJ_FLAG_SCROLLABLE);

	cw_wpm = lv_label_create(cw_box);
	lv_obj_align(cw_wpm, LV_ALIGN_BOTTOM_LEFT, 30, 15);
	lv_label_set_text(cw_wpm, "wpm: ");

	cw_message = lv_label_create(cw_box);
	lv_obj_align(cw_message, LV_ALIGN_BOTTOM_LEFT, 100, 15);
	lv_label_set_text(cw_message, "....");

	cw_led = lv_led_create(cw_box);
	lv_obj_align(cw_led, LV_ALIGN_BOTTOM_LEFT, 0, 15);
	lv_led_set_color(cw_led, lv_palette_main(LV_PALETTE_RED));
	lv_obj_set_size(cw_led, 15, 15);
	lv_led_off(cw_led);
	hide_cw(true);

	lv_style_init(&style_selected_color);
	lv_style_set_bg_opa(&style_selected_color, LV_OPA_COVER);
	lv_style_set_bg_color(&style_selected_color, lv_palette_main(LV_PALETTE_YELLOW));
}

void gui_bar::set_focus()
{
	lv_group_focus_obj(button[0]);
}

void gui_bar::select_option(int option)
{
	switch (option)
	{
	case 0:
		lv_obj_remove_style(vol_slider, &style_selected_color, LV_PART_KNOB);
		lv_obj_remove_style(gain_slider, &style_selected_color, LV_PART_KNOB);
		break;
	case 1:
		lv_obj_remove_style(gain_slider, &style_selected_color, LV_PART_KNOB);
		lv_obj_add_style(vol_slider, &style_selected_color, LV_PART_KNOB);
		break;
	case 2:
		lv_obj_remove_style(vol_slider, &style_selected_color, LV_PART_KNOB);
		lv_obj_add_style(gain_slider, &style_selected_color, LV_PART_KNOB);
		break;
	}
}

void gui_bar::hide_cw(bool hide)
{
	if (hide)
	{
		lv_obj_add_flag(cw_led, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(cw_box, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(cw_wpm, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(cw_message, LV_OBJ_FLAG_HIDDEN);
	}
	else
	{
		lv_obj_clear_flag(cw_led, LV_OBJ_FLAG_HIDDEN);
		lv_obj_clear_flag(cw_box, LV_OBJ_FLAG_HIDDEN);
		lv_obj_clear_flag(cw_wpm, LV_OBJ_FLAG_HIDDEN);
		lv_obj_clear_flag(cw_message, LV_OBJ_FLAG_HIDDEN);
	}
}

void gui_bar::setIfGainOverflow(bool state)
{
	if (state)
	{
		if (ifStyleState == false)
		{
			ifStyleState = true;
			lv_obj_add_style(if_slider, &ifGainStyleKnob, LV_PART_KNOB);
			lv_obj_add_style(if_slider, &ifGainStyleIndicator, LV_PART_INDICATOR);
		}
	}
	else
	{
		if (ifStyleState == true)
		{
			ifStyleState = false;
			lv_obj_remove_style(if_slider, &ifGainStyleKnob, LV_PART_KNOB);
			lv_obj_remove_style(if_slider, &ifGainStyleIndicator, LV_PART_INDICATOR);
		}
	}
}

void gui_bar::set_cw_message(std::string message)
{
	lv_label_set_text(cw_message, message.c_str());
}

void gui_bar::set_cw_wpm(int wpm)
{
	lv_label_set_text_fmt(cw_wpm, "wpm: %d", wpm);
}

void gui_bar::set_led(bool status)
{
	if (status)
		lv_led_on(cw_led);
	else
		lv_led_off(cw_led);
}

void gui_bar::step_vol_slider(int step)
{
	set_vol_slider(lv_slider_get_value(vol_slider) + step);
}

void gui_bar::set_vol_slider(int volume, bool web)
{
	if (volume < 0)
		volume = 0;
	if (volume > max_volume)
		volume = max_volume;
	lv_slider_set_value(vol_slider, volume, LV_ANIM_ON);
	lv_label_set_text_fmt(vol_slider_label, "vol %d", volume);
	audio_output->set_volume(volume);
	catinterface->SetAG(volume);
	Settings_file.write_settings();
	Settings_file.save_vol(volume);
	if (web)
		updateweb();
}

bool gui_bar::get_noise()
{
	if (lv_obj_get_state(button[buttonnoise]) & LV_STATE_CHECKED)
		return true;
	return false;
}

int gui_bar::get_vol_range()
{
	return max_volume;
}

int gui_bar::get_if_slider()
{
	return lv_slider_get_value(if_slider);
}

float gui_bar::get_if()
{
	return ifgain.load();
}

void gui_bar::set_if(int ifg, bool web)
{
	if (ifg > maxifgain)
		ifg = maxifgain;
	ifgain.store(std::pow(10.0, (float)ifg / 20.0));
	lv_slider_set_value(if_slider, ifg, LV_ANIM_ON);
	lv_label_set_text_fmt(if_slider_label, "if %d db", ifg);
	catinterface->SetIG(ifg);
	if_gain[std::to_string(vfo.get_band_in_meters()) + "m"] = ifg;
	Settings_file.set_map_string(default_radio, "if-gain-map", if_gain);
	guift8bar.set_if(ifg);
	if (web)
		updateweb();
}

void gui_bar::get_filter_range(vector<string> &filters)
{
	filters.push_back("0.5 Khz");
	filters.push_back("1 Khz");
	filters.push_back("1.5 Khz");
	filters.push_back("2 Khz");
	filters.push_back("2.5 Khz");
	filters.push_back("3 Khz");
	filters.push_back("3.5 Khz");
	filters.push_back("4 Khz");
	filters.push_back("11 Khz");
	filters.push_back("16 Khz");
}

void gui_bar::hide(bool hide)
{
	if (hide)
	{
		lv_obj_add_flag(barview, LV_OBJ_FLAG_HIDDEN);
	}
	else
	{
		lv_obj_clear_flag(barview, LV_OBJ_FLAG_HIDDEN);
	}
}

void gui_bar::set_vfo(int active_vfo)
{
	if (active_vfo)
	{
		lv_obj_add_state(button[buttonvfo], LV_STATE_CHECKED);
		vfo.set_active_vfo(1);
		std::string txt = std::string("VFO 2");
		lv_label_set_text(label[buttonvfo], txt.c_str());
	}
	else
	{
		lv_obj_clear_state(button[buttonvfo], LV_STATE_CHECKED);
		vfo.set_active_vfo(0);
		std::string txt = std::string("VFO 1");
		lv_label_set_text(label[buttonvfo], txt.c_str());
	}
}

int gui_bar::get_filter_frequency(int mode)
{ // filter frequencies are stored per mode like USB, LSB etc first get the mode (rx/tx code) translate to index in ModeTypes
  // ModeTypes is a list of strings "USB", "LSB" etc ModesMap.at(mode) gets the index
	
	if (filter_to_mode_cutoff_frequencies.find(ModesTypes.at(ModesMap.at(mode))) != filter_to_mode_cutoff_frequencies.end())
		return filter_to_mode_cutoff_frequencies.at(ModesTypes.at(ModesMap.at(mode)));
	else
		return 0;
}

void gui_bar::set_filter_dropdown(int ifilter)
{
	int filter = 7;

	if (ifilter < 500 || ifilter > 110000)
	{
		printf("set filter out of range %d\n", ifilter);
		return;
	}
	if (ifilter >= 500 && ifilter < 1000)
		filter = 0;
	if (ifilter >= 1000 && ifilter < 1500)
		filter = 1;
	if (ifilter >= 1500 && ifilter < 2000)
		filter = 2;
	if (ifilter >= 2000 && ifilter < 2500)
		filter = 3;
	if (ifilter >= 2500 && ifilter < 3000)
		filter = 4;
	if (ifilter >= 3000 && ifilter < 3500)
		filter = 5;
	if (ifilter >= 3500 && ifilter < 4000)
		filter = 6;
	if (ifilter >= 4000 && ifilter < 11000)
		filter = 7;
	if (ifilter >= 11000 && ifilter < 16000)
		filter = 8;
	if (ifilter >= 16000)
		filter = 9;

	if (filter < 0 || filter > 9)
		filter = 9;
	lv_dropdown_set_selected(button[button_filter], filter);
	update_filter(ifilters[filter]);
	catinterface->SetNA(filter);
	if (!IsDigtalMode(mode))
	{
		filter_to_mode_cutoff_frequencies[ModesTypes.at(ModesMap.at(mode))] = ifilter;
		Settings_file.set_map_string("Radio", "Audiofilter", filter_to_mode_cutoff_frequencies);
	}
	updateweb();
}

void gui_bar::set_filter_number(int sel)
{
	if (sel < 0 || sel >= FilterValues.size())
		return;
	set_filter_dropdown(FilterValues.at(sel));
}

void gui_bar::get_gain_range(int &max_gain, int &min_gain)
{
	try
	{
		max_gain = (int)SdrDevices.SdrDevices.at(default_radio)->rx_channels.at(guisdr.get_current_rx_channel())->get_full_gain_range().maximum();
		min_gain = (int)SdrDevices.SdrDevices.at(default_radio)->rx_channels.at(guisdr.get_current_rx_channel())->get_full_gain_range().minimum();
	}
	catch (const std::exception &e)
	{
		std::cout << e.what() << endl;
		max_gain = 100;
		min_gain = 0;
	}
	return;
}

void gui_bar::hidetx()
{
	if (SdrDevices.get_tx_channels(default_radio) == 0)
	{
		lv_obj_add_flag(button[buttontx], LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(button[buttontune], LV_OBJ_FLAG_HIDDEN);
	}
	else
	{
		lv_obj_clear_flag(button[buttontx], LV_OBJ_FLAG_HIDDEN);
		lv_obj_clear_flag(button[buttontune], LV_OBJ_FLAG_HIDDEN);
	}
	return;
}

void gui_bar::updateweb()
{
	json message, data;

	if (webserver.isEnabled())
	{
		int max_gain, min_gain;

		get_gain_range(max_gain, min_gain);
		data.clear();
		data.emplace("volume", get_volume());
		data.emplace("ifvalue", get_if_slider());
		data.emplace("rfvalue", get_rf_gain() - min_gain);
		data.emplace("filter", FilterTypes.at(lv_dropdown_get_selected(button[filter])));
		data.emplace("ifilter", FilterValues.at(lv_dropdown_get_selected(button[filter])));
		message.emplace("type", "sliders");
		message.emplace("data", data);
		webserver.SendMessage(message);
	}
}

void gui_bar::websetfilter(std::string message)
{
	int filter = 0;
	for (auto col : FilterTypes)
	{
		if (col == message)
		{
			lv_dropdown_set_selected(button[button_filter], filter);
			update_filter(ifilters[filter]);
			catinterface->SetNA(filter);
		}
		filter++;
	}
}

void gui_bar::setTxButtons()
{
	if (SdrDevices.get_tx_channels(default_radio) == 0 || !audio_input->isStreamOpen())
		lv_obj_add_state(button[buttontx], LV_STATE_DISABLED);
	else
		lv_obj_clear_state(button[buttontx], LV_STATE_DISABLED);

	if (SdrDevices.get_tx_channels(default_radio) == 0 || !audio_input->isStreamOpen())
		lv_obj_add_state(button[buttontune], LV_STATE_DISABLED);
	else
		lv_obj_clear_state(button[buttontune], LV_STATE_DISABLED);
}