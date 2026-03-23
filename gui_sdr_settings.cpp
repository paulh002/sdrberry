#include "gui_sdr_settings.h"
#include "screen.h"
#include "Settings.h"
#include "sdrberry.h"

gui_sdr_settings guiSdrSettings;

void gui_sdr_settings::settings_press_part_event_cb_class(lv_event_t *e)
{
	lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
	lv_table_t *table = (lv_table_t *)obj;
	uint32_t row, col;
	int db, length;

	int count = lv_table_get_row_cnt(obj);
	lv_table_get_selected_cell(obj, &row, &col);
	std::string targetname(lv_table_get_cell_value(obj, row, 0));
	
	auto settingsList = SdrDevices.SdrDevices.at(default_radio)->getSettingsList();
	auto it = std::find_if(settingsList.begin(), settingsList.end(), [&targetname](const SoapySDR::ArgInfo obj) { return obj.name == targetname; });
	if (it != settingsList.end())
	{
		SoapySDR::ArgInfo setting = *it;
		if (setting.type == SoapySDR::ArgInfo::STRING)
		{
			std::string value(lv_table_get_cell_value(obj, row, 1));
			auto it_setting = std::find(setting.options.begin(), setting.options.end(), value);
			it_setting++;
			if (it_setting == setting.options.end())
				it_setting = setting.options.begin();
			value = *it_setting;
			lv_table_set_cell_value(obj, row, col, value.c_str());
			SdrDevices.SdrDevices.at(default_radio)->writeSetting(it->key, value);
			Settings_file.save_string(default_radio, it->key, value);
			Settings_file.write_settings();
			return;
		}
	}

	
	if (col == 1 && row > 0 && row < count)
	{
		std::string str(lv_table_get_cell_value(obj, row, 1));
		size_t i = str.find("true");
		if (i != string::npos)
		{
			lv_table_set_cell_value(settingsTable, row, col, "false");
		}
		else
		{
			lv_table_set_cell_value(settingsTable, row, col, "true");
		}
		std::string key(lv_table_get_cell_value(obj, row, 0));
		std::string value(lv_table_get_cell_value(obj, row, 1));
		SdrDevices.SdrDevices.at(default_radio)->writeSetting(key, value);
		Settings_file.save_string(default_radio, key, value);
		Settings_file.write_settings();
	}
}

void gui_sdr_settings::settings_draw_part_event_cb_class(lv_event_t *e)
{
	lv_draw_task_t *draw_task = lv_event_get_draw_task(e);
	lv_draw_dsc_base_t *base_dsc = (lv_draw_dsc_base_t *)lv_draw_task_get_draw_dsc(draw_task);

	lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
	lv_table_t *table = (lv_table_t *)obj;

	/*If the cells are drawn...*/
	if (base_dsc->part == LV_PART_ITEMS)
	{
		uint32_t row = base_dsc->id1 / lv_table_get_col_cnt(obj);
		uint32_t col = base_dsc->id2 - row * lv_table_get_col_cnt(obj);

		/*Make the texts in the first cell center aligned*/
		lv_draw_fill_dsc_t *fill_draw_dsc = lv_draw_task_get_fill_dsc(draw_task);
		if (fill_draw_dsc)
		{
			if (row == 0)
			{
				fill_draw_dsc->color = lv_color_mix(lv_palette_main(LV_PALETTE_CYAN), fill_draw_dsc->color, LV_OPA_10);
				fill_draw_dsc->opa = LV_OPA_COVER;
			}
			/*MAke every 2nd row grayish*/
			if ((row != 0 && row % 2) == 0)
			{
				fill_draw_dsc->color = lv_color_mix(lv_palette_main(LV_PALETTE_GREY), fill_draw_dsc->color, LV_OPA_10);
				fill_draw_dsc->opa = LV_OPA_COVER;
			}
		}
	}
}

void gui_sdr_settings::init(lv_obj_t *o_tab, lv_coord_t w, lv_coord_t h, lv_group_t *bg)
{
	settingsTable = lv_table_create(o_tab);
	lv_obj_add_event_cb(settingsTable, settings_draw_part_event_cb, LV_EVENT_DRAW_TASK_ADDED, (void *)this);
	lv_obj_add_flag(settingsTable, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS);
	lv_obj_add_event_cb(settingsTable, settings_press_part_event_cb, LV_EVENT_PRESSED, (void *)this);
	lv_style_init(&settings_style);
	lv_style_set_radius(&settings_style, 0);
	lv_style_set_bg_color(&settings_style, lv_color_black());
	lv_obj_add_style(settingsTable, &settings_style, 0);
	lv_obj_set_style_pad_top(settingsTable, 5, LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(settingsTable, 5, LV_PART_MAIN);
	lv_obj_set_style_pad_left(settingsTable, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_right(settingsTable, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(settingsTable, 10, LV_PART_ITEMS);
	lv_obj_align(settingsTable, LV_ALIGN_TOP_LEFT, 0, 0);

	int width1, width2, height;

	if (screenWidth <= small_res)
	{
		width1 = w / 6;
		width2 = w / 6;
		height = h - 20;
	}
	else
	{
		width1 = w / 6;
		width2 = w / 16;
		height = h / 2;
	}

	lv_table_set_cell_value(settingsTable, 0, 0, "Setting");
	lv_table_set_col_width(settingsTable, 0, width1);
	lv_table_set_cell_value(settingsTable, 0, 1, "value");
	lv_table_set_col_width(settingsTable, 1, width2);
	lv_obj_set_size(settingsTable, width1 + width2, height);
}

void gui_sdr_settings::init_settings()
{
	int row = 1;
	lv_table_set_row_cnt(settingsTable, 1);
	for (auto col : SdrDevices.SdrDevices.at(default_radio)->getSettingsList())
	{
		if (col.type == SoapySDR::ArgInfo::BOOL)
		{
			lv_table_set_cell_value(settingsTable, row, 0, col.name.c_str());
			lv_table_set_cell_value(settingsTable, row, 1, col.value.c_str());

			std::string value(lv_table_get_cell_value(settingsTable, row, 1));
			std::string def_value = Settings_file.get_string(default_radio, col.key);
			if (def_value == "true")
				value = "true";
			if (def_value == "false")
				value = "false";

			lv_table_set_cell_value(settingsTable, row, 1, value.c_str());
			SdrDevices.SdrDevices.at(default_radio)->writeSetting(col.key, value);
			row++;
		}

		if (col.type == SoapySDR::ArgInfo::STRING)
		{
			lv_table_set_cell_value(settingsTable, row, 0, col.name.c_str());
			lv_table_set_cell_value(settingsTable, row, 1, col.value.c_str());

			std::string value;
			if (col.options.size())
			{
				value = col.options.at(0);
			}
			std::string def_value = Settings_file.get_string(default_radio, col.key);
			if (def_value.size())
				value = def_value;
			lv_table_set_cell_value(settingsTable, row, 1, value.c_str());
			SdrDevices.SdrDevices.at(default_radio)->writeSetting(col.key, value);
			row++;
		}
	}
}