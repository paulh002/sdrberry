#include <algorithm>
#include <stdio.h>
#include "guiButtonWindows.h"
#include "CustomEvents.h"

const int windowbar = 40;


guiButtonWindows::guiButtonWindows(lv_obj_t *parent, void *thisptr, std::string name, std::vector<std::string> buttons, int selected, lv_event_code_t eventNumber, int width, int height)
	: Parent(parent), thisPtr(thisptr), eventcode(eventNumber)
{
	if (buttonWindowObj == nullptr)
	{
		buttonWindowObj = lv_win_create(lv_scr_act(), windowbar);
		lv_win_add_title(buttonWindowObj, name.c_str());
		lv_obj_t *btn = lv_win_add_btn(buttonWindowObj, LV_SYMBOL_CLOSE, 60);
		lv_obj_add_event_cb(btn, buttonWindowObj_event_handler, LV_EVENT_CLICKED, (void *)this);
		lv_obj_set_size(buttonWindowObj, width, height);
		lv_obj_align(buttonWindowObj, LV_ALIGN_CENTER, 0, 0);
		createButtons(buttons, selected);
	}
}

guiButtonWindows::~guiButtonWindows()
{
	
}

void guiButtonWindows::buttonWindowObj_event_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_CLICKED)
	{
		lv_obj_del(buttonWindowObj);
		buttonWindowObj = nullptr;
		lv_event_send(Parent, CustomEvents::getCustomEvent(LV_BUTTON_EVENT_CUSTOM), NULL);
	}
}

void guiButtonWindows::createButtons(std::vector<std::string> buttons, int selected)
{
	const lv_coord_t x_margin_dropdown = 0;
	const lv_coord_t x_margin = 2;
	const lv_coord_t y_margin = 2; //5;
	const int x_number_buttons = 4;
	const int y_number_buttons = 2;
	const int cw_margin = 20;
	const int max_col = 4;

	int max_rows = std::max(1, int(buttons.size() / x_number_buttons));
	if (buttons.size() % x_number_buttons)
		max_rows++;
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

	ibuttons = buttons.size();
	lv_obj_t *winAreaObject = lv_win_get_content(buttonWindowObj);
	lv_obj_update_layout(buttonWindowObj);
	lv_coord_t width = lv_obj_get_content_width(winAreaObject);
	lv_coord_t height = lv_obj_get_content_height(winAreaObject);

	const lv_coord_t tab_margin = width / 20;
	int button_width_margin = ((width - tab_margin) / x_number_buttons);
	int button_width = ((width - tab_margin) / x_number_buttons) - x_margin;
	int button_height = height / max_rows - y_margin - y_margin;
	int button_height_margin = button_height + y_margin;
	int ibutton_x = 0, ibutton_y = 0;
	int i = 0;
	
	for (i = 0; i < ibuttons; i++)
	{
		button[i] = lv_btn_create(winAreaObject);
		lv_obj_add_style(button[i], &style_btn, 0);
		lv_obj_add_event_cb(button[i], buttons_handler, LV_EVENT_CLICKED, (void *)this);
		lv_obj_align(button[i], LV_ALIGN_TOP_LEFT, ibutton_x * button_width_margin, y_margin + ibutton_y * button_height_margin);
		lv_obj_set_size(button[i], button_width, button_height);
		lv_group_add_obj(button_group, button[i]);

		lv_obj_t *lv_label = lv_label_create(button[i]);
		lv_label_set_text(lv_label, buttons.at(i).c_str());
		lv_obj_center(lv_label);
		if (i == selected)
			lv_obj_add_state(button[i], LV_STATE_CHECKED);
		
		ibutton_x++;
		if (ibutton_x >= x_number_buttons)
		{
			ibutton_x = 0;
			ibutton_y++;
		}
	}
}

void guiButtonWindows::buttons_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_CLICKED)
	{
		for (int i = 0; i < ibuttons; i++)
		{
			if (obj == button[i])
			{
				lv_obj_del(buttonWindowObj);
				buttonWindowObj = nullptr;
				lv_event_send(Parent, eventcode, (void *)(long)i);
				break;
			}
		}
	}
}