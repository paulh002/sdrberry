#include "gui_ft8bar.h"
#include "vfo.h"
#include "Modes.h"
#include "FT8Generator.h"

gui_ft8bar guift8bar;

gui_ft8bar::gui_ft8bar()
{
}

gui_ft8bar::~gui_ft8bar()
{
	for (int i = 0; i < ibuttons; i++)
	{
		if (button[i] != nullptr)
			lv_obj_del(button[i]);
	}
}

void gui_ft8bar::setmonitor(bool mon)
{
	if (mon)
		lv_obj_add_state(button[1], LV_STATE_CHECKED);
	else
		lv_obj_clear_state(button[1], LV_STATE_CHECKED);
}

void gui_ft8bar::setMessage(std::string callsign, int db)
{
	std::string s;

	s = callsign + " " + call + " " + locator;
	lv_table_set_cell_value(table, 1, 1, s.c_str());
	SetTxMessage(s);

	s = callsign + " " + call + " " + std::to_string(db);
	lv_table_set_cell_value(table, 2, 1, s.c_str());

	s = callsign + " " + call + " R " + std::to_string(db);
	lv_table_set_cell_value(table, 3, 1, s.c_str());
	s = callsign + " " + call + " RRR";
	lv_table_set_cell_value(table, 4, 1, s.c_str());

	s = callsign + " " + call + " 73";
	lv_table_set_cell_value(table, 5, 1, s.c_str());

	SetFilter(callsign);
}

void gui_ft8bar::SetFrequency()
{
	int selection = lv_dropdown_get_selected(frequence);
	vfo.set_vfo(Settings_file.get_ft8(selection));
}

static void textarea_event_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *ta = lv_event_get_target(e);

	if (code == LV_EVENT_CLICKED || code == LV_EVENT_FOCUSED || code == LV_EVENT_VALUE_CHANGED)
	{
		/*Focus on the clicked text area*/
		//printf("text event\n");
	}
}

static void filter_event_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *ta = lv_event_get_target(e);

	if (code == LV_EVENT_CLICKED || code == LV_EVENT_FOCUSED || code == LV_EVENT_VALUE_CHANGED)
	{
		/*Focus on the clicked text area*/
		//printf("text event\n");
	}
}


static void ft8bar_button_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);

	int i = (long)lv_obj_get_user_data(obj);

	if (code == LV_EVENT_CLICKED)
	{
		switch (i)
		{
		case 0:
			break;
		case 1:
			if (lv_obj_get_state(obj) & LV_STATE_CHECKED)
			{
				select_mode(mode_ft8);
				gbar.set_mode(mode_ft8);
			}
			else
			{
				select_mode(mode_usb);
				gbar.set_mode(mode_usb);
			}
			break;
		case 2:
			break;
		case 3:
			// Execute the QSO
			guift8bar.Transmit();
			break;
		case 4:
			//CQ
			guift8bar.SetTxMessage();
			guift8bar.SetFilterCall();
			guift8bar.Transmit();
			break;
		case 5:
			guift8bar.ClearMessage();
			break;
		}
	}
}

void gui_ft8bar::ClearMessage()
{
	lv_table_set_cell_value(table, 1, 0, "1");
	lv_table_set_cell_value(table, 1, 1, "");
	lv_table_set_cell_value(table, 2, 0, "2");
	lv_table_set_cell_value(table, 2, 1, "");
	lv_table_set_cell_value(table, 3, 0, "3");
	lv_table_set_cell_value(table, 3, 1, "");
	lv_table_set_cell_value(table, 4, 0, "4");
	lv_table_set_cell_value(table, 4, 1, "");
	lv_table_set_cell_value(table, 5, 0, "5");
	lv_table_set_cell_value(table, 5, 1, "");
	SetTxMessage();
	SetFilterCall();
}

void gui_ft8bar::SetTxMessage(std::string msg)
{
	if (msg.length() > 0)
		lv_textarea_set_text(Textfield, msg.c_str());
	else
		lv_textarea_set_text(Textfield, cq.c_str());
}

void gui_ft8bar::SetFilter(std::string msg)
{
	lv_textarea_set_text(FilterField, msg.c_str());
}

void gui_ft8bar::SetFilterCall()
{
	lv_textarea_set_text(FilterField, call.c_str());
}

std::string gui_ft8bar::GetFilter()
{
	const char *ptr = lv_textarea_get_text(FilterField);
	std::string s(ptr);
	return s;
}

static int messageToSend = 1;

static void press_part_event_cb(lv_event_t *e)
{
	lv_obj_t *obj = lv_event_get_target(e);
	lv_table_t *table = (lv_table_t *)obj;
	uint16_t row, col;
	char *ptr;
	
	lv_table_get_selected_cell(obj, &row, &col);
	if (row == 0)
		return;
	ptr = (char *)lv_table_get_cell_value(obj, row, col);
	guift8bar.SetTxMessage(std::string(ptr));
	messageToSend = row;
}


static void message_part_event_cb(lv_event_t *e)
{
	lv_obj_t *obj = lv_event_get_target(e);
	lv_table_t *table = (lv_table_t *)obj;
	lv_obj_draw_part_dsc_t *dsc = (lv_obj_draw_part_dsc_t *)lv_event_get_param(e);
	
	/*If the cells are drawn...*/
	if (dsc->part == LV_PART_ITEMS)
	{
		uint32_t row = dsc->id / lv_table_get_col_cnt(obj);
		uint32_t col = dsc->id - row * lv_table_get_col_cnt(obj);
		if (row == messageToSend)
		{
			dsc->rect_dsc->bg_color = lv_color_mix(lv_palette_main(LV_PALETTE_GREEN), dsc->rect_dsc->bg_color, LV_OPA_30);
			dsc->rect_dsc->bg_opa = LV_OPA_COVER;
		}
	}
}

static void freq_event_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	if (code == LV_EVENT_VALUE_CHANGED)
	{
		guift8bar.SetFrequency();
	}
}


static void lv_spinbox_increment_event_cb_tx(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	if (code == LV_EVENT_SHORT_CLICKED || code == LV_EVENT_LONG_PRESSED_REPEAT)
	{
		lv_spinbox_increment(guift8bar.getspintx());
	}
}

static void lv_spinbox_decrement_event_cb_tx(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	if (code == LV_EVENT_SHORT_CLICKED || code == LV_EVENT_LONG_PRESSED_REPEAT)
	{
		lv_spinbox_decrement(guift8bar.getspintx());
	}
}

static void lv_spinbox_increment_event_cb_rx(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	if (code == LV_EVENT_SHORT_CLICKED || code == LV_EVENT_LONG_PRESSED_REPEAT)
	{
		lv_spinbox_increment(guift8bar.getspinrx());
	}
}

static void lv_spinbox_decrement_event_cb_rx(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	if (code == LV_EVENT_SHORT_CLICKED || code == LV_EVENT_LONG_PRESSED_REPEAT)
	{
		lv_spinbox_decrement(guift8bar.getspinrx());
	}
}

void gui_ft8bar::init(lv_obj_t *o_parent, lv_group_t *button_group, lv_group_t *keyboard_group, int mode, lv_coord_t w, lv_coord_t h)
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

	keyboardgroup = keyboard_group;
	ft8generator = make_unique<FT8Generator>(audio_input);
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

	ibuttons = number_of_buttons;
	for (i = 0; i < ibuttons; i++)
	{
		char str[80];

		if (i < number_of_pushbuttons)
		{
			button[i] = lv_btn_create(o_parent);
			lv_obj_add_style(button[i], &style_btn, 0);
			lv_obj_add_event_cb(button[i], ft8bar_button_handler, LV_EVENT_CLICKED, NULL);
			lv_obj_align(button[i], LV_ALIGN_TOP_LEFT, ibutton_x * button_width_margin, y_margin + ibutton_y * button_height_margin);
			lv_obj_set_size(button[i], button_width, button_height);
			lv_group_add_obj(button_group, button[i]);

			lv_obj_t *lv_label = lv_label_create(button[i]);
			switch (i)
			{
			case 0:
				lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
				lv_obj_set_user_data(button[i], (void *)0);
				strcpy(str, "Stop");
				break;
			case 1:
				lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
				lv_obj_set_user_data(button[i], (void *)1);
				strcpy(str, "Monitor");
				break;
			case 2:
				lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
				lv_obj_set_user_data(button[i], (void *)2);
				strcpy(str, "Log");
				break;
			case 3:
				lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
				lv_obj_set_user_data(button[i], (void *)3);
				strcpy(str, "TX");
				txbutton = i;
				break;
			case 4:
				lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
				lv_obj_set_user_data(button[i], (void *)4);
				strcpy(str, "CQ");
				rxbutton = i;
				break;
			case 5:
				//lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
				lv_obj_set_user_data(button[i], (void *)5);
				strcpy(str, "Clear");
				break;
			}
			lv_label_set_text(lv_label, str);
			lv_obj_center(lv_label);
		}
		ibutton_x++;
		if (ibutton_x >= x_number_buttons)
		{
			ibutton_x = 0;
			ibutton_y++;
		}
	}

	button[i] = lv_spinbox_create(o_parent);
	lv_spinbox_set_range(button[i], 100, 5000);
	lv_obj_add_style(button[i], &style_btn, 0);
	lv_obj_add_event_cb(button[i], ft8bar_button_handler, LV_EVENT_CLICKED, NULL);
	lv_obj_align(button[i], LV_ALIGN_TOP_LEFT, ibutton_x * button_width_margin + button_width_margin / 2 + 8, y_margin + ibutton_y * button_height_margin);
	lv_obj_set_size(button[i], button_width , button_height);
	lv_group_add_obj(button_group, button[i]);
	lv_obj_set_style_pad_hor(button[i], 15, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(button[i], 3, LV_PART_MAIN);
	int val = Settings_file.get_int("ft8", "tx");
	lv_spinbox_set_value(button[i], val);
	ibutton_x++;

	button[i + 1] = lv_btn_create(o_parent);
	lv_obj_add_style(button[i + 1], &style_btn, 0);
	lv_obj_set_size(button[i + 1], button_width / 2 - 3, button_height);
	lv_obj_align_to(button[i + 1], button[i], LV_ALIGN_OUT_RIGHT_MID, 5, 0);
	lv_obj_set_style_bg_img_src(button[i+1], LV_SYMBOL_MINUS, 0);
	lv_obj_add_event_cb(button[i+1], lv_spinbox_decrement_event_cb_tx, LV_EVENT_ALL, NULL);
	ibutton_x++;

	button[i + 2] = lv_btn_create(o_parent);
	lv_obj_add_style(button[i + 2], &style_btn, 0);
	lv_obj_set_size(button[i + 2], button_width / 2 + 3, button_height);
	lv_obj_align_to(button[i + 2], button[i], LV_ALIGN_OUT_LEFT_MID, -5, 0);
	lv_obj_set_style_bg_img_src(button[i + 2], LV_SYMBOL_PLUS, 0);
	lv_obj_add_event_cb(button[i + 2], lv_spinbox_increment_event_cb_tx, LV_EVENT_ALL, NULL);
	ibutton_x++;

	i = i + 3;
	ibutton_x = 0;
	ibutton_y++;
	button[i+3] = lv_spinbox_create(o_parent);
	lv_spinbox_set_range(button[i + 3], 100, 5000);
	lv_obj_add_style(button[i + 3], &style_btn, 0);
	lv_obj_add_event_cb(button[i + 3], ft8bar_button_handler, LV_EVENT_CLICKED, NULL);
	lv_obj_align(button[i + 3], LV_ALIGN_TOP_LEFT, ibutton_x * button_width_margin + button_width_margin / 2 + 8, y_margin + ibutton_y * button_height_margin);
	lv_obj_set_size(button[i + 3], button_width, button_height);
	lv_group_add_obj(button_group, button[i + 3]);
	lv_obj_set_style_pad_hor(button[i + 3], 15, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(button[i + 3], 3, LV_PART_MAIN);
	val = Settings_file.get_int("ft8", "rx");
	lv_spinbox_set_value(button[i + 3], val);
	ibutton_x++;

	button[i + 4] = lv_btn_create(o_parent);
	lv_obj_add_style(button[i + 4], &style_btn, 0);
	lv_obj_set_size(button[i + 4], button_width / 2 - 3, button_height);
	lv_obj_align_to(button[i + 4], button[i + 3], LV_ALIGN_OUT_RIGHT_MID, 5, 0);
	lv_obj_set_style_bg_img_src(button[i + 4], LV_SYMBOL_MINUS, 0);
	lv_obj_add_event_cb(button[i + 4], lv_spinbox_decrement_event_cb_rx, LV_EVENT_ALL, NULL);
	ibutton_x++;

	button[i + 5] = lv_btn_create(o_parent);
	lv_obj_add_style(button[i + 5], &style_btn, 0);
	lv_obj_set_size(button[i + 5], button_width / 2 + 3, button_height);
	lv_obj_align_to(button[i + 5], button[i + 3], LV_ALIGN_OUT_LEFT_MID, -5, 0);
	lv_obj_set_style_bg_img_src(button[i + 5], LV_SYMBOL_PLUS, 0);
	lv_obj_add_event_cb(button[i + 5], lv_spinbox_increment_event_cb_rx, LV_EVENT_ALL, NULL);
	
	ibutton_y = 1;
	frequence = lv_dropdown_create(o_parent);
	lv_group_add_obj(buttonGroup, frequence);
	lv_obj_add_style(frequence, &style_btn, 0);
	lv_obj_align(frequence, LV_ALIGN_TOP_LEFT, ibutton_x * button_width_margin + button_width_margin / 2 , y_margin + ibutton_y * button_height_margin);
	lv_obj_set_size(frequence, 2 * button_width, button_height);
	lv_dropdown_clear_options(frequence);
	lv_obj_add_event_cb(frequence, freq_event_handler, LV_EVENT_VALUE_CHANGED, NULL);
	lv_obj_set_style_pad_ver(frequence, 4, LV_PART_MAIN);
	int ii = 0;
	for (auto it = begin(Settings_file.ft8_freq); it != end(Settings_file.ft8_freq); ++it)
	{
		char str[80];
		sprintf(str, "%3ld.%03ld Khz", *it / 1000, (long)((*it / 1) % 100));
		lv_dropdown_add_option(frequence, str, LV_DROPDOWN_POS_LAST);
	}
	
	lv_style_init(&ft8_style);
	lv_style_set_radius(&ft8_style, 0);
	lv_style_set_bg_color(&ft8_style, lv_color_black());

	table = lv_table_create(o_parent);
	lv_obj_add_event_cb(table, message_part_event_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);
	lv_obj_add_event_cb(table, press_part_event_cb, LV_EVENT_PRESSED, NULL);
	
	lv_obj_add_style(table, &ft8_style, 0);
	lv_obj_align(table, LV_ALIGN_TOP_RIGHT, 0, 0);
	lv_obj_set_size(table, w/3, h);

	lv_obj_set_style_pad_top(table, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(table, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_left(table, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_right(table, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(table, 1, LV_PART_ITEMS);

	lv_obj_set_style_pad_left(table, 12, LV_PART_ITEMS | LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(table, 1, LV_PART_ITEMS | LV_STATE_DEFAULT);
	
	lv_table_set_cell_value(table, 0, 0, "No");
	lv_table_set_col_width(table, 0, w / 24);
	lv_table_set_cell_value(table, 0, 1, "Message");
	lv_table_set_col_width(table, 1, w - w/12);

	ibutton_y++;
	Textfield = lv_textarea_create(o_parent);
	lv_obj_add_style(Textfield, &ft8_style, 0);
	lv_textarea_set_one_line(Textfield, true);
	lv_obj_align(Textfield, LV_ALIGN_TOP_LEFT, ibutton_x * button_width_margin + button_width_margin / 2, y_margin + ibutton_y * button_height_margin);
	lv_obj_add_event_cb(Textfield, textarea_event_handler, LV_EVENT_ALL, Textfield);
	lv_obj_add_state(Textfield, LV_STATE_FOCUSED); /*To be sure the cursor is visible*/
	lv_obj_set_size(Textfield, 3 * button_width, button_height);
	lv_obj_set_style_pad_top(Textfield, 4, LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(Textfield, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_left(Textfield, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_right(Textfield, 2, LV_PART_MAIN);
	if (keyboard_group != nullptr)
		lv_group_add_obj(keyboard_group, Textfield);
	call = Settings_file.get_string("ft8","call");
	locator = Settings_file.get_string("ft8", "locator");
	cq = "CQ " + call + " " + locator;
	lv_textarea_add_text(Textfield, cq.c_str());

	ibutton_x++;
	ibutton_x++;
	ibutton_x++;
	FilterField = lv_textarea_create(o_parent);
	lv_obj_add_style(FilterField, &ft8_style, 0);
	lv_textarea_set_one_line(FilterField, true);
	lv_obj_align(FilterField, LV_ALIGN_TOP_LEFT, ibutton_x * button_width_margin, y_margin + ibutton_y * button_height_margin);
	lv_obj_add_event_cb(FilterField, filter_event_handler, LV_EVENT_ALL, Textfield);
	lv_obj_add_state(FilterField, LV_STATE_FOCUSED); /*To be sure the cursor is visible*/
	lv_obj_set_size(FilterField, button_width, button_height);
	lv_obj_set_style_pad_top(FilterField, 4, LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(FilterField, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_left(FilterField, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_right(FilterField, 2, LV_PART_MAIN);
	if (keyboard_group != nullptr)
		lv_group_add_obj(keyboard_group, FilterField);
	
	lv_table_set_cell_value(table, 1, 0, "1");
	lv_table_set_cell_value(table, 1, 1, "");
	lv_table_set_cell_value(table, 2, 0, "2");
	lv_table_set_cell_value(table, 2, 1, "");
	lv_table_set_cell_value(table, 3, 0, "3");
	lv_table_set_cell_value(table, 3, 1, "");
	lv_table_set_cell_value(table, 4, 0, "4");
	lv_table_set_cell_value(table, 4, 1, "");
	lv_table_set_cell_value(table, 5, 0, "5");
	lv_table_set_cell_value(table, 5, 1, "");
}

void gui_ft8bar::hide(bool hide)
{
	if (hide)
	{
		lv_obj_add_flag(barview, LV_OBJ_FLAG_HIDDEN);
	}
	else
	{
		lv_dropdown_set_selected(frequence, vfo.getBandIndex(vfo.get_band_no(vfo.get_active_vfo())));
		lv_obj_clear_flag(barview, LV_OBJ_FLAG_HIDDEN);
	}
}

extern void StartDigitalTransmission(ModulatorParameters &param);

void gui_ft8bar::Transmit()
{
	int frequency;
	std::string message;

	const char *ptr = lv_textarea_get_text(Textfield);
	message = std::string(ptr);
	if (transmitting || mode != mode_ft8 || message.size() == 0)
		return;
	transmitting = true;
	frequency = lv_spinbox_get_value(getspintx());
	param.mode = mode;
	param.tone = 0;
	param.ifrate = ifrate_tx;
	param.even = true;
	param.timeslot = 15;
	param.ft8signal = ft8generator->generate(frequency, FT8, message);
	StartDigitalTransmission(param);
}

void gui_ft8bar::ClearTransmit() 
{
	lv_obj_clear_state(button[txbutton], LV_STATE_CHECKED);
	lv_obj_clear_state(button[rxbutton], LV_STATE_CHECKED);
	transmitting = false; 
}