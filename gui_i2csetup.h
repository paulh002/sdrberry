#pragma once
#include "lvgl.h"


class gui_i2csetup
{
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
	void setAdressList(std::string dev);
	void setDevice(std::string dev);
	void setAdress(std::string buffer);
	void setRowSelected(int row) { rowSelected = row;}
	int getRowSelected() { return rowSelected; }
	void setBandRowSelected(int row) { bandRowSelected = row; }
	int getBandRowSelected() { return bandRowSelected; }
	void updateListAddress(std::string buf);
	lv_obj_t *GetaddressObj() { return addressObj; }
	void updateListDevice(std::string buf);
	void setBand(int band);
	void AddLine(std::string byteA, std::string byteB);
	void EditLine(std::string byteA, std::string byteB);
	void AddDevice(std::string dev, std::string add);
	void DelDevice();
	void DelLine();
	void Save();

  private:
	lv_group_t *buttonGroup{nullptr};
	lv_obj_t *deviceDropdown, *bandDropdown;
	lv_obj_t *table, *bandtable;
	lv_style_t tablestyle, style_btn;
	lv_obj_t *addressObj;
	lv_obj_t *SaveObj, *addButton, *editButton, *delButton;
	int RowCount, rowSelected, bandRowSelected;
	std::vector<std::string> devicesList;
	std::vector<std::string> devicesAdresses ;
};

extern gui_i2csetup i2csetup;