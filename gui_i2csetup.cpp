#include <vector>
#include <string>

#include "gui_i2csetup.h"
#include "Settings.h"
#include "strlib.h"
#include "HexKeyboardWindow.h"

gui_i2csetup i2csetup;
const char *None_opts = "";
const char *pcf8475_opts = "20\n21\n22\n23\n24\n25\n26\n27";
const char *pcf8475a_opts = "38\n39\n3A\n3B\n3C\n3D\n3E\n3F";
const char *mcp22008_opts = "20\n21\n22\n23\n24\n25\n26\n27";
const char *tca9548_opts = "70\n71\n72\n73\n74\n75\n76\n77";
std::vector<std::string> devices{"NONE", "PCF8574", "PCF8574A", "MCP23008", "TCA9548"};


void gui_i2csetup::editButton_handler_class(lv_event_t *e)
{
	lv_obj_t *obj = lv_event_get_target(e);
	lv_table_t *table = (lv_table_t *)obj;

	CreateHexKeyboardWindow(lv_scr_act());
	
}

void gui_i2csetup::delButton_handler_class(lv_event_t *e)
{
	lv_obj_t *obj = lv_event_get_target(e);
	lv_table_t *table = (lv_table_t *)obj;

	DelDevice();
	DelLine();
}

void gui_i2csetup::addButton_handler_class(lv_event_t *e)
{
	lv_obj_t *obj = lv_event_get_target(e);
	lv_table_t *table = (lv_table_t *)obj;

	AddDevice("NONE", "00");
	AddLine("00", "00");
}

void gui_i2csetup::bandtable_press_part_event_cb_class(lv_event_t *e)
{
	lv_obj_t *obj = lv_event_get_target(e);
	lv_table_t *table = (lv_table_t *)obj;
	uint16_t row, col;
	
	// Do something
	lv_table_get_selected_cell(obj, &row, &col);
	if (lv_table_get_row_cnt(obj) < (row + 1) || row == 0)
		return;
	setBandRowSelected(row);
}

void gui_i2csetup::table_press_part_event_cb_class(lv_event_t *e)
{
	lv_obj_t *obj = lv_event_get_target(e);
	lv_table_t *table = (lv_table_t *)obj;
	uint16_t row, col;
	char *ptr;
	std::string buffer;

	lv_table_get_selected_cell(obj, &row, &col);
	if (lv_table_get_row_cnt(obj) < (row + 1) || row == 0)
		return;
	ptr = (char *)lv_table_get_cell_value(obj, row, 1);
	buffer.resize(80);
	strcpy(buffer.data(), ptr);
	buffer.resize(strlen(buffer.data()));
	buffer = strlib::toUpper(buffer);
	setDevice(buffer);
	setAdressList(buffer);
	ptr = (char *)lv_table_get_cell_value(obj, row, 2);
	buffer.resize(80);
	strcpy(buffer.data(), ptr);
	buffer.resize(strlen(buffer.data()));
	buffer = strlib::toUpper(buffer);
	setAdress(buffer);
	setRowSelected(row);
}

void gui_i2csetup::table_draw_part_event_cb_class(lv_event_t *e)
{
	lv_obj_t *obj = lv_event_get_target(e);
	lv_table_t *table = (lv_table_t *)obj;
	lv_obj_draw_part_dsc_t *dsc = (lv_obj_draw_part_dsc_t *)lv_event_get_param(e);
	/*If the cells are drawn...*/
	if (dsc->part == LV_PART_ITEMS)
	{
		uint32_t row = dsc->id / lv_table_get_col_cnt(obj);
		uint32_t col = dsc->id - row * lv_table_get_col_cnt(obj);
		if (row == i2csetup.getRowSelected())
		{
			dsc->rect_dsc->bg_color = lv_color_mix(lv_palette_main(LV_PALETTE_ORANGE), dsc->rect_dsc->bg_color, LV_OPA_30);
			dsc->rect_dsc->bg_opa = LV_OPA_COVER;
		}
	}
}

void gui_i2csetup::bandtable_draw_part_event_cb_class(lv_event_t *e)
{
	lv_obj_t *obj = lv_event_get_target(e);
	lv_table_t *table = (lv_table_t *)obj;
	lv_obj_draw_part_dsc_t *dsc = (lv_obj_draw_part_dsc_t *)lv_event_get_param(e);
	/*If the cells are drawn...*/
	if (dsc->part == LV_PART_ITEMS)
	{
		uint32_t row = dsc->id / lv_table_get_col_cnt(obj);
		uint32_t col = dsc->id - row * lv_table_get_col_cnt(obj);
		if (row == i2csetup.getBandRowSelected())
		{
			dsc->rect_dsc->bg_color = lv_color_mix(lv_palette_main(LV_PALETTE_ORANGE), dsc->rect_dsc->bg_color, LV_OPA_30);
			dsc->rect_dsc->bg_opa = LV_OPA_COVER;
		}
	}
}

void gui_i2csetup::bandDropdownHandler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_VALUE_CHANGED)
	{
		setBandRowSelected(-1);
		setBand(lv_dropdown_get_selected(obj));
	}
}

void gui_i2csetup::deviceDropdownHandler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_VALUE_CHANGED)
	{
		// if row selected set device
		std::string buffer;
		buffer.resize(80);
		int selection = lv_dropdown_get_selected(obj);
		lv_dropdown_get_selected_str(obj, buffer.data(), 79);
		buffer.resize(strlen(buffer.data()));
		i2csetup.setAdressList(buffer);
		i2csetup.updateListDevice(buffer);
	}
}

void gui_i2csetup::AddressHandler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	std::string buf;

	buf.resize(80);
	if (code == LV_EVENT_VALUE_CHANGED)
	{
		lv_roller_get_selected_str(GetaddressObj(), buf.data(), 79);
		buf.resize(strlen(buf.data()));
		buf = strlib::toUpper(buf);
		i2csetup.updateListAddress(buf);
	}
}

void gui_i2csetup::updateListAddress(std::string buf)
{
	if (rowSelected > 0)
		lv_table_set_cell_value(table, rowSelected, 2, buf.c_str());
}

void gui_i2csetup::updateListDevice(std::string buf)
{
	std::string buf1;
	if (rowSelected > 0)
	{
		buf1 = strlib::toUpper(buf);
		lv_table_set_cell_value(table, rowSelected, 1, buf1.c_str());
		if (buf1 == "NONE")
			updateListAddress("00");
	}
}

void gui_i2csetup::setAdress(std::string buffer)
{
	std::string buf1 = strlib::toUpper(buffer);
	std::string buf;

	buf.resize(80);
	int count = lv_roller_get_option_cnt(addressObj);
	for (int i=0; i < count; i++)
	{
		lv_roller_set_selected(addressObj, i, LV_ANIM_ON);
		lv_roller_get_selected_str(addressObj, buf.data(), 79);
		buf.resize(strlen(buf.data()));
		buf = strlib::toUpper(buf);
		if (buf == buf1)
			break;
	}
}

void gui_i2csetup::setAdressList(std::string buffer)
{
	std::string buf = strlib::toUpper(buffer);

	if (buf == "NONE")
		lv_roller_set_options(addressObj, None_opts, LV_ROLLER_MODE_NORMAL);
	if (buf == "PCF8574")
		lv_roller_set_options(addressObj, pcf8475_opts, LV_ROLLER_MODE_NORMAL);
	if (buf == "PCF8574A")
		lv_roller_set_options(addressObj, pcf8475a_opts, LV_ROLLER_MODE_NORMAL);
	if (buf == "MCP23008")
		lv_roller_set_options(addressObj, mcp22008_opts, LV_ROLLER_MODE_NORMAL);
	if (buf == "TCA9548")
		lv_roller_set_options(addressObj,tca9548_opts, LV_ROLLER_MODE_NORMAL);
}

void gui_i2csetup::setDevice(std::string buffer)
{
	std::string buf1 = strlib::toUpper(buffer);

	int i = 0;
	for (auto col : devices)
	{
		std::string buf = strlib::toUpper(col);
		if (buf == buf1)
			lv_dropdown_set_selected(deviceDropdown, i);
		i++;
	}
}

void gui_i2csetup::msg_txtmessage_handler_class(void *e, lv_msg_t *m)
{
	std::string payload((const char *)lv_msg_get_payload((lv_msg_t *)m));
	std::string byteA;
	std::string byteB;

	if (payload.size() == 5)
	{
		int pos = payload.find(',');
		byteA = payload.substr(0, pos);
		byteB = payload.substr(pos + 1, payload.size());
		EditLine(byteA, byteB);
	}
}

void gui_i2csetup::AddLine(std::string byteA, std::string byteB)
{
	
	int i = lv_table_get_row_cnt(bandtable);
	lv_table_set_cell_value(bandtable, i, 0, byteA.c_str());
	lv_table_set_cell_value(bandtable, i, 1, byteB.c_str());
}

void gui_i2csetup::DelLine()
{
	int i = lv_table_get_row_cnt(bandtable);
	if (i > 1)
		lv_table_set_row_cnt(bandtable, i - 1);
}

void gui_i2csetup::AddDevice(std::string dev, std::string add)
{
	char str[80];
	
	int i = lv_table_get_row_cnt(table);
	sprintf(str, "%02d", i);
	lv_table_set_cell_value(table, i, 0, str);
	lv_table_set_cell_value(table, i, 1, dev.c_str());
	lv_table_set_cell_value(table, i, 2, add.c_str());
}

void gui_i2csetup::DelDevice()
{
	int i = lv_table_get_row_cnt(table);
	if (i > 1)
		lv_table_set_row_cnt(table, i-1);
}

void gui_i2csetup::EditLine(std::string byteA, std::string byteB)
{
	if (bandRowSelected >= 0)
	{
		lv_table_set_cell_value(bandtable, bandRowSelected, 0, byteA.c_str());
		lv_table_set_cell_value(bandtable, bandRowSelected, 1, byteB.c_str());
		int count = lv_table_get_row_cnt(bandtable);
		if (bandRowSelected < count)
		{
			if (devicesAdresses.size() < bandRowSelected)
				bandRowSelected++;
			lv_obj_invalidate(bandtable);
		}
	}
}

void gui_i2csetup::setBand(int band)
{
	char str[80];
	int length, devicesCount;

	devicesCount = lv_table_get_row_cnt(table) - 1;
	if (devicesCount > 0)
	{
		if (band < Settings_file.meters.size())
			sprintf(str, "%d%s", Settings_file.meters.at(band), Settings_file.labels.at(band).c_str());
		else
			strcpy(str, "Through");
		std::vector<std::string> List = Settings_file.get_array_string("i2c", std::string(str));
		lv_table_set_row_cnt(bandtable, 1);
		int count = List.size() / 2;
		if (count > devicesCount)
			count = devicesCount;
		for (int i = 0; i < count; i++)
		{
			std::string buf = strlib::toUpper(List.at(i));
			std::string buf1 = strlib::toUpper(List.at(i + List.size() / 2));

			lv_table_set_cell_value(bandtable, i + 1, 0, buf.c_str());
			lv_table_set_cell_value(bandtable, i + 1, 1, buf1.c_str());
		}
		if (List.size() / 2 < devicesCount)
		{
			for (int i = 0; i < devicesCount - (List.size() / 2); i++)
			{
				lv_table_set_cell_value(bandtable, i + 1, 0, "00");
				lv_table_set_cell_value(bandtable, i + 1, 1, "00");
			}
		}
	}
}

void gui_i2csetup::save_button_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);

	Save();
}

void gui_i2csetup::Save()
{
	char str[80];
	int count;
	std::vector<string> list;

	lv_dropdown_get_selected_str(bandDropdown, str, 79);
	std::string band(str);
	band = strlib::remove_spaces(band);

	count = lv_table_get_row_cnt(bandtable);
	for (int i = 1; i < count; i++)
	{
		std::string col(lv_table_get_cell_value(bandtable, i, 0));
		list.push_back(col);
	}
	for (int i = 1; i < count; i++)
	{
		std::string col(lv_table_get_cell_value(bandtable, i, 1));
		list.push_back(col);
	}
	Settings_file.set_array_string("i2c", band, list);

	list.clear();
	count = lv_table_get_row_cnt(table);
	for (int i = 1; i < count; i++)
	{
		std::string col(lv_table_get_cell_value(table, i, 1));
		list.push_back(col);
	}
	Settings_file.set_array_string("i2c", "devices", list);

	list.clear();
	count = lv_table_get_row_cnt(table);
	for (int i = 1; i < count; i++)
	{
		std::string col(lv_table_get_cell_value(table, i, 2));
		list.push_back(col);
	}
	Settings_file.set_array_string("i2c", "address", list);
	Settings_file.write_settings();
}

void gui_i2csetup::init(lv_obj_t *o_tab, lv_coord_t w, lv_coord_t h, lv_group_t *bg)
{
	const lv_coord_t x_page_margin = 5;
	const lv_coord_t x_margin = 10;
	const lv_coord_t y_margin = 5;
	const int x_number_buttons = 6;
	const int y_number_buttons = 4;
	const lv_coord_t tab_margin = 20;

	int button_width_margin = ((w - tab_margin) / x_number_buttons);
	int button_width = ((w - tab_margin) / x_number_buttons) - x_margin;
	int button_height = 30;
	int button_height_margin = button_height + y_margin;
	int ibutton_x = 0, ibutton_y = 0;
	int cHeight = lv_obj_get_content_height(o_tab);
	int cWidth = lv_obj_get_content_width(o_tab);
	float fraction = 21.0f / 80.0f;
	float fraction1 = 14.0f / 80.0f;

	// Example casting
	lv_msg_subscribe(MSG_TEXTMESSAGE, msg_txtmessage_handler,(void *)this);
	
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

	SaveObj = lv_btn_create(o_tab);
	lv_obj_add_style(SaveObj, &style_btn, 0);
	lv_obj_add_event_cb(SaveObj, save_button_handler, LV_EVENT_CLICKED, (void *)this);
	lv_obj_align(SaveObj, LV_ALIGN_TOP_LEFT, cWidth - button_width_margin - x_margin, cHeight - button_height_margin );
	lv_obj_set_size(SaveObj, button_width, button_height);
	lv_obj_t *lv_label = lv_label_create(SaveObj);
	lv_label_set_text(lv_label, "Save");
	lv_obj_center(lv_label);

	deviceDropdown = lv_dropdown_create(o_tab);
	lv_obj_add_style(deviceDropdown, &style_btn, 0);
	lv_group_add_obj(bg, deviceDropdown);
	lv_obj_align(deviceDropdown, LV_ALIGN_TOP_LEFT, x_page_margin + x_margin + cWidth * fraction, y_margin + ibutton_y * button_height_margin);
	lv_dropdown_clear_options(deviceDropdown);
	lv_obj_add_event_cb(deviceDropdown, deviceDropdownHandler, LV_EVENT_VALUE_CHANGED, (void *)this);
	lv_obj_set_size(deviceDropdown, button_width, button_height + y_margin);
	
	for (auto col : devices)
	{
		lv_dropdown_add_option(deviceDropdown, col.c_str(), LV_DROPDOWN_POS_LAST);
	}
	
	addressObj = lv_roller_create(o_tab);
	lv_obj_add_style(addressObj, &style_btn, 0);
	lv_roller_set_options(addressObj, pcf8475_opts, LV_ROLLER_MODE_NORMAL);
	lv_roller_set_visible_row_count(addressObj, 2);
	//lv_obj_add_style(addressObj, &style_sel, LV_PART_SELECTED);
	lv_obj_align(addressObj, LV_ALIGN_TOP_LEFT, x_page_margin + x_margin + cWidth * fraction,
				 (ibutton_y + 3) * button_height);

	lv_obj_add_event_cb(addressObj, AddressHandler, LV_EVENT_ALL, (void *)this);
	lv_roller_set_selected(addressObj, 5, LV_ANIM_OFF);
	lv_group_add_obj(bg, addressObj);

	
	//ibutton_y++;
	lv_style_init(&tablestyle);
	lv_style_set_radius(&tablestyle, 0);
	lv_style_set_bg_color(&tablestyle, lv_color_black());

	//lv_style_set_bg_color(&tablestyle, lv_color_make(0x60, 0x60, 0x60));
	lv_style_set_bg_grad_color(&tablestyle, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_bg_grad_dir(&tablestyle, LV_GRAD_DIR_VER);
	lv_style_set_bg_opa(&tablestyle, 255);
	lv_style_set_border_color(&tablestyle, lv_color_make(0x9b, 0x36, 0x36)); // lv_color_make(0x2e, 0x44, 0xb2)
	lv_style_set_border_width(&tablestyle, 2);
	lv_style_set_border_opa(&tablestyle, 255);
	lv_style_set_outline_color(&tablestyle, lv_color_black());
	lv_style_set_outline_opa(&tablestyle, 255);
	
	table = lv_table_create(o_tab);
	lv_obj_add_style(table, &tablestyle, 0);
	lv_obj_clear_flag(table, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_pos(table, x_page_margin, y_margin + ibutton_y * button_height_margin);
	lv_obj_set_size(table, cWidth * fraction, cHeight - (y_margin + ibutton_y * button_height_margin) );
	lv_obj_add_event_cb(table, table_draw_part_event_cb, LV_EVENT_DRAW_PART_BEGIN, (void *)this);
	lv_obj_add_event_cb(table, table_press_part_event_cb, LV_EVENT_PRESSED, (void *)this);

	lv_obj_set_style_pad_top(table, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(table, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_left(table, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_right(table, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(table, 0, LV_PART_ITEMS);

	lv_obj_set_style_pad_left(table, 0, LV_PART_ITEMS);
	lv_obj_set_style_pad_right(table, 0, LV_PART_ITEMS);

	lv_table_set_cell_value(table, 0, 0, "No");
	lv_table_set_col_width(table, 0, cWidth / 16);
	lv_table_set_cell_value(table, 0, 1, "Device");
	lv_table_set_col_width(table, 1, cWidth / 10);
	lv_table_set_cell_value(table, 0, 2, "Address");
	lv_table_set_col_width(table, 2, cWidth / 10);

	devicesList = Settings_file.get_array_string("i2c", "devices");
	devicesAdresses = Settings_file.get_array_string("i2c", "address");
	char str[128];
	RowCount = 1;
	for (auto col: devicesList)
	{
		sprintf(str, "%02d", RowCount);
		lv_table_set_cell_value(table, RowCount, 0, str);

		std::string buf = strlib::toUpper(col);
		lv_table_set_cell_value(table, RowCount, 1, buf.c_str());

		if (devicesAdresses.size() > RowCount - 1)
			lv_table_set_cell_value(table, RowCount, 2, devicesAdresses.at(RowCount - 1).c_str());
		else
			lv_table_set_cell_value(table, RowCount, 2, "00");		
		RowCount++;
	}
	
	addButton = lv_btn_create(o_tab);
	lv_obj_add_style(addButton, &style_btn, 0);
	lv_obj_add_event_cb(addButton, addButton_handler, LV_EVENT_CLICKED, (void *)this);
	lv_obj_align_to(addButton, table, LV_ALIGN_OUT_RIGHT_TOP, x_margin, y_margin + button_height_margin);
	lv_obj_set_size(addButton, button_width / 2, button_height);
	lv_obj_t *lv_label_add = lv_label_create(addButton);
	lv_label_set_text(lv_label_add, "Add");
	lv_obj_center(lv_label_add);

	delButton = lv_btn_create(o_tab);
	lv_obj_add_style(delButton, &style_btn, 0);
	lv_obj_add_event_cb(delButton, delButton_handler, LV_EVENT_CLICKED, (void *)this);
	lv_obj_align_to(delButton, table, LV_ALIGN_OUT_RIGHT_TOP, x_margin / 2 + x_margin + button_width / 2, y_margin + button_height_margin);
	lv_obj_set_size(delButton, button_width / 2, button_height);
	lv_obj_t *lv_label_del = lv_label_create(delButton);
	lv_label_set_text(lv_label_del, "Del");
	lv_obj_center(lv_label_del);
	
	bandtable = lv_table_create(o_tab);
	lv_obj_add_style(bandtable, &tablestyle, 0);
	lv_obj_clear_flag(bandtable, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_pos(bandtable, cWidth / 2, y_margin + ibutton_y * button_height_margin);
	lv_obj_set_size(bandtable, cWidth * fraction1, cHeight - (y_margin + ibutton_y * button_height_margin));
	lv_obj_add_event_cb(bandtable, bandtable_draw_part_event_cb, LV_EVENT_DRAW_PART_BEGIN, (void *)this);
	lv_obj_add_event_cb(bandtable, bandtable_press_part_event_cb, LV_EVENT_PRESSED, (void *)this);

	lv_obj_set_style_pad_top(bandtable, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(bandtable, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_left(bandtable, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_right(bandtable, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(bandtable, 0, LV_PART_ITEMS);

	lv_obj_set_style_pad_left(bandtable, 20, LV_PART_ITEMS);
	lv_obj_set_style_pad_right(bandtable, 0, LV_PART_ITEMS);

	lv_table_set_cell_value(bandtable, 0, 0, "RX");
	lv_table_set_col_width(bandtable, 0, cWidth / 12);
	lv_table_set_cell_value(bandtable, 0, 1, "TX");
	lv_table_set_col_width(bandtable, 1, cWidth / 12);
	setBand(0);
	
	bandDropdown = lv_dropdown_create(o_tab);
	lv_obj_add_style(bandDropdown, &style_btn, 0);
	lv_group_add_obj(bg, bandDropdown);
	lv_obj_align_to(bandDropdown, bandtable, LV_ALIGN_OUT_RIGHT_TOP, x_margin , 0);
	lv_dropdown_clear_options(bandDropdown);
	lv_obj_add_event_cb(bandDropdown, bandDropdownHandler, LV_EVENT_VALUE_CHANGED, (void *)this);
	lv_obj_set_size(bandDropdown, button_width, button_height + y_margin);
	
	int i = 0;
	for (auto col : Settings_file.meters)
	{
		char str[80];

		sprintf(str, "%d %s", col, Settings_file.labels.at(i++).c_str());
		lv_dropdown_add_option(bandDropdown, str, LV_DROPDOWN_POS_LAST);
	}
	lv_dropdown_add_option(bandDropdown, "Through", LV_DROPDOWN_POS_LAST);
	
	editButton = lv_btn_create(o_tab);
	lv_obj_add_style(editButton, &style_btn, 0);
	lv_obj_add_event_cb(editButton, editButton_handler, LV_EVENT_CLICKED, (void *)this);
	lv_obj_align_to(editButton, bandtable, LV_ALIGN_OUT_RIGHT_TOP, x_margin, y_margin + button_height_margin);
	lv_obj_set_size(editButton, button_width, button_height);
	lv_obj_t *lv_label_edit = lv_label_create(editButton);
	lv_label_set_text(lv_label_edit, "Edit");
	lv_obj_center(lv_label_edit);
	
}

