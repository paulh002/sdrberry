#include "ButtonBar.h"
#include "screen.h"
#include <cstring>

ButtonBar buttonbar;

void ButtonBar::init(lv_obj_t *parent,  int mode, lv_coord_t w, lv_coord_t h)
{
	const lv_coord_t x_margin = 2;
	const lv_coord_t y_margin = 2; // 5;
	const int x_number_buttons = 5;
	const int y_number_buttons = 1;
	const int max_rows = 1;
	const int slide_max_rows = 3;
	const lv_coord_t tab_margin = w / 3;
	const int cw_margin = 20;
	const int number_of_buttons = 4;

	int button_width_margin = ((w - tab_margin) / (x_number_buttons + 1));
	int button_width = ((w - tab_margin) / (x_number_buttons + 1)) - x_margin;
	int button_height = buttonHeight;
	int button_height_margin = button_height + y_margin;
	int ibutton_x = 0, ibutton_y = 0;
	int i = 0;
	int slider_height_margin = h / slide_max_rows - y_margin;
	int button_width_dropdown = button_width;

	button_group = lv_group_create();
	lv_style_init(&button_background);
	lv_style_set_radius(&button_background, 0);
	lv_style_set_bg_color(&button_background, lv_color_black());
	lv_style_set_border_opa(&button_background, 0);
	lv_style_set_bg_opa(&button_background, 0);
	
	buttonbar = lv_obj_create(parent);
	lv_obj_add_style(buttonbar, &button_background, 0);
	lv_obj_set_pos(buttonbar, 0, h - 2 * y_margin);
	lv_obj_set_size(buttonbar, w, button_height_margin);
	lv_obj_clear_flag(buttonbar, LV_OBJ_FLAG_SCROLLABLE);

	lv_obj_set_style_pad_top(buttonbar, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(buttonbar, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_right(buttonbar, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_left(buttonbar, 0, LV_PART_MAIN);
	
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
	button_group = lv_group_create();
	int ibuttons = number_of_buttons;
	int width = 0;
	for (int i = 0; i < ibuttons; i++)
	{
		char str[80];

		buttons[i] = lv_btn_create(buttonbar);
		lv_group_add_obj(button_group, buttons[i]);
		lv_obj_add_style(buttons[i], &style_btn, 0);
		//lv_obj_add_event_cb(buttons[i], buttons_handler, LV_EVENT_CLICKED, (void *)this);
		lv_obj_align(buttons[i], LV_ALIGN_TOP_LEFT, ibutton_x * button_width_margin, ibutton_y * button_height_margin);
		// lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
		lv_obj_set_size(buttons[i], button_width, button_height);
		width += button_width_margin;
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
	lv_obj_set_size(buttonbar, width + x_margin, button_height_margin);
	lv_obj_move_foreground(buttonbar);
	lv_obj_add_flag(buttonbar, LV_OBJ_FLAG_HIDDEN);
}

void ButtonBar::hide_buttonbar(bool enable)
{
	if (enable)
		lv_obj_add_flag(buttonbar, LV_OBJ_FLAG_HIDDEN);
	else
		lv_obj_clear_flag(buttonbar, LV_OBJ_FLAG_HIDDEN);
}