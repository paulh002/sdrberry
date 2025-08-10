#pragma once
#include "lvgl_.h"
#include <string>
#include <map>
#include <tuple>

class gui_preset
{
  private:
	lv_group_t *buttonGroup{nullptr};
	lv_obj_t *preset_table, *display_table;
	lv_style_t tablestyle, style_btn;
	lv_obj_t *addressObj;
	lv_obj_t *SaveObj, *addButton, *updateButton, *deleteButton;
	lv_group_t *keyboardgroup;
	int row_selected, display_row_selected;
	std::map<int, std::tuple<std::string, long, std::string>> preset_list;
	lv_obj_t *tileview, *main_tile, *edit_tile;

	void save_button_handler_class(lv_event_t *e);
	void update_button_handler_class(lv_event_t *e);
	void delete_button_handler_class(lv_event_t *e);
	void add_button_handler_class(lv_event_t *e);
	void table_press_part_event_class(lv_event_t *e);
	void table_draw_part_event_class(lv_event_t *e);
	void display_press_part_event_class(lv_event_t *e);
	void display_draw_part_event_class(lv_event_t *e);

	void reload_display_table();

  public:
	void init(lv_obj_t *o_tab, lv_coord_t w, lv_coord_t h, lv_group_t *bg, lv_group_t *keyboard_group);
	static constexpr auto save_button_handler = EventHandler<gui_preset, &gui_preset::save_button_handler_class>::staticHandler;
	static constexpr auto add_button_handler = EventHandler<gui_preset, &gui_preset::add_button_handler_class>::staticHandler;
	static constexpr auto update_button_handler = EventHandler<gui_preset, &gui_preset::update_button_handler_class>::staticHandler;
	static constexpr auto delete_button_handler = EventHandler<gui_preset, &gui_preset::delete_button_handler_class>::staticHandler;
	static constexpr auto table_press_part_event_cb = EventHandler<gui_preset, &gui_preset::table_press_part_event_class>::staticHandler;
	static constexpr auto table_draw_part_event_cb = EventHandler<gui_preset, &gui_preset::table_draw_part_event_class>::staticHandler;
	static constexpr auto display_press_part_event_cb = EventHandler<gui_preset, &gui_preset::display_press_part_event_class>::staticHandler;
	static constexpr auto display_draw_part_event_cb = EventHandler<gui_preset, &gui_preset::display_draw_part_event_class>::staticHandler;
	
};

extern gui_preset guipreset;