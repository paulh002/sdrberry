#include "guiSliderWindows.h"
#include "CustomEvents.h"

LV_IMG_DECLARE(ui_img_btn_knob_png); // assets/btn_knob.png

const int windowsliderbar = 40;

void guiSliderWindows::lv_spinbox_increment_event_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	if (code == LV_EVENT_SHORT_CLICKED || code == LV_EVENT_LONG_PRESSED_REPEAT)
	{
		lv_spinbox_increment(spinbox);
		value = lv_spinbox_get_value(spinbox);
		lv_obj_send_event(Parent, LV_EVENT_VALUE_CHANGED, (void *)(long)value);
	}
}

void guiSliderWindows::lv_spinbox_decrement_event_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	if (code == LV_EVENT_SHORT_CLICKED || code == LV_EVENT_LONG_PRESSED_REPEAT)
	{
		lv_spinbox_decrement(spinbox);
		value = lv_spinbox_get_value(spinbox);
		lv_obj_send_event(Parent, LV_EVENT_VALUE_CHANGED, (void *)(long)value);
	}
}


guiSliderWindows::guiSliderWindows(lv_obj_t *parent, void *thisptr, std::string name, std::vector<std::string> buttons, int val, lv_event_code_t eventNumber, int width, int height)
	: Parent(parent), thisPtr(thisptr), event(eventNumber), value(val)
{
	if (buttonWindowObj == nullptr)
	{
		buttonWindowObj = lv_win_create(lv_scr_act()); // windowsliderbar
		lv_win_add_title(buttonWindowObj, name.c_str());
		lv_obj_t *btn = lv_win_add_button(buttonWindowObj, LV_SYMBOL_CLOSE, 60);
		lv_obj_add_event_cb(btn, btnWindowObj_event_handler, LV_EVENT_CLICKED, (void *)this);
		lv_obj_t *btnok = lv_win_add_button(buttonWindowObj, LV_SYMBOL_OK, 60);
		lv_obj_add_event_cb(btnok, btnokWindowObj_event_handler, LV_EVENT_CLICKED, (void *)this);
		lv_obj_set_size(buttonWindowObj, width, height);
		lv_obj_align(buttonWindowObj, LV_ALIGN_CENTER, 0, 0);
		create_rit_control(buttonWindowObj, -20);
	}
}

guiSliderWindows::~guiSliderWindows()
{
	lv_obj_del(spinbox);
	lv_obj_del(btn_spin1);
	lv_obj_del(btn_spin2);
}

void guiSliderWindows::btnWindowObj_event_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
	if (code == LV_EVENT_CLICKED)
	{
		lv_obj_del(buttonWindowObj);
		buttonWindowObj = nullptr;
		lv_obj_send_event(Parent, customLVevents.getCustomEvent(LV_SLIDER_EVENT_CUSTOM), NULL);
	}
}
void guiSliderWindows::btnokWindowObj_event_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
	if (code == LV_EVENT_CLICKED)
	{
		lv_obj_del(buttonWindowObj);
		buttonWindowObj = nullptr;
		value = lv_spinbox_get_value(spinbox);
		lv_obj_send_event(Parent, customLVevents.getCustomEvent(LV_SLIDER_EVENT_CUSTOM_OK), (void *)(long)value);
	}
}

void guiSliderWindows::slider_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
	if (code == LV_EVENT_VALUE_CHANGED)
	{
		value = lv_spinbox_get_value(spinbox);
		lv_obj_send_event(Parent, event, (void *)(long)value);
	}
}

void guiSliderWindows::create_rit_control(lv_obj_t *parent, int dy)
{
	lv_obj_t *winAreaObject = lv_win_get_content(buttonWindowObj);
	lv_obj_update_layout(buttonWindowObj);
	lv_coord_t width = lv_obj_get_content_width(winAreaObject);
	lv_coord_t height = lv_obj_get_content_height(winAreaObject);

	spinbox = lv_spinbox_create(winAreaObject);
	lv_spinbox_set_range(spinbox, -999, 999);
	lv_spinbox_set_digit_count(spinbox, 3);
	lv_spinbox_set_dec_point_pos(spinbox, 0);
	lv_spinbox_step_prev(spinbox);
	lv_obj_set_width(spinbox, 100);
	lv_obj_center(spinbox);

	lv_spinbox_set_value(spinbox, value);

	int32_t h = lv_obj_get_height(spinbox);

	btn_spin1 = lv_button_create(winAreaObject);
	lv_obj_set_size(btn_spin1, h, h);
	lv_obj_align_to(btn_spin1, spinbox, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
	lv_obj_set_style_bg_image_src(btn_spin1, LV_SYMBOL_PLUS, 0);
	lv_obj_add_event_cb(btn_spin1, lv_spinbox_increment_event_cb, LV_EVENT_ALL, this);

	btn_spin2 = lv_button_create(winAreaObject);
	lv_obj_set_size(btn_spin2, h, h);
	lv_obj_align_to(btn_spin2, spinbox, LV_ALIGN_OUT_LEFT_MID, -5, 0);
	lv_obj_set_style_bg_image_src(btn_spin2, LV_SYMBOL_MINUS, 0);
	lv_obj_add_event_cb(btn_spin2, lv_spinbox_decrement_event_cb, LV_EVENT_ALL, this);
}