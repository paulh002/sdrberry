#include "guiSliderWindows.h"

const int windowsliderbar = 40;
bool guiSliderWindows::initialized = false;
std::array<uint32_t, lv_custom_slider_events> guiSliderWindows::custom_event;

void guiSliderWindows::InitEvents()
{
	if (!initialized)
	{
		initialized = true;
		for (int i = 0; i < lv_custom_slider_events; i++)
		{
			custom_event[i] = lv_event_register_id();
		}
	}
}

guiSliderWindows::guiSliderWindows(lv_obj_t *parent, void *thisptr, std::string name, std::vector<std::string> buttons, lv_event_code_t eventNumber, int width, int height)
	: Parent(parent), thisPtr(thisptr), event(eventNumber)
{
	guiSliderWindows::InitEvents();
	if (buttonWindowObj == nullptr)
	{
		buttonWindowObj = lv_win_create(lv_scr_act(), windowsliderbar);
		lv_win_add_title(buttonWindowObj, name.c_str());
		lv_obj_t *btn = lv_win_add_btn(buttonWindowObj, LV_SYMBOL_CLOSE, 60);
		lv_obj_add_event_cb(btn, btnWindowObj_event_handler, LV_EVENT_CLICKED, (void *)this);
		lv_obj_set_size(buttonWindowObj, width, height);
		lv_obj_align(buttonWindowObj, LV_ALIGN_CENTER, 0, 0);
		//createSliders(sliders);
	}
}

guiSliderWindows::~guiSliderWindows()
{
}

void guiSliderWindows::btnWindowObj_event_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_CLICKED)
	{
		lv_obj_del(buttonWindowObj);
		buttonWindowObj = nullptr;
		lv_event_send(Parent, getCustomEvent(LV_SLIDER_EVENT_CUSTOM), NULL);
	}
}

void guiSliderWindows::slider_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_VALUE_CHANGED)
	{

	}
}