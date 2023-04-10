#include "gui_cal.h"

gui_cal gcal;
extern std::string default_radio;

static void gain_slider_event_cb(lv_event_t *e)
{
	lv_obj_t *slider = lv_event_get_target(e);
	lv_label_set_text_fmt(gcal.get_txgain_slider_label(), "tx gain %d", lv_slider_get_value(slider));
	gcal.setTxGain(lv_slider_get_value(slider));
	Settings_file.save_int(default_radio, "tx-gain-correction", lv_slider_get_value(slider));
}

static void phase_slider_event_cb(lv_event_t *e)
{
	lv_obj_t *slider = lv_event_get_target(e);
	lv_label_set_text_fmt(gcal.get_txphase_slider_label(), "tx phase %d", lv_slider_get_value(slider));
	gcal.setTxPhase(lv_slider_get_value(slider));
	Settings_file.save_int(default_radio, "tx-phase-correction", lv_slider_get_value(slider));
}

static void rxgain_slider_event_cb(lv_event_t *e)
{
	lv_obj_t *slider = lv_event_get_target(e);
	lv_label_set_text_fmt(gcal.get_rxgain_slider_label(), "rx gain %d", lv_slider_get_value(slider));
	gcal.setRxGain(lv_slider_get_value(slider));
	Settings_file.save_int(default_radio, "rx-gain-correction", lv_slider_get_value(slider));
}

static void rxphase_slider_event_cb(lv_event_t *e)
{
	lv_obj_t *slider = lv_event_get_target(e);
	lv_label_set_text_fmt(gcal.get_rxphase_slider_label(), "rx phase %d", lv_slider_get_value(slider));
	gcal.setRxPhase(lv_slider_get_value(slider));
	Settings_file.save_int(default_radio, "rx-phase-correction", lv_slider_get_value(slider));
}

void gui_cal::init(lv_obj_t *o_parent, lv_group_t *button_group, lv_group_t *keyboard_group, lv_coord_t w, lv_coord_t h)
{
	const lv_coord_t x_margin_dropdown = 0;
	const lv_coord_t x_margin = 2;
	const lv_coord_t y_margin = 2; //5;
	const int x_number_buttons = 6;
	const int y_number_buttons = 4;
	const int max_rows = 3;
	const lv_coord_t tab_margin = w / 3;
	const int cw_margin = 20;
	const int number_of_pushbuttons = 6;

	int button_width_margin = ((w - tab_margin) / (x_number_buttons + 1));
	int button_width = ((w - tab_margin) / (x_number_buttons + 1)) - x_margin;
	int button_height = h / max_rows - y_margin - y_margin;
	int button_height_margin = button_height + y_margin;
	int ibutton_x = 0, ibutton_y = 0;
	int i = 0;


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

	lv_obj_set_style_pad_hor(o_parent, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(o_parent, 0, LV_PART_MAIN);
	lv_obj_clear_flag(o_parent, LV_OBJ_FLAG_SCROLLABLE);
	buttonGroup = button_group;
	int txgaincorrection = Settings_file.get_int(default_radio, "tx-gain-correction", 0);
	int txphasecorrection = Settings_file.get_int(default_radio, "tx-phase-correction", 0);
	int rxgaincorrection = Settings_file.get_int(default_radio, "rx-gain-correction", 0);
	int rxphasecorrection = Settings_file.get_int(default_radio, "rx-phase-correction", 0);

	int gainx = 10 ;
	int gainwidth = (w / 3); //	-30;
	txgainlabel = lv_label_create(o_parent);
	lv_label_set_text_fmt(txgainlabel, "tx gain %d", txgaincorrection);

	lv_obj_align(txgainlabel, LV_ALIGN_TOP_LEFT, gainx + gainwidth + 5, 15);
	txgainslider = lv_slider_create(o_parent);
	lv_slider_set_range(txgainslider, -150, 150);
	lv_obj_set_width(txgainslider, gainwidth);
	lv_obj_align(txgainslider, LV_ALIGN_TOP_LEFT, gainx, 15);
	lv_obj_add_event_cb(txgainslider, gain_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	lv_slider_set_value(txgainslider, txgaincorrection, LV_ANIM_ON);
	
	int phase_y = 15 + button_height_margin;
	txphaselabel = lv_label_create(o_parent);
	lv_label_set_text_fmt(txphaselabel, "tx phase %d", txphasecorrection);
	lv_obj_align(txphaselabel, LV_ALIGN_TOP_LEFT, gainx + gainwidth + 5, phase_y);
	txphaseslider = lv_slider_create(o_parent);
	lv_slider_set_range(txphaseslider, -50, 50);
	lv_obj_set_width(txphaseslider, gainwidth);
	lv_obj_align(txphaseslider, LV_ALIGN_TOP_LEFT, gainx, phase_y);
	lv_obj_add_event_cb(txphaseslider, phase_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	lv_slider_set_value(txphaseslider, txphasecorrection, LV_ANIM_ON);

	int rx_x_offset = gainx + gainwidth + 100;
	rxgainlabel = lv_label_create(o_parent);
	lv_label_set_text_fmt(rxgainlabel, "rx gain %d", rxgaincorrection);
	lv_obj_align(rxgainlabel, LV_ALIGN_TOP_LEFT, rx_x_offset+ gainx + gainwidth + 5, 15);
	rxgainslider = lv_slider_create(o_parent);
	lv_slider_set_range(rxgainslider, -50, 50);
	lv_obj_set_width(rxgainslider, gainwidth);
	lv_obj_align(rxgainslider, LV_ALIGN_TOP_LEFT, rx_x_offset + gainx, 15);
	lv_obj_add_event_cb(rxgainslider, rxgain_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	lv_slider_set_value(rxgainslider, rxgaincorrection, LV_ANIM_ON);

	rxphaselabel = lv_label_create(o_parent);
	lv_label_set_text_fmt(rxphaselabel, "rx phase %d", rxphasecorrection);
	lv_obj_align(rxphaselabel, LV_ALIGN_TOP_LEFT, rx_x_offset + gainx + gainwidth + 5, phase_y);
	rxphaseslider = lv_slider_create(o_parent);
	lv_slider_set_range(rxphaseslider, -50, 50);
	lv_obj_set_width(rxphaseslider, gainwidth);
	lv_obj_align(rxphaseslider, LV_ALIGN_TOP_LEFT, rx_x_offset + gainx, phase_y);
	lv_obj_add_event_cb(rxphaseslider, rxphase_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	lv_slider_set_value(rxphaseslider, rxphasecorrection, LV_ANIM_ON);
}

void gui_cal::hide(bool hide)
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