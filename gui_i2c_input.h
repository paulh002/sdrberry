#pragma once
#include "lvgl_.h"
#include <string>

class gui_i2c_input
{
  private:
	lv_group_t *buttonGroup{nullptr};
	lv_obj_t *deviceDropdown, *bandDropdown;
	lv_obj_t *table, *bandtable;
	lv_style_t tablestyle, style_btn;
	lv_obj_t *addressObj;
	lv_obj_t *SaveObj, *addButton, *editButton, *delButton;
	lv_obj_t *gpiotable;

	void save_button_handler_class(lv_event_t *e);
	void AddressHandler_class(lv_event_t *e);
	void deviceDropdownHandler_class(lv_event_t *e);

	void setAdressList(std::string buffer);

  public:
	void init(lv_obj_t *o_tab, lv_coord_t w, lv_coord_t h, lv_group_t *bg);

	static constexpr auto save_button_handler = EventHandler<gui_i2c_input, &gui_i2c_input::save_button_handler_class>::staticHandler;
	static constexpr auto AddressHandler = EventHandler<gui_i2c_input, &gui_i2c_input::AddressHandler_class>::staticHandler;
	static constexpr auto deviceDropdownHandler = EventHandler<gui_i2c_input, &gui_i2c_input::deviceDropdownHandler_class>::staticHandler;
};

extern gui_i2c_input gui_i2cinput;