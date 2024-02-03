#include "gui_ft8bar.h"
#include "wsjtx_lib.h"
#include "vfo.h"
#include "Modes.h"
#include <fstream>
#include <regex>
#include "wave.h"
#include "date.h"

extern const int tunerHeight;
extern const int barHeight;
extern int barHeightft8;
extern unique_ptr<wsjtx_lib> wsjtx;


/*R"(
        ^\s*                                      # optional leading spaces
        ( [A-Z]{0,2} | [A-Z][0-9] | [0-9][A-Z] )  # part 1
        ( [0-9][A-Z]{0,3} )                       # part 2
        (/R | /P)?                                # optional suffix
        \s*$                                      # optional trailing spaces
*/
bool stdCall(std::string const &w)
{
	std::regex standard_call_re{R"(^\s*([A-Z]{0,2}|[A-Z][0-9]|[0-9][A-Z])([0-9][A-Z]{0,3})(/R|/P)?\s*$)", std::regex::icase};
	return std::regex_match(w, standard_call_re);
}

gui_ft8bar guift8bar;

static int messageToSend = 1;

gui_ft8bar::gui_ft8bar()
	: ft8status{ft8status_t::idle}, rxtxmode{mode_ft8}
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

void gui_ft8bar::Process(const IQSampleVector &input)
{
	waterfall->Process(input);
}

void gui_ft8bar::DrawWaterfall()
{
	waterfall->Draw();
}

void gui_ft8bar::setmonitor(bool mon)
{
	if (mon)
		lv_obj_add_state(button[0], LV_STATE_CHECKED);
	else
		lv_obj_clear_state(button[0], LV_STATE_CHECKED);
}

void gui_ft8bar::setMessage(std::string callsign, int db, int row)
{
	std::string s73 = Settings_file.get_string("wsjtx", "73");
	std::string Message;

	bool bMyCall = stdCall(call);
	bool bHisCall = stdCall(callsign);
	bool b73 = false;

	if (s73.length() == 0 || s73 == "73")
		b73 = true;
	
	printf("Mycall: %s %s Hiscall: %s %s\n", call.c_str(), bMyCall ? "true" : "false", callsign.c_str(), bHisCall ? "true" : "false");
	
	Message = callsign + " " + call + " ";
	if (!bMyCall)
		Message = callsign + " <" + call + "> ";
	if (!bHisCall)
		Message = "<" + callsign + "> " + call + " ";
	lv_obj_invalidate(table);
	Message += locator;
	lv_table_set_cell_value(table, 1, 1, Message.c_str());
	if (row == 1)
		SetTxMessage(Message);

	Message = callsign + " " + call + " ";
	if (!bMyCall)
		Message = callsign + " <" + call + "> ";
	if (!bHisCall)
		Message = "<" + callsign + "> " + call + " ";
	if (db > 0)
		Message += "+";
	Message += std::to_string(db);
	lv_table_set_cell_value(table, 2, 1, Message.c_str());
	if (row == 2)
		SetTxMessage(Message);

	Message = callsign + " " + call + " R";
	if (!bHisCall and bMyCall)
		Message = "<" + callsign + "> " + call + " R";
	if (bHisCall and !bMyCall)
		Message = callsign + " <" + call + "> R";
	
	if (db > 0)
		Message += "+";
	Message += std::to_string(db);
	lv_table_set_cell_value(table, 3, 1, Message.c_str());
	if (row == 3)
		SetTxMessage(Message);

	if (b73)
	{
		Message = callsign + " " + call + " RRR";
		if (!bHisCall and bMyCall)
			Message = callsign + " <" + call + "> RRR";
		if (bHisCall and !bMyCall)
			Message = "<" + callsign + "> " + call + " RRR";
		lv_table_set_cell_value(table, 4, 1, Message.c_str());
	}
	else
	{
		Message = callsign + " " + call + " RR73";
		if (!bHisCall and bMyCall)
			Message = callsign + " <" + call + "> RR73";
		if (bHisCall and !bMyCall)
			Message = "<" + callsign + "> " + call + " RR73";
		lv_table_set_cell_value(table, 4, 1, Message.c_str());
	}
	if (row == 4)
		SetTxMessage(Message);

	if (b73)
	{
		Message = callsign + " " + call + " 73";
		if (!bHisCall and bMyCall)
			Message = callsign + " <" + call + "> 73";
		if (bHisCall and !bMyCall)
			Message = "<" + callsign + "> " + call + " RR73";
		lv_table_set_cell_value(table, 5, 1, Message.c_str());
		if (row == 5)
			SetTxMessage(Message);
	}
	SetFilter(callsign);
	messageToSend = row;
}

void gui_ft8bar::SetFrequency()
{
	std::vector<long> freqencies;
	
	int selection = lv_dropdown_get_selected(frequence);
	int modeselection = lv_dropdown_get_selected(guift8bar.getwsjtxmode());

	switch (modeselection)
	{
	case 0:
		Settings_file.get_array_long("wsjtx", "freqFT8", freqencies);
		vfo.set_vfo(freqencies.at(selection) * 1000L);
		break;
	case 1:
		Settings_file.get_array_long("wsjtx", "freqFT4", freqencies);
		vfo.set_vfo(freqencies.at(selection) * 1000L);
		break;
	}
}

void gui_ft8bar::textarea_event_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *ta = lv_event_get_target(e);

	if (code == LV_EVENT_CLICKED || code == LV_EVENT_FOCUSED || code == LV_EVENT_VALUE_CHANGED)
	{
		/*Focus on the clicked text area*/
		//printf("text event\n");
	}
}

void gui_ft8bar::mode_event_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *ta = lv_event_get_target(e);

	if (code == LV_EVENT_VALUE_CHANGED && !guift8bar.getmonitorstate())
	{
		std::vector<int> ftx_freq;
		
		int selection = lv_dropdown_get_selected(guift8bar.getwsjtxmode());
		if (selection == 0)
		{
			setrxtxmode(mode_ft8);
			lv_dropdown_clear_options(guift8bar.getfrequency());
			Settings_file.get_array_int("wsjtx", "freqFT8", ftx_freq);
			for (auto it = begin(ftx_freq); it != end(ftx_freq); ++it)
			{
				char str[80];
				sprintf(str, "%3ld.%03ld Khz", *it / 1000, (long)((*it / 1) % 100));
				lv_dropdown_add_option(guift8bar.getfrequency(), str, LV_DROPDOWN_POS_LAST);
			}
		}
		if (selection == 1)
		{
			setrxtxmode(mode_ft4);
			lv_dropdown_clear_options(guift8bar.getfrequency());
			Settings_file.get_array_int("wsjtx", "freqFT4", ftx_freq);
			for (auto it = begin(ftx_freq); it != end(ftx_freq); ++it)
			{
				char str[80];
				sprintf(str, "%3ld.%03ld Khz", *it / 1000, (long)((*it / 1) % 100));
				lv_dropdown_add_option(guift8bar.getfrequency(), str, LV_DROPDOWN_POS_LAST);
			}
		}
		
	}
}

void gui_ft8bar::filter_event_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *ta = lv_event_get_target(e);

	if (code == LV_EVENT_CLICKED || code == LV_EVENT_FOCUSED || code == LV_EVENT_VALUE_CHANGED)
	{
		/*Focus on the clicked text area*/
		//printf("text event\n");
	}
}

void gui_ft8bar::ft8bar_button_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);

	int i = (long)lv_obj_get_user_data(obj);

	if (code == LV_EVENT_CLICKED)
	{
		switch (i)
		{
		case 0:
			if (lv_obj_get_state(obj) & LV_STATE_CHECKED)
			{
				SetFrequency();
				select_mode(guift8bar.getrxtxmode());
				gbar.set_mode(mode_usb);
				setmodeclickable(false);
				ft8status = ft8status_t::monitor;
			}
			else
			{
				setmodeclickable(true);
				select_mode(vfo.get_current_mode());
				gbar.set_mode(vfo.get_current_mode());
				ft8status = ft8status_t::idle;
			}
			break;
		case 1:
			// log
			{
				std::ofstream outfile;

				ft8status = ft8status_t::monitor;
				outfile.open("/home/pi/qso-log.csv", std::ios::out | std::ios::app);
				if (!outfile.fail())
				{
					auto today = date::year_month_weekday{ date::floor<date::days>(std::chrono::high_resolution_clock::now()) };
					outfile << today << ",";
					int rows = gft8.getQsoLogRows();
					for (int i = 0; i < rows; i++)
					{
						std::string line = gft8.getQso(i);
						outfile << line << std::endl;
					}
					outfile.close();
				}
				ClearMessage();
			}
			break;
		case 2:
			// Execute the QSO
			if (Transmit(obj))
				ft8status = ft8status_t::monitor;
			else
				ft8status = ft8status_t::response;
			break;
		case 3:
			//CQ
			SetTxMessage();
			//guift8bar.SetFilterCall();
			if (Transmit(obj))
				ft8status = ft8status_t::monitor;
			else
				ft8status = ft8status_t::cq;
			break;
		case 4:
			ft8status = ft8status_t::monitor;
			ClearMessage();
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
	messageToSend = 1;
	SetTxMessage();
	SetFilter("");
	gft8.clr_qso();
	gft8.clr_cq();
}

void gui_ft8bar::setmodeclickable(bool clickable)
{
	if (clickable)
		lv_obj_add_flag(wsjtxmode, LV_OBJ_FLAG_CLICKABLE);
	else
		lv_obj_clear_flag(wsjtxmode, LV_OBJ_FLAG_CLICKABLE);
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

void gui_ft8bar::tx_slider_event_cb_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *slider = lv_event_get_target(e);

	if (code == LV_EVENT_VALUE_CHANGED)
	{
		lv_obj_t *tx_slider_label = (lv_obj_t *)lv_event_get_user_data(e);
		lv_label_set_text_fmt(tx_slider_label, "tx %d Hz", 50 * lv_slider_get_value(slider));
		Settings_file.save_int("wsjtx", "tx", lv_slider_get_value(slider) * 50);
	}
}

void gui_ft8bar::if_slider_event_cb_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *slider = lv_event_get_target(e);

	if (code == LV_EVENT_VALUE_CHANGED)
	{
		lv_obj_t *if_slider_label = (lv_obj_t *)lv_event_get_user_data(e);
		lv_label_set_text_fmt(if_slider_label, "if %d db", lv_slider_get_value(slider));
		gbar.set_if(lv_slider_get_value(slider));
	}
}

void gui_ft8bar::set_if(int ifgain)
{
	lv_slider_set_value(if_slider, ifgain, LV_ANIM_ON);
	lv_label_set_text_fmt(if_slider_label, "if %d db", ifgain);
}

void gui_ft8bar::init(lv_obj_t *o_parent, lv_group_t *button_group, lv_group_t *keyboard_group, int mode, lv_coord_t w, lv_coord_t h)
{
	const lv_coord_t x_margin_dropdown = 0;
	const lv_coord_t x_margin = 2;
	const lv_coord_t y_margin = 2; //5;
	const int x_number_buttons = 5;
	const int y_number_buttons = 4;
	const int max_rows = 4;
	const lv_coord_t tab_margin = w / 3;
	const int cw_margin = 20;
	const int number_of_pushbuttons = 5;

	int button_width_margin = ((w - tab_margin) / (x_number_buttons + 1));
	int button_width = ((w - tab_margin) / (x_number_buttons + 1)) - x_margin;
	int button_height = h / max_rows - y_margin - y_margin;
	int button_height_margin = button_height + y_margin;
	int ibutton_x = 0, ibutton_y = 0;
	int i = 0;

	keyboardgroup = keyboard_group;
	
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
			lv_obj_add_event_cb(button[i], ft8bar_button_handler, LV_EVENT_CLICKED, (void *)this);
			lv_obj_align(button[i], LV_ALIGN_TOP_LEFT, ibutton_x * button_width_margin, y_margin + ibutton_y * button_height_margin);
			lv_obj_set_size(button[i], button_width, button_height);
			lv_group_add_obj(button_group, button[i]);

			lv_obj_t *lv_label = lv_label_create(button[i]);
			switch (i)
			{
			case 0:
				lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
				lv_obj_set_user_data(button[i], (void *)0);
				strcpy(str, "Monitor");
				break;
			case 1:
				lv_obj_set_user_data(button[i], (void *)1);
				strcpy(str, "Log");
				break;
			case 2:
				lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
				lv_obj_set_user_data(button[i], (void *)2);
				strcpy(str, "TX");
				txbutton = i;
				break;
			case 3:
				lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
				lv_obj_set_user_data(button[i], (void *)3);
				strcpy(str, "CQ");
				rxbutton = i;
				break;
			case 4:
				//lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
				lv_obj_set_user_data(button[i], (void *)4);
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
	
	ibutton_x = 5;
	ibutton_y = 0;
	wsjtxmode = lv_dropdown_create(o_parent);
	lv_group_add_obj(buttonGroup, wsjtxmode);
	lv_obj_add_style(wsjtxmode, &style_btn, 0);
	lv_obj_align(wsjtxmode, LV_ALIGN_TOP_LEFT, ibutton_x * button_width_margin, y_margin + ibutton_y * button_height_margin);
	lv_obj_set_size(wsjtxmode, button_width, button_height);
	lv_dropdown_clear_options(wsjtxmode);
	lv_obj_add_event_cb(wsjtxmode, mode_event_handler, LV_EVENT_VALUE_CHANGED, (void *)this);
	lv_obj_set_style_pad_ver(wsjtxmode, 4, LV_PART_MAIN);
	lv_dropdown_add_option(wsjtxmode, "FT8", LV_DROPDOWN_POS_LAST);
	lv_dropdown_add_option(wsjtxmode, "FT4", LV_DROPDOWN_POS_LAST);
	lv_dropdown_add_option(wsjtxmode, "FTST4", LV_DROPDOWN_POS_LAST);
	lv_dropdown_add_option(wsjtxmode, "WSPR", LV_DROPDOWN_POS_LAST);

	ibutton_x = 0;
	ibutton_y = 1;
	frequence = lv_dropdown_create(o_parent);
	lv_group_add_obj(buttonGroup, frequence);
	lv_obj_add_style(frequence, &style_btn, 0);
	lv_obj_align(frequence, LV_ALIGN_TOP_LEFT, ibutton_x * button_width_margin , y_margin + ibutton_y * button_height_margin);
	lv_obj_set_size(frequence, 2 * button_width, button_height);
	lv_dropdown_clear_options(frequence);
	lv_obj_add_event_cb(frequence, freq_event_handler, LV_EVENT_VALUE_CHANGED, (void *)this);
	lv_obj_set_style_pad_ver(frequence, 4, LV_PART_MAIN);
	int ii = 0;

	std::vector<int> ftx_freq;
	Settings_file.get_array_int("wsjtx", "freqFT8", ftx_freq);
	for (auto it = begin(ftx_freq); it != end(ftx_freq); ++it)
	{
		char str[80];
		sprintf(str, "%3ld.%03ld Khz", *it / 1000, (long)((*it / 1) % 100));
		lv_dropdown_add_option(frequence, str, LV_DROPDOWN_POS_LAST);
	}
	ibutton_x++;
	
	lv_style_init(&ft8_style);
	lv_style_set_radius(&ft8_style, 0);
	lv_style_set_bg_color(&ft8_style, lv_color_black());

	table = lv_table_create(o_parent);
	lv_obj_add_event_cb(table, message_part_event_cb, LV_EVENT_DRAW_PART_BEGIN, (void *)this);
	lv_obj_add_event_cb(table, press_part_event_cb, LV_EVENT_PRESSED, (void *)this);
	
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

	ibutton_x++;
	Textfield = lv_textarea_create(o_parent);
	lv_obj_add_style(Textfield, &ft8_style, 0);
	lv_textarea_set_one_line(Textfield, true);
	lv_obj_align(Textfield, LV_ALIGN_TOP_LEFT, ibutton_x * button_width_margin , y_margin + ibutton_y * button_height_margin);
	lv_obj_add_event_cb(Textfield, textarea_event_handler, LV_EVENT_ALL, Textfield);
	lv_obj_add_state(Textfield, LV_STATE_FOCUSED); /*To be sure the cursor is visible*/
	lv_obj_set_size(Textfield, 3 * button_width, button_height);
	lv_obj_set_style_pad_top(Textfield, 4, LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(Textfield, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_left(Textfield, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_right(Textfield, 2, LV_PART_MAIN);
	if (keyboard_group != nullptr)
		lv_group_add_obj(keyboard_group, Textfield);
	call = Settings_file.get_string("wsjtx","call");
	locator = Settings_file.get_string("wsjtx", "locator");
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

	ibutton_y++;
	int if_width = ibutton_x * button_width_margin - 20; //	-30;
	int if_y = button_height / 4 + 2 * y_margin + ibutton_y * button_height_margin;
	if_slider_label = lv_label_create(o_parent);
	lv_obj_align(if_slider_label, LV_ALIGN_TOP_LEFT, if_width + 15, if_y);
	if_slider = lv_slider_create(o_parent);
	lv_slider_set_range(if_slider, 0, maxifgain);
	lv_obj_set_width(if_slider, if_width);
	lv_obj_align(if_slider, LV_ALIGN_TOP_LEFT, 10, if_y);
	lv_obj_add_event_cb(if_slider, if_slider_event_cb, LV_EVENT_VALUE_CHANGED, (void *)if_slider_label);

	ibutton_y++;
	int freq = Settings_file.get_int("wsjtx", "tx", 0);
	int tx_width = ibutton_x * button_width_margin - 20; //	-30;
	int tx_y = button_height / 4 + 2 * y_margin + ibutton_y * button_height_margin;
	tx_slider_label = lv_label_create(o_parent);
	lv_label_set_text_fmt(tx_slider_label, "tx %d Hz", freq);
	lv_obj_align(tx_slider_label, LV_ALIGN_TOP_LEFT, tx_width + 15, tx_y);
	tx_slider = lv_slider_create(o_parent);
	lv_slider_set_range(tx_slider, 0, 80);
	lv_obj_set_width(tx_slider, tx_width);
	lv_obj_align(tx_slider, LV_ALIGN_TOP_LEFT, 10, tx_y);
	lv_obj_add_event_cb(tx_slider, tx_slider_event_cb, LV_EVENT_VALUE_CHANGED, (void *)tx_slider_label);
	lv_slider_set_value(tx_slider, freq / 50, LV_ANIM_ON);
	
	
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
	
	float bandwidth = Settings_file.get_int("wsjtx", "bandwidth", 4500);
	int waterfallfloor = Settings_file.get_int("wsjtx", "waterfallfloor", 60);
	float resampleRate = bandwidth / ft8_rate;
	waterfall = std::make_unique<Waterfall>(o_parent, 0, barHeightft8, w, tunerHeight, resampleRate, waterfallfloor, down, allparts);
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

int gui_ft8bar::Transmit(lv_obj_t *obj)
{
	wsjtxMode wstx_mode;
	ModulatorParameters param;
	int frequency;
	std::string message;
	int retval = 0; // 1 == no tx action

	const char *ptr = lv_textarea_get_text(Textfield);
	message = std::string(ptr);
	if (mode == mode_ft8 || mode == mode_ft4 || mode == mode_wspr)
	{
		if (transmitting || message.size() == 0)
		{
			if (transmitting)
			{
				printf("Cancel tx mode\n");
				if (DigitalTransmission::CancelDigitalTransmission())
				{
					printf("tx mode canceld\n");
					transmitting = false;
					retval = 1;
				}
				else
				{
					lv_obj_add_state(obj, LV_STATE_CHECKED);
					printf("Cannot cancel tx mode\n");
				}
			}
		}
		else
		{
			printf("wsjtx message %s\n", message.c_str());
			transmitting = true;
			frequency = lv_slider_get_value(tx_slider) * 50;
			param.tone = audioTone::NoTone;
			param.ifrate = ifrate_tx;
			param.even = true;
			param.rxChannel = gsetup.get_current_rx_channel();
			param.txChannel = gsetup.get_current_tx_channel();
			int selection = lv_dropdown_get_selected(wsjtxmode);
			switch (selection)
			{
			case 0:
				wstx_mode = wsjtxMode::FT8;
				param.timeslotTensofSec = 150;
				param.mode = mode_ft8;
				break;
			case 1:
				wstx_mode = wsjtxMode::FT4;
				param.timeslotTensofSec = 75;
				param.mode = mode_ft4;
				break;
			case 2:
				break;
			}
			param.signal = wsjtx->encode(wstx_mode, frequency, message, msgsend);
			save_wav(param.signal.data(), (int)param.signal.size(), 48000, "./wave.wav");
			DigitalTransmission::StartDigitalTransmission(std::move(param));
		}
	}
	else
	{	
		retval = 1;
	}
	return retval;
}

void gui_ft8bar::ClearTransmit() 
{
	lv_obj_clear_state(button[txbutton], LV_STATE_CHECKED);
	lv_obj_clear_state(button[rxbutton], LV_STATE_CHECKED);
	transmitting = false; 
}
