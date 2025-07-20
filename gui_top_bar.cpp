#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <string>
#include <chrono>
#include <tz.h>
#include "lvgl.h"
#include "gui_top_bar.h"
#include "screen.h"
#include "Settings.h"

gui_top_bar GuiTopBar;

void gui_top_bar::setup_top_bar(lv_obj_t* scr)
{	
	lv_style_init(&top_style);
	lv_style_set_radius(&top_style, 0);
	lv_style_set_bg_color(&top_style, lv_palette_main(LV_PALETTE_INDIGO));
		
	bg_top = lv_obj_create(scr);
	lv_obj_add_style(bg_top, &top_style, 0);
	lv_obj_set_size(bg_top, LV_HOR_RES, topHeight);
	lv_obj_clear_flag(bg_top, LV_OBJ_FLAG_SCROLLABLE);

	lv_style_init(&LabelTextStyle);
	lv_style_set_text_color(&LabelTextStyle, lv_color_white());
	
	label_status = lv_label_create(bg_top);
	lv_obj_add_style(label_status, &LabelTextStyle, 0);
	lv_obj_align(label_status, LV_ALIGN_LEFT_MID, 0, 0);

	lv_style_init(&TimeTextStyle);
	lv_style_set_text_color(&TimeTextStyle, lv_color_white());

	label_date_time = lv_label_create(bg_top);
	lv_obj_add_style(label_date_time, &TimeTextStyle, 0);
	lv_obj_align(label_date_time, LV_ALIGN_LEFT_MID, LV_HOR_RES - LV_HOR_RES / 3.5, 0);
}

void gui_top_bar::set_time_label()
{
	std::stringstream ss;
	std::string timezone = Settings_file.get_string("Radio", "timezone");
	auto t = make_zoned(date::current_zone(), date::floor<std::chrono::seconds>(std::chrono::system_clock::now()));
	if (timezone.size())
	{
		try
		{
			auto zone = date::locate_zone(timezone);
			t = make_zoned(zone, date::floor<std::chrono::seconds>(std::chrono::system_clock::now()));
		}
		catch (const date::nonexistent_local_time &e)
		{
			std::cout << e.what() << '\n';
		}
	}
	ss << t;
	lv_label_set_text(label_date_time, ss.str().c_str());
}

void gui_top_bar::set_label_status(std::string s)
{
	label = s;
	lv_label_set_text(label_status, s.c_str());
}

std::string gui_top_bar::getLabel()
{
	return label;
}