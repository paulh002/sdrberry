#include "gui_bottom_bar.h"
#include "screen.h"

void gui_bottom_bar::init(lv_obj_t *o_parent, lv_group_t *button_group, int mode, lv_coord_t w, lv_coord_t h)
{
	const lv_coord_t x_margin_dropdown = 0;
	const lv_coord_t x_margin = 2;
	const lv_coord_t y_margin = 2; //5;
	const int x_number_buttons = 4;
	const int y_number_buttons = 4;
	const int max_rows = 3;
	const lv_coord_t tab_margin = w / 3;
	const int cw_margin = 20;

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
	buttongroup = button_group;

	ibuttons = number_of_buttons;
	for (i = 0; i < ibuttons; i++)
	{
		char str[80];

		if (i < number_of_buttons - 1)
		{
			button[i] = lv_btn_create(o_parent);
			lv_obj_add_style(button[i], &style_btn, 0);
			lv_obj_add_event_cb(button[i], bottonbar_button_handler, LV_EVENT_CLICKED, (void *)this);
			lv_obj_align(button[i], LV_ALIGN_TOP_LEFT, ibutton_x * button_width_margin, y_margin + ibutton_y * button_height_margin);
			lv_obj_set_size(button[i], button_width, button_height);
			lv_group_add_obj(button_group, button[i]);

			lv_obj_t *lv_label = lv_label_create(button[i]);
			switch (i)
			{
			case 0:
				lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
				lv_obj_set_user_data(button[i], NULL);
				strcpy(str, "<<");
				//if (SdrDevices.get_tx_channels(default_radio) == 0)
				//	lv_obj_add_flag(button[i], LV_OBJ_FLAG_HIDDEN);
				break;
			case 1:
				strcpy(str, "Waterfall");
				lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
				lv_obj_set_user_data(button[i], (void *)(long)i);
				//if (mode == mode_usb)
				//	lv_obj_add_state(button[i], LV_STATE_CHECKED);
				break;
			case 2:
				strcpy(str, "HOLD");
				lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
				lv_obj_set_user_data(button[i], (void *)(long)i);
				//if (mode == mode_lsb)
				//	lv_obj_add_state(button[i], LV_STATE_CHECKED);
				break;
			case 3:
				lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
				lv_obj_set_user_data(button[i], (void *)(long)i);
				strcpy(str, "Scope");
				//if (mode == mode_am)
				//	lv_obj_add_state(button[i], LV_STATE_CHECKED);
				break;
			case 4:
				strcpy(str, ">>");
				lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
				lv_obj_set_user_data(button[i], (void *)(long)i);
				//if (mode == mode_narrowband_fm)
				//	lv_obj_add_state(button[i], LV_STATE_CHECKED);
				break;
			case 5:
				strcpy(str, "CW");
				lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
				lv_obj_set_user_data(button[i], (void *)(long)i);
				//if (mode == mode_cw)
				//	lv_obj_add_state(button[i], LV_STATE_CHECKED);
				break;
			case 6:
				strcpy(str, ">>");
				lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
				lv_obj_set_user_data(button[i], (void *)(long)i);
				///if (mode == modefreedv)
				//	lv_obj_add_state(button[i], LV_STATE_CHECKED);
				//lv_obj_add_state(button[i], LV_STATE_DISABLED);
				break;
			}
			lv_label_set_text(lv_label, str);
			lv_obj_center(lv_label);
		}
	}
}
