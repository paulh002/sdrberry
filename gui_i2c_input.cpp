#include "gui_i2c_input.h"
#include <vector>
#include "Settings.h"
#include "strlib.h"
#include <algorithm>

extern const char *None_opts;
extern const char *pcf8475_opts;
extern const char *pcf8475a_opts;
extern const char *mcp22008_opts;
extern const char *tca9548_opts;                                         
extern std::vector<std::string> devices;

gui_i2c_input gui_i2cinput;

void gui_i2c_input::save_button_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_CLICKED)
	{
		std::string buffer;
		buffer.resize(80);
		int selection = lv_dropdown_get_selected(deviceDropdown);
		lv_dropdown_get_selected_str(deviceDropdown, buffer.data(), 79);
		buffer.resize(strlen(buffer.data()));
		Settings_file.save_string("i2c", "input_device", buffer);

		selection = lv_roller_get_selected(addressObj);
		lv_roller_get_selected_str(addressObj, buffer.data(), 79);
		buffer.resize(strlen(buffer.data()));
		Settings_file.save_string("i2c", "input_address", buffer);
		Settings_file.write_settings();
	}
}

void gui_i2c_input::AddressHandler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
}

void gui_i2c_input::deviceDropdownHandler_class(lv_event_t *e)
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
		setAdressList(buffer);
		//updateListDevice(buffer);
	}
}


void gui_i2c_input::init(lv_obj_t *o_tab, lv_coord_t w, lv_coord_t h, lv_group_t *bg)
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
	float fraction = 14.0f / 80.0f;
	
	
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
	lv_obj_align(deviceDropdown, LV_ALIGN_TOP_LEFT, x_page_margin + x_margin, y_margin + button_height_margin );
	lv_dropdown_clear_options(deviceDropdown);
	lv_obj_add_event_cb(deviceDropdown, deviceDropdownHandler, LV_EVENT_VALUE_CHANGED, (void *)this);
	lv_obj_set_size(deviceDropdown, button_width, button_height + y_margin);
	
	for (auto col : devices)
	{
		lv_dropdown_add_option(deviceDropdown, col.c_str(), LV_DROPDOWN_POS_LAST);
	}

	lv_obj_t *i2cdevice_label = lv_label_create(o_tab);
	lv_label_set_text(i2cdevice_label, "I2C input device");
	lv_obj_align_to(i2cdevice_label, deviceDropdown, LV_ALIGN_OUT_TOP_LEFT, 0, -10);

	std::string str = Settings_file.get_string("i2c", "input_device");
	int ii = 0;
	if (std::find(devices.begin(), devices.end(), str) != devices.end())
		ii = std::distance(devices.begin(), std::find(devices.begin(), devices.end(), str));
	lv_dropdown_set_selected(deviceDropdown, ii);

	addressObj = lv_roller_create(o_tab);
	lv_obj_add_style(addressObj, &style_btn, 0);
	lv_roller_set_options(addressObj, pcf8475_opts, LV_ROLLER_MODE_NORMAL);
	lv_roller_set_visible_row_count(addressObj, 2);
	//lv_obj_add_style(addressObj, &style_sel, LV_PART_SELECTED);
	lv_obj_align(addressObj, LV_ALIGN_TOP_LEFT, x_page_margin + x_margin + cWidth * fraction,
				 y_margin + button_height_margin );

	lv_obj_add_event_cb(addressObj, AddressHandler, LV_EVENT_ALL, (void *)this);
	setAdressList(str);

	lv_roller_set_selected(addressObj, 0, LV_ANIM_OFF);
	str = Settings_file.get_string("i2c", "input_address");
	for (int i = 0; i < lv_roller_get_option_cnt(addressObj); i++)
	{
		std::string buf;
		buf.resize(80);
		lv_roller_set_selected(addressObj, i, LV_ANIM_OFF);
		lv_roller_get_selected_str(addressObj, buf.data(), 79);
		buf.resize(strlen(buf.data()));
		if (buf == str)
			break;
	}

	lv_obj_t *i2cAddress_label = lv_label_create(o_tab);
	lv_label_set_text(i2cAddress_label, "I2C address");
	lv_obj_align_to(i2cAddress_label, addressObj, LV_ALIGN_OUT_TOP_LEFT, 0, -10);

	lv_style_init(&tablestyle);
	lv_style_set_radius(&tablestyle, 0);
	lv_style_set_bg_color(&tablestyle, lv_color_black());
	// lv_style_set_bg_color(&tablestyle, lv_color_make(0x60, 0x60, 0x60));
	lv_style_set_bg_grad_color(&tablestyle, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_bg_grad_dir(&tablestyle, LV_GRAD_DIR_VER);
	lv_style_set_bg_opa(&tablestyle, 255);
	lv_style_set_border_color(&tablestyle, lv_color_make(0x9b, 0x36, 0x36)); // lv_color_make(0x2e, 0x44, 0xb2)
	lv_style_set_border_width(&tablestyle, 2);
	lv_style_set_border_opa(&tablestyle, 255);
	lv_style_set_outline_color(&tablestyle, lv_color_black());
	lv_style_set_outline_opa(&tablestyle, 255);
	
	float fraction1 = 20.0f / 80.0f;
	gpiotable = lv_table_create(o_tab);
	lv_obj_add_style(gpiotable, &tablestyle, 0);
	lv_obj_clear_flag(gpiotable, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_pos(gpiotable, cWidth / 2, y_margin + ibutton_y * button_height_margin);
	lv_obj_set_size(gpiotable, cWidth * fraction1, cHeight - (y_margin + ibutton_y * button_height_margin));
	//lv_obj_add_event_cb(gpiotable, bandtable_draw_part_event_cb, LV_EVENT_DRAW_PART_BEGIN, (void *)this);
	//lv_obj_add_event_cb(gpiotable, bandtable_press_part_event_cb, LV_EVENT_PRESSED, (void *)this);

	lv_obj_set_style_pad_top(gpiotable, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(gpiotable, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_left(gpiotable, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_right(gpiotable, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(gpiotable, 0, LV_PART_ITEMS);

	lv_obj_set_style_pad_left(gpiotable, 20, LV_PART_ITEMS);
	lv_obj_set_style_pad_right(gpiotable, 0, LV_PART_ITEMS);

	lv_table_set_cell_value(gpiotable, 0, 0, "GPIO");
	lv_table_set_col_width(gpiotable, 0, cWidth / 12);
	lv_table_set_cell_value(gpiotable, 0, 1, "Function");
	lv_table_set_col_width(gpiotable, 1, cWidth * fraction1 - (cWidth / 12));
	lv_table_set_cell_value(gpiotable, 1, 0, "0");
	lv_table_set_cell_value(gpiotable, 1, 1, "TX/RX");
	lv_table_set_cell_value(gpiotable, 2, 0, "1");
	lv_table_set_cell_value(gpiotable, 2, 1, "Vol+");
	lv_table_set_cell_value(gpiotable, 3, 0, "2");
	lv_table_set_cell_value(gpiotable, 3, 1, "Vol-");
	lv_table_set_cell_value(gpiotable, 4, 0, "3");
	lv_table_set_cell_value(gpiotable, 4, 1, "Band+");
	lv_table_set_cell_value(gpiotable, 5, 0, "4");
	lv_table_set_cell_value(gpiotable, 5, 1, "Band-");
	lv_table_set_cell_value(gpiotable, 6, 0, "5");
	lv_table_set_cell_value(gpiotable, 6, 1, "Mode+");
	lv_table_set_cell_value(gpiotable, 7, 0, "6");
	lv_table_set_cell_value(gpiotable, 7, 1, "Mode-");

	lv_group_add_obj(bg, addressObj);
	
}

void gui_i2c_input::setAdressList(std::string buffer)
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
		lv_roller_set_options(addressObj, tca9548_opts, LV_ROLLER_MODE_NORMAL);
}