#pragma once
#include "lvgl_.h"

class gui_sdr_settings
{
  private:
	void settings_draw_part_event_cb_class(lv_event_t *e);
	void settings_press_part_event_cb_class(lv_event_t *e);
	lv_obj_t *settingsTable;
	lv_style_t style_btn, settings_style;

  public:
	void init(lv_obj_t *o_tab, lv_coord_t w, lv_coord_t h, lv_group_t *bg);
	void init_settings();

	static constexpr auto settings_draw_part_event_cb = EventHandler<gui_sdr_settings, &gui_sdr_settings::settings_draw_part_event_cb_class>::staticHandler;
	static constexpr auto settings_press_part_event_cb = EventHandler<gui_sdr_settings, &gui_sdr_settings::settings_press_part_event_cb_class>::staticHandler;
};

extern gui_sdr_settings guiSdrSettings;