#include "guiSliderWindows.h"
#include "CustomEvents.h"

LV_IMG_DECLARE(ui_img_btn_knob_png); // assets/btn_knob.png

const int windowsliderbar = 40;

guiSliderWindows::guiSliderWindows(lv_obj_t *parent, void *thisptr, std::string name, std::vector<std::string> buttons, int val, lv_event_code_t eventNumber, int width, int height)
	: Parent(parent), thisPtr(thisptr), event(eventNumber), value(val)
{
	if (buttonWindowObj == nullptr)
	{
		buttonWindowObj = lv_win_create(lv_scr_act(), windowsliderbar);
		lv_win_add_title(buttonWindowObj, name.c_str());
		lv_obj_t *btn = lv_win_add_btn(buttonWindowObj, LV_SYMBOL_CLOSE, 60);
		lv_obj_add_event_cb(btn, btnWindowObj_event_handler, LV_EVENT_CLICKED, (void *)this);
		lv_obj_t *btnok = lv_win_add_btn(buttonWindowObj, LV_SYMBOL_OK, 60);
		lv_obj_add_event_cb(btnok, btnokWindowObj_event_handler, LV_EVENT_CLICKED, (void *)this);
		lv_obj_set_size(buttonWindowObj, width, height);
		lv_obj_align(buttonWindowObj, LV_ALIGN_CENTER, 0, 0);
		create_rotary_button(buttonWindowObj, -20);
	}
}

guiSliderWindows::~guiSliderWindows()
{
	//lv_obj_del(ui_Image1);
	//lv_obj_del(ui_Arc1);
}

void guiSliderWindows::btnWindowObj_event_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_CLICKED)
	{
		lv_obj_del(buttonWindowObj);
		buttonWindowObj = nullptr;
		lv_event_send(Parent, customLVevents.getCustomEvent(LV_SLIDER_EVENT_CUSTOM), NULL);
	}
}
void guiSliderWindows::btnokWindowObj_event_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_CLICKED)
	{
		lv_obj_del(buttonWindowObj);
		buttonWindowObj = nullptr;
		lv_event_send(Parent, customLVevents.getCustomEvent(LV_SLIDER_EVENT_CUSTOM_OK), NULL);
	}
}

void guiSliderWindows::slider_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_VALUE_CHANGED)
	{
		char buf[20];
		
		value = lv_arc_get_value(ui_Arc1);
		lv_event_send(Parent, event, (void *)(long)value);
		sprintf(buf, "%d Hz", value);
		lv_label_set_text(label, buf);
	}
}

void guiSliderWindows::create_rotary_button(lv_obj_t *parent, int dy)
{
	lv_obj_t *winAreaObject = lv_win_get_content(buttonWindowObj);
	lv_obj_update_layout(buttonWindowObj);
	lv_coord_t width = lv_obj_get_content_width(winAreaObject);
	lv_coord_t height = lv_obj_get_content_height(winAreaObject);

	ui_Image1 = lv_img_create(winAreaObject);
	lv_img_set_src(ui_Image1, &ui_img_btn_knob_png);
	lv_obj_set_width(ui_Image1, 104);
	lv_obj_set_height(ui_Image1, 105);
	lv_obj_set_x(ui_Image1, 0);
	lv_obj_set_y(ui_Image1, 0);

	lv_obj_align_to(ui_Image1, winAreaObject, LV_ALIGN_CENTER, 0, dy);
	lv_obj_add_flag(ui_Image1, LV_OBJ_FLAG_ADV_HITTEST); /// Flags

	ui_Arc1 = lv_arc_create(winAreaObject);
	lv_obj_set_width(ui_Arc1, 95);
	lv_obj_set_height(ui_Arc1, 97);
	lv_obj_set_x(ui_Arc1, 0);
	lv_obj_set_y(ui_Arc1, 0);
	lv_obj_align_to(ui_Arc1, winAreaObject, LV_ALIGN_CENTER, 0, dy);
	//lv_obj_set_align(ui_Arc1, LV_ALIGN_CENTER);
	lv_arc_set_range(ui_Arc1, -500, 500);
	lv_arc_set_value(ui_Arc1, value);
	lv_arc_set_bg_angles(ui_Arc1, 129, 51);
	lv_obj_set_style_pad_left(ui_Arc1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui_Arc1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui_Arc1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui_Arc1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_arc_color(ui_Arc1, lv_color_hex(0x4040FF), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_arc_opa(ui_Arc1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_arc_width(ui_Arc1, 20, LV_PART_MAIN | LV_STATE_DEFAULT);

	lv_obj_set_style_arc_color(ui_Arc1, lv_color_hex(0x50FF7D), LV_PART_INDICATOR | LV_STATE_DEFAULT);
	lv_obj_set_style_arc_opa(ui_Arc1, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
	lv_obj_set_style_arc_width(ui_Arc1, 2, LV_PART_INDICATOR | LV_STATE_DEFAULT);
	lv_obj_set_style_arc_rounded(ui_Arc1, true, LV_PART_INDICATOR | LV_STATE_DEFAULT);

	lv_obj_set_style_bg_color(ui_Arc1, lv_color_hex(0x50FF7D), LV_PART_KNOB | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_Arc1, 0, LV_PART_KNOB | LV_STATE_DEFAULT);

	lv_arc_set_mode(ui_Arc1, LV_ARC_MODE_SYMMETRICAL);
	lv_obj_add_event_cb(ui_Arc1, slider_handler, LV_EVENT_VALUE_CHANGED, (void *)this);

	label = lv_label_create(winAreaObject);
	lv_obj_align_to(label, ui_Arc1,LV_ALIGN_BOTTOM_MID, 0, 30);
	
	char buf[20];
	sprintf(buf, "%d Hz", value);
	lv_label_set_text(label, buf);
}