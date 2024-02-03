#pragma once
#include "lvgl_.h"


class gui_i2csetup
{
  private:
	lv_group_t *buttonGroup{nullptr};
	lv_obj_t *deviceDropdown, *bandDropdown;
	lv_obj_t *table, *bandtable;
	lv_style_t tablestyle, style_btn;
	lv_obj_t *addressObj;
	lv_obj_t *SaveObj, *addButton, *editButton, *delButton;
	int RowCount, rowSelected, bandRowSelected;
	std::vector<std::string> devicesList;
	std::vector<std::string> devicesAdresses;

	void AddLine(std::string byteA, std::string byteB);
	void EditLine(std::string byteA, std::string byteB);
	void AddDevice(std::string dev, std::string add);
	void DelDevice();
	void DelLine();
	void Save();
	void setAdressList(std::string dev);
	void setDevice(std::string dev);
	void setAdress(std::string buffer);
	void setRowSelected(int row) { rowSelected = row; }
	int getRowSelected() { return rowSelected; }
	void setBandRowSelected(int row) { bandRowSelected = row; }
	int getBandRowSelected() { return bandRowSelected; }
	void updateListAddress(std::string buf);
	lv_obj_t *GetaddressObj() { return addressObj; }
	void updateListDevice(std::string buf);
	void setBand(int band);

	void editButton_handler_class(lv_event_t *e);
	void delButton_handler_class(lv_event_t *e);
	void addButton_handler_class(lv_event_t *e);
	void bandtable_press_part_event_cb_class(lv_event_t *e);
	void table_press_part_event_cb_class(lv_event_t *e);
	void table_draw_part_event_cb_class(lv_event_t *e);
	void bandtable_draw_part_event_cb_class(lv_event_t *e);
	void bandDropdownHandler_class(lv_event_t *e);
	void deviceDropdownHandler_class(lv_event_t *e);
	void AddressHandler_class(lv_event_t *e);
	void save_button_handler_class(lv_event_t *e);
	void msg_txtmessage_handler_class(void *e, lv_msg_t *m);

  public:
	enum I2CDevice
	{
		None,
		PCF8574,
		PCF8574A,
		MCP23016
	};
	gui_i2csetup() : rowSelected(-1), bandRowSelected(-1) {};
	void init(lv_obj_t *o_tab, lv_coord_t w, lv_coord_t h, lv_group_t *bg);

	
	static constexpr auto editButton_handler = EventHandler<gui_i2csetup, &gui_i2csetup::editButton_handler_class>::staticHandler;
	static constexpr auto delButton_handler = EventHandler<gui_i2csetup, &gui_i2csetup::delButton_handler_class>::staticHandler;
	static constexpr auto addButton_handler = EventHandler<gui_i2csetup, &gui_i2csetup::addButton_handler_class>::staticHandler;
	static constexpr auto table_press_part_event_cb = EventHandler<gui_i2csetup, &gui_i2csetup::table_press_part_event_cb_class>::staticHandler;
	static constexpr auto table_draw_part_event_cb = EventHandler<gui_i2csetup, &gui_i2csetup::table_draw_part_event_cb_class>::staticHandler;
	static constexpr auto bandtable_draw_part_event_cb = EventHandler<gui_i2csetup, &gui_i2csetup::bandtable_draw_part_event_cb_class>::staticHandler;
	static constexpr auto bandDropdownHandler = EventHandler<gui_i2csetup, &gui_i2csetup::bandDropdownHandler_class>::staticHandler;
	static constexpr auto deviceDropdownHandler = EventHandler<gui_i2csetup, &gui_i2csetup::deviceDropdownHandler_class>::staticHandler;
	static constexpr auto AddressHandler = EventHandler<gui_i2csetup, &gui_i2csetup::AddressHandler_class>::staticHandler;
	static constexpr auto bandtable_press_part_event_cb = EventHandler <gui_i2csetup, &gui_i2csetup::bandtable_press_part_event_cb_class>::staticHandler;
	static constexpr auto save_button_handler = EventHandler<gui_i2csetup, &gui_i2csetup::save_button_handler_class>::staticHandler;
	static constexpr auto msg_txtmessage_handler = EventHandlerMsg<gui_i2csetup, &gui_i2csetup::msg_txtmessage_handler_class>::staticHandler;
};

extern gui_i2csetup i2csetup;