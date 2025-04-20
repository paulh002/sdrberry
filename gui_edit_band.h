#pragma once
#include "lvgl_.h"

class gui_edit_band
{
  private:
	lv_group_t *buttonGroup{nullptr};
	lv_obj_t *table, *parent;
	lv_style_t tablestyle, style_btn;
	lv_obj_t *SaveObj, *CancelObj, *DelObj, *editButton, *AddObj;
	lv_group_t *keyboardgroup;

	int button_width;
	int button_height;
	int button_width_margin;
	int button_height_margin;
	int column, rowno;
	int selected;

	void Save();
	void fill_band_table();
	void save_button_handler_class(lv_event_t *e);
	void cancel_button_handler_class(lv_event_t *e);
	void add_button_handler_class(lv_event_t *e);
	void del_button_handler_class(lv_event_t *e);
	void table_press_part_event_cb_class(lv_event_t *e);
	void table_custom_keypad_handler_class(lv_event_t *e);
	void table_draw_part_event_cb_class(lv_event_t *e);
	
  public:
	void init(lv_obj_t *o_tab, lv_coord_t w, lv_coord_t h, lv_group_t *bg, lv_group_t *keyboard_group);
	
	static constexpr auto save_button_handler = EventHandler<gui_edit_band, &gui_edit_band::save_button_handler_class>::staticHandler;
	static constexpr auto cancel_button_handler = EventHandler<gui_edit_band, &gui_edit_band::cancel_button_handler_class>::staticHandler;
	static constexpr auto table_press_part_event_cb = EventHandler<gui_edit_band, &gui_edit_band::table_press_part_event_cb_class>::staticHandler;
	static constexpr auto table_custom_keypad_handler = EventHandler<gui_edit_band, &gui_edit_band::table_custom_keypad_handler_class>::staticHandler;
	static constexpr auto table_draw_part_event_cb = EventHandler<gui_edit_band, &gui_edit_band::table_draw_part_event_cb_class>::staticHandler;
	static constexpr auto add_button_handler = EventHandler<gui_edit_band, &gui_edit_band::add_button_handler_class>::staticHandler;
	static constexpr auto del_button_handler = EventHandler<gui_edit_band, &gui_edit_band::del_button_handler_class>::staticHandler;
};

extern gui_edit_band guieditband;