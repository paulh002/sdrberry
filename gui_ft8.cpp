#include "gui_ft8.h"
#include "gui_ft8bar.h"
#include "table.h"
#include "strlib.h"
#include "screen.h"
#include "WebServer.h"
#include "FT8Processor.h"
#include "vfo.h"

extern std::unique_ptr<FT8UdpClient> ft8udpclient;
gui_ft8 gft8;

std::ostream &operator<<(std::ostream &os, const struct StatusMessage &statusmessage)
{
	// Format decode_time: convert to sys_time, then to std::tm
	os << "Frequency: " << statusmessage.dialFrequency << '\n'
	   << "Mode: " << statusmessage.mode << '\n'
	   << "Call: " << statusmessage.dxCall << '\n'
	   << "Grid: " << statusmessage.dxGrid << '\n'
	   << "Report: " << statusmessage.report << '\n'
	   << "Tx: " << statusmessage.txEnabled << '\n';
	return os;
}

std::ostream &operator<<(std::ostream &os, const qso_logging &qso)
{
	auto decode_time = date::floor<std::chrono::seconds>(qso.decode_time);
	
	// Format decode_time: convert to sys_time, then to std::tm
	os << "Time: " << date::format("%F %H:%M:%S", decode_time) << '\n'
	   << "Frequency: " << qso.freq << '\n'
	   << "DX Call: " << qso.dxCall << '\n'
	   << "Message: " << qso.message << '\n'
	   << "Report Received: " << qso.report_received << '\n'
	   << "Report Sent: " << qso.report_send << '\n'
	   << "Frequency Offset: " << qso.frequency_offset << " Hz" << '\n'
	   << "dxGrid: " << qso.dxGrid << '\n';
	return os;
}

std::ostream &operator<<(std::ostream &os, const message &msg)
{
	// Format decode_time: convert to sys_time, then to std::tm
	os << msg.hh << ':'
	   << msg.min << ':'
	   << msg.sec << ' '
	   << "snr: " << msg.snr << ' '
	   << "correct_bits: " << msg.correct_bits << ' '
	   << "off: " << msg.off << ' '
	   << "hz0: " << msg.hz0 << " Hz" << '\n'
	   << "Message: " << msg.msg << '\n';
	return os;
}

bool is_report(const std::string &word) 
{
	if (word.size() == 3 && (word[0] == '+' || word[0] == '-') &&
		isdigit(word[1]) && isdigit(word[2]))
	{
		return true;
	}
	if (word.size() == 4 && word[0] == 'R' &&
		(word[1] == '+' || word[1] == '-') &&
		isdigit(word[2]) && isdigit(word[3]))
	{
		return true;
	}
	return false;
}

int which_report(const std::string msg)
{
	std::string word;

	int end_pos = msg.find_last_of(' ') +1;
	word = msg.substr(end_pos, msg.size() - end_pos);

	if (word.size() == 3 && (word[0] == '+' || word[0] == '-') &&
		isdigit(word[1]) && isdigit(word[2]))
	{
		return 3;
	}
	
	if (word.size() == 4 && word[0] == 'R' &&
		(word[1] == '+' || word[1] == '-') &&
		isdigit(word[2]) && isdigit(word[3]))
	{
		return 4;
	}
	
	if (word.size() == 3 && word[0] == 'R' && word[1] == 'R')
	{
		return 5;
	}

	if (word.size() == 2 && word[0] == '7' && word[1] == '3')
	{
		return 5;
	}
	return 2;
}

bool is_dxGrid(const std::string &word)
{
	if (word.size() != 4 && word.size() != 6)
		return false;
	if (!isupper(word[0]) || !isupper(word[1]))
		return false;
	if (!isdigit(word[2]) || !isdigit(word[3]))
		return false;
	if (word.size() == 6)
	{
		return isupper(word[4]) && isupper(word[5]);
	}
	return true;
}

void gui_ft8::qso_press_part_event_class(lv_event_t *e)
{
	
}

void gui_ft8::cq_press_part_event_class(lv_event_t *e)
{
	lv_obj_t *obj = lv_event_get_target(e);
	lv_table_t *table = (lv_table_t *)obj;
	uint16_t row, col;
	int db, length;

	lv_table_get_selected_cell(obj, &row, &col);
	if (lv_table_get_row_cnt(obj) < row+1)
		return;
	if (lv_table_get_cell_value(obj, row, 1) != nullptr)
		db = atoi(lv_table_get_cell_value(obj, row, 1));
	std::string str(lv_table_get_cell_value(obj, row, col));
	size_t i = str.find(' ');
	size_t q = str.rfind(' ');
	if (i != string::npos && q != string::npos && (q - i - 1) > 0)
	{
		std::string dxGrid;
		int start_pos_report = str.find_last_of(' ') + 1;
		std::string s = str.substr(start_pos_report, str.size());
		if (is_dxGrid(s))
		{
			dxGrid = s;
		}
		guift8bar.setMessage(str.substr(i + 1, q - i - 1), db, which_report(str), dxGrid);
		cpy_conversationtoqso();
	}
}

void gui_ft8::press_part_event_class(lv_event_t *e)
{
	lv_obj_t *obj = lv_event_get_target(e);
	lv_table_t *table = (lv_table_t *)obj;
	uint16_t row, col;
	int db, length;

	lv_table_get_selected_cell(obj, &row, &col);
	if (lv_table_get_cell_value(obj, row, 1) != nullptr)
		db = atoi(lv_table_get_cell_value(obj, row, 1));
	std::string str(lv_table_get_cell_value(obj, row, col));

	std::string::iterator new_end =
		std::unique(str.begin(), str.end(),
					[](char lhs, char rhs) { return (lhs == rhs) && (lhs == ' '); });
	str.erase(new_end, str.end());
	if (str.rfind("CQ ", 0) == 0 && guift8bar.get_status() == ft8status_t::monitor)
	{
		std::string dxGrid;
		int start_pos_report = str.find_last_of(' ') + 1;
		std::string s = str.substr(start_pos_report, str.size());
		if (is_dxGrid(s))
		{
			dxGrid = s;
		}

		int i = str.find(' ',3) - 3;
		if (i > 0)
			guift8bar.setMessage(str.substr(3, i), db, 1, dxGrid);
		else
			guift8bar.setMessage(str.substr(3), db, 1, dxGrid);
		clr_qso();
		clr_cq();
		cpy_qso(row);
		QsoScrollLatestItem();
		
		//message m{12, 1, 1, 1, 1, 1, 1000, "PA0PHH PB23AMF JO22"};
		//add_cq(m);
	}
}

void gui_ft8::qso_draw_part_event_class(lv_event_t *e)
{
	lv_obj_t *obj = lv_event_get_target(e);
	lv_table_t *table = (lv_table_t *)obj;
	lv_obj_draw_part_dsc_t *dsc = (lv_obj_draw_part_dsc_t *)lv_event_get_param(e);
	/*If the cells are drawn...*/
	if (dsc->part == LV_PART_ITEMS)
	{
		uint32_t row = dsc->id / lv_table_get_col_cnt(obj);
		uint32_t col = dsc->id - row * lv_table_get_col_cnt(obj);

		/*Make the texts in the first cell center aligned*/

		/*MAke every 2nd row grayish*/
		if (col == 3)
		{
			uint32_t cell = row * table->col_cnt + col;

			char *ptr = table->cell_data[cell]->txt;
			if (strstr(ptr, getcall().c_str()) != NULL)
			{
				dsc->rect_dsc->bg_color = lv_color_mix(lv_palette_main(LV_PALETTE_ORANGE), dsc->rect_dsc->bg_color, LV_OPA_30);
				dsc->rect_dsc->bg_opa = LV_OPA_COVER;
			}
		}
	}
}

void gui_ft8::draw_part_event_class(lv_event_t *e)
{
	lv_obj_t *obj = lv_event_get_target(e);
	lv_table_t *table = (lv_table_t *)obj;
	lv_obj_draw_part_dsc_t *dsc = (lv_obj_draw_part_dsc_t *)lv_event_get_param(e);
	/*If the cells are drawn...*/
	if (dsc->part == LV_PART_ITEMS)
	{
		uint32_t row = dsc->id / lv_table_get_col_cnt(obj);
		uint32_t col = dsc->id - row * lv_table_get_col_cnt(obj);
		if (col == 3 && row > 1)
		{
			uint32_t cell = row * table->col_cnt + col;

			char *ptr = table->cell_data[cell]->txt;
			if (strstr(ptr, "CQ ") != NULL)
			{
				dsc->rect_dsc->bg_color = lv_color_mix(lv_palette_main(LV_PALETTE_GREEN), dsc->rect_dsc->bg_color, LV_OPA_30);
				dsc->rect_dsc->bg_opa = LV_OPA_COVER;
			}
			if (strstr(ptr, getcall().c_str()) != NULL)
			{
				dsc->rect_dsc->bg_color = lv_color_mix(lv_palette_main(LV_PALETTE_ORANGE), dsc->rect_dsc->bg_color, LV_OPA_30);
				dsc->rect_dsc->bg_opa = LV_OPA_COVER;
			}
			if (strstr(ptr, guift8bar.GetFilter().c_str()) != NULL && guift8bar.GetFilter().length() > 0) 
			{
				dsc->rect_dsc->bg_color = lv_color_mix(lv_palette_main(LV_PALETTE_YELLOW), dsc->rect_dsc->bg_color, LV_OPA_30);
				dsc->rect_dsc->bg_opa = LV_OPA_COVER;
			}
		}
	}
}

void gui_ft8::init(lv_obj_t *o_tab, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h)
{

	const lv_coord_t x_margin = 10;
	const lv_coord_t y_margin = 5;
	const int x_number_buttons = 5;
	const int y_number_buttons = 4;
	const lv_coord_t tab_margin = 20;

	int button_width_margin = ((w - tab_margin) / x_number_buttons);
	int button_width = ((w - tab_margin) / x_number_buttons) - x_margin;
	int button_height = 50;
	int button_height_margin = button_height + y_margin;
	int ibutton_x = 0, ibutton_y = 0;

	qsoRowCount = 0;
	cqRowCount = 0;

	tableviewsize = Settings_file.get_int("wsjtx", "tableviewsize", 50);
	lv_style_init(&ft8_style);
	lv_style_set_radius(&ft8_style, 0);
	lv_style_set_bg_color(&ft8_style, lv_color_black());
	
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
	lv_obj_clear_flag(o_tab, LV_OBJ_FLAG_SCROLLABLE);
	//m_button_group = lv_group_create();
	lv_obj_set_style_pad_hor(o_tab, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(o_tab, 0, LV_PART_MAIN);
	
	table = lv_table_create(o_tab);
	lv_obj_add_event_cb(table, draw_part_event_cb, LV_EVENT_DRAW_PART_BEGIN, (void *)this);
	lv_obj_add_event_cb(table, press_part_event_cb, LV_EVENT_PRESSED, (void *)this);
	
	lv_obj_add_style(table, &ft8_style, 0);
	//lv_obj_align(table, LV_ALIGN_TOP_LEFT, w, h);
	lv_obj_set_pos(table, x, y);
	lv_obj_set_size(table, w/2, h);

	lv_obj_set_style_pad_top(table, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(table, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_left(table, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_right(table, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(table, 0, LV_PART_ITEMS);

	lv_obj_set_style_pad_left(table, 0, LV_PART_ITEMS);
	lv_obj_set_style_pad_right(table, 0, LV_PART_ITEMS);

	
	lv_table_set_cell_value(table, 0, 0, "Time");
	lv_table_set_col_width(table, 0, w/12);
	lv_table_set_cell_value(table, 0, 1, "db");
	lv_table_set_col_width(table, 1, w/16);
	lv_table_set_cell_value(table, 0, 2, "Freq");
	lv_table_set_col_width(table, 2, w/12);
	lv_table_set_cell_value(table, 0, 3, "Message");
	lv_table_set_col_width(table, 3, w/2 - (w / 12 + w / 16 + w / 12) - 10);

	qsoTable = lv_table_create(o_tab);
	lv_obj_add_event_cb(qsoTable, qso_draw_part_event_cb, LV_EVENT_DRAW_PART_BEGIN, (void *)this);
	lv_obj_add_event_cb(qsoTable, qso_press_part_event_cb, LV_EVENT_PRESSED, (void *)this);

	lv_obj_add_style(qsoTable, &ft8_style, 0);
	//lv_obj_align(table, LV_ALIGN_TOP_LEFT, w, h);
	lv_obj_set_pos(qsoTable, w / 2, y);
	lv_obj_set_size(qsoTable, w / 2, h / 2);

	lv_obj_set_style_pad_top(qsoTable, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(qsoTable, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_left(qsoTable, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_right(qsoTable, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(qsoTable, 0, LV_PART_ITEMS);

	lv_obj_set_style_pad_left(qsoTable, 0, LV_PART_ITEMS);
	lv_obj_set_style_pad_right(qsoTable, 0, LV_PART_ITEMS);

	lv_table_set_cell_value(qsoTable, 0, 0, "Time");
	lv_table_set_col_width(qsoTable, 0, w / 12);
	lv_table_set_cell_value(qsoTable, 0, 1, "db");
	lv_table_set_col_width(qsoTable, 1, w / 16);
	lv_table_set_cell_value(qsoTable, 0, 2, "Freq");
	lv_table_set_col_width(qsoTable, 2, w / 12);
	lv_table_set_cell_value(qsoTable, 0, 3, "Message");
	lv_table_set_col_width(qsoTable, 3, w / 2 - (w / 12 + w / 16 + w / 12) - 10);
	qsoRowCount++;

	call = Settings_file.get_string("wsjtx", "call");
	if (call.size() == 0)
		call = "PA0PHH";


	cqTable = lv_table_create(o_tab);
	// lv_obj_add_event_cb(cqTable, cq_draw_part_event_cb, LV_EVENT_DRAW_PART_BEGIN, (void *)this);
	lv_obj_add_event_cb(cqTable, cq_press_part_event_cb, LV_EVENT_PRESSED, (void *)this);

	lv_obj_add_style(cqTable, &ft8_style, 0);
	// lv_obj_align(table, LV_ALIGN_TOP_LEFT, w, h);
	lv_obj_set_pos(cqTable, w / 2, y + h / 2);
	lv_obj_set_size(cqTable, w / 2, h / 2);

	lv_obj_set_style_pad_top(cqTable, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(cqTable, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_left(cqTable, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_right(cqTable, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(cqTable, 0, LV_PART_ITEMS);

	lv_obj_set_style_pad_left(cqTable, 0, LV_PART_ITEMS);
	lv_obj_set_style_pad_right(cqTable, 0, LV_PART_ITEMS);

	lv_table_set_cell_value(cqTable, 0, 0, "Time");
	lv_table_set_col_width(cqTable, 0, w / 12);
	lv_table_set_cell_value(cqTable, 0, 1, "db");
	lv_table_set_col_width(cqTable, 1, w / 16);
	lv_table_set_cell_value(cqTable, 0, 2, "Freq");
	lv_table_set_col_width(cqTable, 2, w / 12);
	lv_table_set_cell_value(cqTable, 0, 3, "Message");
	lv_table_set_col_width(cqTable, 3, w / 2 - (w / 12 + w / 16 + w / 12) - 10);
	cqRowCount++;
	set_decode_start_time(std::chrono::system_clock::now());
	/*std::string timezone = Settings_file.get_string("Radio", "timezone");
	set_decode_start_time(make_zoned(date::current_zone(), date::floor<std::chrono::seconds>(std::chrono::system_clock::now())));
	if (timezone.size())
	{
		try
		{
			auto zone = date::locate_zone(timezone);
			set_decode_start_time(make_zoned(zone, date::floor<std::chrono::seconds>(std::chrono::system_clock::now())));
		}
		catch (const date::nonexistent_local_time &e)
		{
			std::cout << e.what() << '\n';
		}
	}
	*/
	
/*
	// DK7ZT
	message m{12, 1, 1, 1, 1, 1, 1000, "PA0PHH PB23AMF JO22"};
	add_cq(m);

	message m1{12, 1, 1, 1, 1, 1, 1000, "PA0PHH PB23AMF R-03"};
	add_cq(m1);

	message m2{12, 1, 1, 1, 1, 1, 1000, "PA0PHH PB23AMF JO22"};
	add_cq(m2);

	message m3{12, 1, 1, 1, 1, 1, 1000, "PA0PHH M0ZMF KO21"};
	add_cq(m3);

	for (int i = 0; i < 20; i++)
	{
		add_line(12, i, 1, 1, 1, 1.0, 1000, std::string("PA0PHH M0ZMF KO21"));
		add_line(12, i, 1, 1, 1, 1.0, 1000, std::string("PA0PHH M0ZMF -12"));
		add_line(12, i, 1, 1, 1, 1.0, 1000, std::string("PA0PHH M0ZMF R-12"));
		add_line(12, i, 1, 1, 1, 1.0, 1000, std::string("PA0PHH M0ZMF 73"));
	}
	for (int i = 0; i < 100; i++)
	{
		add_line(12, 1, 1, 1, 1, 1.0, 1000, std::string("PA0XXX M0ZMF KO21"));
	}
add_line(12, 1, 1, 1, 1, 1.0, 1000, std::string("PA0XXX M0ZMF KO21"));
*/
}


void gui_ft8::add_line(int hh, int min, int sec, int snr, int correct_bits, double off,int hz0, string msg)
{
	std::unique_lock<std::mutex> mlock(mutex_);

	char str[128];
	
	if (lv_table_get_row_cnt(table) > tableviewsize)
	{
		// remove first row to limit table become to large, but retain qso entries
		int row = 1;
		int size = lv_table_get_row_cnt(table);
		std::string message(lv_table_get_cell_value(table, 1, 3));		
		if (message.find(call) != std::string::npos)
		{
			do
			{
				row++;
				std::string str(lv_table_get_cell_value(table, row, 3));
				message = str;

			} while (message.find(call) != std::string::npos && row < size);
			if (row > tableviewsize / 5)
				row = 1;
		}
		lv_table_remove_rows(table, row, 1);
	}

	//std::cout << msg;
	if (msg.find(call) != std::string::npos)
	{
		qso_logging log_item, old_log_item;

		//"PA0PHH M0ZMF KO21"
		//std::cout << "PA0PHH CALL FOUND: " << msg << '\n';
		log_item.decode_time = decode_start_time;
		int start_pos_dxcall  = msg.find(' ') + 1;
		int end_pos_dxcall = msg.find_last_of(' ');
		log_item.dxCall = msg.substr(start_pos_dxcall, end_pos_dxcall - start_pos_dxcall);
		int start_pos_report = msg.find_last_of(' ') + 1;
		std::string s = msg.substr(start_pos_report, msg.size());
		if (is_report(s))
		{
			log_item.report_received = s;
		}
		if (is_dxGrid(s))
		{
			log_item.dxGrid = s;
		}
		log_item.frequency_offset = hz0;
		log_item.report_send = "R" + std::to_string(snr);
		log_item.message = msg;
		log_item.freq = vfo.get_active_vfo_freq();
		if (log_item.freq == 0) // this is for test reasons
			log_item.freq = 7074000L;

		if (qso_logging_map.find(log_item.dxCall) == qso_logging_map.end())
			qso_logging_map[log_item.dxCall] = log_item;
		else
		{
			old_log_item = qso_logging_map[log_item.dxCall];
			if (log_item.report_received.size())
				old_log_item.report_received = log_item.report_received;
			if (is_dxGrid(log_item.dxGrid))
				old_log_item.dxGrid = log_item.dxGrid;
			old_log_item.decode_time_off = decode_start_time;
			qso_logging_map[log_item.dxCall] = old_log_item;
		}
		//std::cout << qso_logging_map[log_item.dxCall] << '\n';
	}
	
	if ((msg.find(call) != std::string::npos && guift8bar.GetFilter().length() == 0) ||
		(msg.find(call) != std::string::npos && msg.find(guift8bar.GetFilter()) == std::string::npos))
	{
		message m{hh, min, sec, snr, correct_bits, off, hz0, msg};
		add_cq(m);
	}
	else
	{
		if (guift8bar.GetFilter().length() > 0)
		{
			if (msg.find(call) != std::string::npos && msg.find(guift8bar.GetFilter()) != std::string::npos && msg.find("CQ") == std::string::npos)
			{
				message m{hh, min, sec, snr, correct_bits, off, hz0, msg};
				add_qso(m);
			}

			if (msg.find(guift8bar.GetFilter()) != std::string::npos && msg.find("CQ") != std::string::npos)
			{
				message m{hh, min, sec, snr, correct_bits, off, hz0, msg};
				add_cq(m);
			}
		}
	}

	int row = lv_table_get_row_cnt(table);
	sprintf(str,"%02d:%02d:%02d", hh, min, sec);
	lv_table_set_cell_value(table, row, 0, str);
	sprintf(str,"%3d",snr);
	lv_table_set_cell_value(table, row, 1, str);
	sprintf(str, "%6d", hz0);
	lv_table_set_cell_value(table, row, 2, str);
	lv_table_set_cell_value(table, row, 3, msg.c_str());
	ScrollLatestItem();

	uint32_t in_milliseconds_since_midnight = (hh * 3600 + min * 60 + sec) * 1000;
	double deltaTime = 0.0;

	if (ft8udpclient != nullptr)
		ft8udpclient->SendDecode(true, in_milliseconds_since_midnight, (int32_t)snr, deltaTime, (uint32_t)hz0, mode, msg, false, false);
	
	message m{hh, min, sec, snr, correct_bits, off, hz0, msg};
	web_message(m);
}

void gui_ft8::add_qso(struct message msg)
{
	char str[128];

	sprintf(str, "%02d:%02d:%02d", msg.hh, msg.min, msg.sec);
	lv_table_set_cell_value(qsoTable, qsoRowCount, 0, str);

	sprintf(str, "%3d", msg.snr);
	lv_table_set_cell_value(qsoTable, qsoRowCount, 1, str);

	sprintf(str, "%6d", msg.hz0);
	lv_table_set_cell_value(qsoTable, qsoRowCount, 2, str);

	lv_table_set_cell_value(qsoTable, qsoRowCount, 3, msg.msg.c_str());

	qsoRowCount++;
	web_qso();
}

void gui_ft8::add_cq(struct message msg)
{
	char str[128];

	sprintf(str, "%02d:%02d:%02d", msg.hh, msg.min, msg.sec);
	lv_table_set_cell_value(cqTable, cqRowCount, 0, str);

	sprintf(str, "%3d", msg.snr);
	lv_table_set_cell_value(cqTable, cqRowCount, 1, str);

	sprintf(str, "%6d", msg.hz0);
	lv_table_set_cell_value(cqTable, cqRowCount, 2, str);

	lv_table_set_cell_value(cqTable, cqRowCount, 3, msg.msg.c_str());

	cqRowCount++;
	web_cq();
}


void gui_ft8::add_cq(json msg)
{
	lv_table_set_cell_value(cqTable, cqRowCount, 0, strlib::removeCharacters(msg.at("time").dump(), '"').c_str());

	lv_table_set_cell_value(cqTable, cqRowCount, 1, strlib::removeCharacters(msg.at("decibel").dump(), '"').c_str());

	lv_table_set_cell_value(cqTable, cqRowCount, 2, strlib::removeCharacters(msg.at("frequency").dump(), '"').c_str());

	lv_table_set_cell_value(cqTable, cqRowCount, 3, strlib::removeCharacters(msg.at("message").dump(), '"').c_str());

	cqRowCount++;
}

void gui_ft8::add_qso(json msg)
{
	lv_table_set_cell_value(qsoTable, qsoRowCount, 0, strlib::removeCharacters(msg.at("time").dump(), '"').c_str());

	lv_table_set_cell_value(qsoTable, qsoRowCount, 1, strlib::removeCharacters(msg.at("decibel").dump(), '"').c_str());

	lv_table_set_cell_value(qsoTable, qsoRowCount, 2, strlib::removeCharacters(msg.at("frequency").dump(), '"').c_str());

	lv_table_set_cell_value(qsoTable, qsoRowCount, 3, strlib::removeCharacters(msg.at("message").dump(), '"').c_str());

	
	qsoRowCount++;
}

void gui_ft8::cpy_qso(int row)
{

	lv_table_set_cell_value(qsoTable, qsoRowCount, 0, lv_table_get_cell_value(table, row, 0));
	lv_table_set_cell_value(qsoTable, qsoRowCount, 1, lv_table_get_cell_value(table, row, 1));
	lv_table_set_cell_value(qsoTable, qsoRowCount, 2, lv_table_get_cell_value(table, row, 2));
	lv_table_set_cell_value(qsoTable, qsoRowCount, 3, lv_table_get_cell_value(table, row, 3));
	qsoRowCount++;
	web_qso();
}

void gui_ft8::cpy_cq(int row)
{
	lv_table_set_cell_value(cqTable, cqRowCount, 0, lv_table_get_cell_value(table, row, 0));
	lv_table_set_cell_value(cqTable, cqRowCount, 1, lv_table_get_cell_value(table, row, 1));
	lv_table_set_cell_value(cqTable, cqRowCount, 2, lv_table_get_cell_value(table, row, 2));
	lv_table_set_cell_value(cqTable, cqRowCount, 3, lv_table_get_cell_value(table, row, 3));
	cqRowCount++;
	web_cq();
}

void gui_ft8::cpy_cqtoqso(int row)
{
	if (row > 0)
	{
		lv_table_set_cell_value(qsoTable, qsoRowCount, 0, lv_table_get_cell_value(cqTable, row, 0));
		lv_table_set_cell_value(qsoTable, qsoRowCount, 1, lv_table_get_cell_value(cqTable, row, 1));
		lv_table_set_cell_value(qsoTable, qsoRowCount, 2, lv_table_get_cell_value(cqTable, row, 2));
		lv_table_set_cell_value(qsoTable, qsoRowCount, 3, lv_table_get_cell_value(cqTable, row, 3));
		qsoRowCount++;
		web_qso();
	}
}

void gui_ft8::cpy_conversationtoqso()
{
	qsoRowCount = 1;
	lv_table_set_row_cnt(qsoTable, qsoRowCount);
	for (int row = 1; row < lv_table_get_row_cnt(cqTable); row++)
	{
		std::string msg(lv_table_get_cell_value(cqTable, row, 3));
		if (msg.find(call) != std::string::npos && msg.find(guift8bar.GetFilter()) != std::string::npos)
		{
			lv_table_set_cell_value(qsoTable, qsoRowCount, 0, lv_table_get_cell_value(cqTable, row, 0));
			lv_table_set_cell_value(qsoTable, qsoRowCount, 1, lv_table_get_cell_value(cqTable, row, 1));
			lv_table_set_cell_value(qsoTable, qsoRowCount, 2, lv_table_get_cell_value(cqTable, row, 2));
			lv_table_set_cell_value(qsoTable, qsoRowCount, 3, lv_table_get_cell_value(cqTable, row, 3));
			qsoRowCount++;
		}
	}
	web_qso();
	lv_obj_invalidate(qsoTable);
}

int gui_ft8::getQsoLogRows(){
	return lv_table_get_row_cnt(qsoTable);
}

std::string gui_ft8::getQso(int row)
{
	std::string s;

	s = std::string(lv_table_get_cell_value(qsoTable, row, 0));
	s += ",";
	s += std::string(lv_table_get_cell_value(qsoTable, row, 1));
	s += ",";
	s += std::string(lv_table_get_cell_value(qsoTable, row, 2));
	s += ",";
	s += std::string(lv_table_get_cell_value(qsoTable, row, 3));
	return s;
}

std::string gui_ft8::getQso_dxCall()
{
	std::string dxCall;
	
	int rows = lv_table_get_row_cnt(qsoTable);
	if (rows > 1)
	{
		std::string line = std::string(lv_table_get_cell_value(qsoTable, 1, 3));
		int start_pos_dxcall = line.find(' ') + 1;
		int end_pos_dxcall = line.find_last_of(' ');
		dxCall = line.substr(start_pos_dxcall, end_pos_dxcall - start_pos_dxcall);
	}
	return dxCall;
}

void gui_ft8::clr_qso()
{
	lv_table_set_row_cnt(qsoTable, 1);
	QsoScrollFirstItem();
	lv_obj_invalidate(table);
	web_qso();
	qsoRowCount = 1;
}

void gui_ft8::clr_cq()
{
	if (guift8bar.GetFilter().size() == 0)
	{	// remove all cq's's
		lv_table_set_row_cnt(cqTable, 1);
		CqScrollFirstItem();
		lv_obj_invalidate(table);
		cqRowCount = 1;
	}
	else
	{	// remove cq's (with call in filter) from cq window but retain all other 
		std::vector<std::string> r1, r2, r3, r4;
		for (int row = 1; row < cqRowCount; row++)
		{
			std::string rr1(lv_table_get_cell_value(cqTable, row, 0));
			std::string rr2(lv_table_get_cell_value(cqTable, row, 1));
			std::string rr3(lv_table_get_cell_value(cqTable, row, 2));
			std::string rr4(lv_table_get_cell_value(cqTable, row, 3));
			if (rr4.find(guift8bar.GetFilter()) == std::string::npos)
			{
				r1.push_back(rr1);
				r2.push_back(rr2);
				r3.push_back(rr3);
				r4.push_back(rr4);
			}
		}
		lv_table_set_row_cnt(cqTable, 1);
		cqRowCount = 1;
		for (int row = 0; row < r1.size(); row++)
		{
			lv_table_set_cell_value(cqTable, cqRowCount, 0, r1.at(row).c_str());
			lv_table_set_cell_value(cqTable, cqRowCount, 1, r2.at(row).c_str());
			lv_table_set_cell_value(cqTable, cqRowCount, 2, r3.at(row).c_str());
			lv_table_set_cell_value(cqTable, cqRowCount, 3, r4.at(row).c_str());
			cqRowCount++;
		}
		CqScrollFirstItem();
		lv_obj_invalidate(table);
	}
	web_cq();
}

void gui_ft8::set_group()
{

}

void gui_ft8::reset()
{
	lv_table_set_row_cnt(table, 1);
}

void gui_ft8::ScrollLatestItem()
{
	lv_coord_t currScrollPos = lv_obj_get_scroll_y(table);
	Scroll(table,currScrollPos);
}

void gui_ft8::ScrollFirstItem()
{
	lv_coord_t currScrollPos{};
	lv_obj_scroll_to(table, 0, currScrollPos, LV_ANIM_OFF);
}

void gui_ft8::QsoScrollLatestItem()
{
	lv_coord_t currScrollPos = lv_obj_get_scroll_y(qsoTable);
	Scroll(qsoTable, currScrollPos);
}

void gui_ft8::QsoScrollFirstItem()
{
	lv_coord_t currScrollPos{};
	lv_obj_scroll_to(qsoTable, 0, currScrollPos, LV_ANIM_OFF);
}

void gui_ft8::CqScrollLatestItem()
{
	lv_coord_t currScrollPos = lv_obj_get_scroll_y(cqTable);
	Scroll(cqTable, currScrollPos);
}

void gui_ft8::CqScrollFirstItem()
{
	lv_coord_t currScrollPos{};
	lv_obj_scroll_to(cqTable, 0, currScrollPos, LV_ANIM_OFF);
}

void gui_ft8::Scroll(lv_obj_t *table, lv_coord_t currScrollPos)
{
	lv_coord_t y = lv_obj_get_self_height(table);

	//If the object content is big enough to scroll
	if (y > lv_obj_get_height(table))
	{
		//Calculate the "out of view" y size
		lv_coord_t outOfView = y - lv_obj_get_height(table);

		if (outOfView > currScrollPos)
		{
			//Calculate the difference between the required scroll pos and the current scroll pos
			lv_coord_t differenceToScroll = -(outOfView - currScrollPos);

			//this will bring the bottom of the table into view
			lv_obj_scroll_by(table, 0, differenceToScroll, LV_ANIM_ON);
		}
	}
	return;
}

void gui_ft8::web_messages()
{
	json result = json::array();
	json message;
	int rowcount = lv_table_get_row_cnt(table);

	if (webserver.isEnabled())
	{
		for (int row = 1; row < rowcount; row++)
		{
			message.emplace("time", lv_table_get_cell_value(table, row, 0));
			message.emplace("decibel", lv_table_get_cell_value(table, row, 1));
			message.emplace("frequency", lv_table_get_cell_value(table, row, 2));
			message.emplace("message", lv_table_get_cell_value(table, row, 3));
			result.push_back(message);
			message.clear();
		}

		message.clear();
		message.emplace("type", "wsjtxmessages");
		message.emplace("data", result);
		webserver.SendMessage(message);
	}
}

void gui_ft8::web_message(message m)
{
	json message, data;
	char str[80];
	std::string s;

	if (webserver.isEnabled())
	{
		sprintf(str, "%02d:%02d:%02d", m.hh, m.min, m.sec);
		s = str;
		data.emplace("time", s);
		sprintf(str, "%3d", m.snr);
		s = str;
		data.emplace("decibel", s);
		sprintf(str, "%6d", m.hz0);
		s = str;
		data.emplace("frequency", s);
		data.emplace("message", m.msg);
		message.emplace("type", "wsjtxmessage");
		message.emplace("data", data);
		webserver.SendMessage(message);
	}
}

void gui_ft8::SelectMessage(json jsonMessage)
{
	std::unique_lock<std::mutex> mlock(mutex_);
	
	std::string message = strlib::removeCharacters(jsonMessage.at("message").dump(), '"');
	std::string dbmessage = strlib::removeCharacters(jsonMessage.at("decibel").dump(), '"');
	int db = atol(dbmessage.c_str());
	if (message.rfind("CQ ", 0) == 0 && guift8bar.get_status() == ft8status_t::monitor)
	{
		int i = message.find(' ', 3) - 3;
		if (i > 0)
			guift8bar.setMessage(message.substr(3, i), db);
		else
			guift8bar.setMessage(message.substr(3), db);
		clr_qso();
		clr_cq();
		add_qso(jsonMessage);
		QsoScrollLatestItem();
		web_qso();
	}
}

void gui_ft8::web_qso()
{
	// no lock here
	// messages to the web should not lock the GUI
	json result = json::array();
	json message;
	int rowcount = lv_table_get_row_cnt(qsoTable);

	if (webserver.isEnabled())
	{
		for (int row = 1; row < rowcount; row++)
		{
			message.emplace("time", lv_table_get_cell_value(qsoTable, row, 0));
			message.emplace("decibel", lv_table_get_cell_value(qsoTable, row, 1));
			message.emplace("frequency", lv_table_get_cell_value(qsoTable, row, 2));
			message.emplace("message", lv_table_get_cell_value(qsoTable, row, 3));
			result.push_back(message);
			message.clear();
		}
		message.clear();
		message.emplace("type", "qsomessages");
		message.emplace("data", result);
		webserver.SendMessage(message);
	}
}

void gui_ft8::web_cq()
{
	json result = json::array();
	json message;
	int rowcount = lv_table_get_row_cnt(cqTable);
	if (webserver.isEnabled())
	{

		for (int row = 1; row < rowcount; row++)
		{
			message.emplace("time", lv_table_get_cell_value(cqTable, row, 0));
			message.emplace("decibel", lv_table_get_cell_value(cqTable, row, 1));
			message.emplace("frequency", lv_table_get_cell_value(cqTable, row, 2));
			message.emplace("message", lv_table_get_cell_value(cqTable, row, 3));
			result.push_back(message);
			message.clear();
		}
		message.clear();
		message.emplace("type", "cqmessages");
		message.emplace("data", result);
		webserver.SendMessage(message);
	}
}

void gui_ft8::set_decode_start_time(std::chrono::time_point<std::chrono::system_clock> time)
{
	decode_start_time = time;
}

qso_entry gui_ft8::get_qso_entry(std::string dxCall)
{
	return qso_logging_map[dxCall];
}

void gui_ft8::delete_qso_entry(std::string dxCall)
{
	qso_logging_map.erase(dxCall);
}