#include <vector>
#include <string>

#include "gui_i2csetup.h"
#include "Settings.h"
#include "strlib.h"

gui_i2csetup i2csetup;
const char *None_opts = "";
const char *pcf8475_opts = "20\n21\n22\n23\n24\n25\n26\n27";
const char *pcf8475a_opts = "38\n39\n3A\n3B\n3C\n3D\n3E\n3F";
const char *mcp22016_opts = "38\n39\n3A\n3B\n3C\n3D\n3E\n3F";

static void bandtable_press_part_event_cb(lv_event_t *e)
{
	lv_obj_t *obj = lv_event_get_target(e);
	lv_table_t *table = (lv_table_t *)obj;
	uint16_t row, col;
	char *ptr;
	std::string buffer;
	
	// Do something
	lv_table_get_selected_cell(obj, &row, &col);
	if (lv_table_get_row_cnt(obj) < row + 1)
		return;
	ptr = (char *)lv_table_get_cell_value(obj, row, 1);
	i2csetup.setBandRowSelected(row);
}

static void table_press_part_event_cb(lv_event_t *e)
{
	lv_obj_t *obj = lv_event_get_target(e);
	lv_table_t *table = (lv_table_t *)obj;
	uint16_t row, col;
	char *ptr;
	std::string buffer;

	lv_table_get_selected_cell(obj, &row, &col);
	if (lv_table_get_row_cnt(obj) < row + 1)
		return;
	ptr = (char *)lv_table_get_cell_value(obj, row, 1);
	buffer.resize(80);
	strcpy(buffer.data(), ptr);
	buffer.resize(strlen(buffer.data()));
	buffer = strlib::toUpper(buffer);
	i2csetup.setDevice(buffer);
	i2csetup.setAdressList(buffer);
	ptr = (char *)lv_table_get_cell_value(obj, row, 2);
	buffer.resize(80);
	strcpy(buffer.data(), ptr);
	buffer.resize(strlen(buffer.data()));
	buffer = strlib::toUpper(buffer);
	i2csetup.setAdress(buffer);
	i2csetup.setRowSelected(row);
}

static void table_draw_part_event_cb(lv_event_t *e)
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

	}
}

static void bandDropdownHandler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_VALUE_CHANGED)
	{
		i2csetup.setBand(lv_dropdown_get_selected(obj));
	}
}


static void deviceDropdownHandler(lv_event_t *e)
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

static void AddressHandler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	std::string buf;

	buf.resize(80);
	if (code == LV_EVENT_VALUE_CHANGED)
	{
		lv_roller_get_selected_str(i2csetup.GetaddressObj(), buf.data(), 79);
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
	if (buf == "MCP23016")
		lv_roller_set_options(addressObj, mcp22016_opts, LV_ROLLER_MODE_NORMAL);
}

void gui_i2csetup::setDevice(std::string buffer)
{
	std::string buf1 = strlib::toUpper(buffer);

	int i = 0;
	for (auto col : devicesList)
	{
		std::string buf = strlib::toUpper(col);
		if (buf == buf1)
			lv_dropdown_set_selected(deviceDropdown, i);
		i++;
	}
}

static void button_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);

		
		
}

void gui_i2csetup::setBand(int band)
{
	char str[80];
	int length;

	sprintf(str, "%d%s", Settings_file.meters.at(band), Settings_file.labels.at(band).c_str());
	std::vector<std::string> List = Settings_file.get_array_string("i2c", std::string(str));
	lv_table_set_row_cnt(bandtable, 1);
	for (int i =0 ; i < List.size() /2; i++)
	{
		std::string buf = strlib::toUpper(List.at(i));
		std::string buf1 = strlib::toUpper(List.at(i+List.size()/2));

		lv_table_set_cell_value(bandtable, i+1, 0, buf.c_str());
		lv_table_set_cell_value(bandtable, i+1, 1, buf1.c_str());
	}
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
	lv_obj_add_event_cb(SaveObj, button_handler, LV_EVENT_CLICKED, NULL);
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
	lv_obj_add_event_cb(deviceDropdown, deviceDropdownHandler, LV_EVENT_VALUE_CHANGED, NULL);

	std::vector<std::string> devices{"NONE", "PCF8574", "PCF8574A", "MCP23016"};
	for (auto col : devices)
	{
		lv_dropdown_add_option(deviceDropdown, col.c_str(), LV_DROPDOWN_POS_LAST);
	}

	addressObj = lv_roller_create(o_tab);
	lv_obj_add_style(addressObj, &style_btn, 0);
	lv_roller_set_options(addressObj, pcf8475_opts, LV_ROLLER_MODE_NORMAL);
	lv_roller_set_visible_row_count(addressObj, 3);
	//lv_obj_add_style(addressObj, &style_sel, LV_PART_SELECTED);
	lv_obj_align(addressObj, LV_ALIGN_TOP_LEFT, x_page_margin + x_margin + cWidth * fraction,
				 (ibutton_y + 2) * button_height_margin);

	lv_obj_add_event_cb(addressObj, AddressHandler, LV_EVENT_ALL, NULL);
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
	lv_obj_add_event_cb(table, table_draw_part_event_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);
	lv_obj_add_event_cb(table, table_press_part_event_cb, LV_EVENT_PRESSED, NULL);

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
	for (int i = 0; i < 10; i++)
	{
		sprintf(str, "%02d", RowCount);
		lv_table_set_cell_value(table, RowCount, 0, str);

		if (devicesList.size() > i)
		{
			std::string buf = strlib::toUpper(devicesList.at(i));
			lv_table_set_cell_value(table, RowCount, 1, buf.c_str());
		}
		else
			lv_table_set_cell_value(table, RowCount, 1, "NONE");

		if (devicesAdresses.size() > i)
			lv_table_set_cell_value(table, RowCount, 2, devicesAdresses.at(i).c_str());
		else
			lv_table_set_cell_value(table, RowCount, 2, "00");		
		RowCount++;
	}

	bandtable = lv_table_create(o_tab);
	lv_obj_add_style(bandtable, &tablestyle, 0);
	lv_obj_clear_flag(bandtable, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_pos(bandtable, cWidth / 2, y_margin + ibutton_y * button_height_margin);
	lv_obj_set_size(bandtable, cWidth * fraction1, cHeight - (y_margin + ibutton_y * button_height_margin));
	//lv_obj_add_event_cb(bandtable, table_draw_part_event_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);
	lv_obj_add_event_cb(bandtable, bandtable_press_part_event_cb, LV_EVENT_PRESSED, NULL);

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
	lv_obj_add_event_cb(bandDropdown, bandDropdownHandler, LV_EVENT_VALUE_CHANGED, NULL);

	int i = 0;
	for (auto col : Settings_file.meters)
	{
		char str[80];

		sprintf(str, "%d %s", col, Settings_file.labels.at(i++).c_str());
		lv_dropdown_add_option(bandDropdown, str, LV_DROPDOWN_POS_LAST);
	}
	
}

