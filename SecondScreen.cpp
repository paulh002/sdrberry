#include "SecondScreen.h"
#include "sdrberry.h"
#include "vfo.h"

void SecondScreen::init(lv_obj_t *parent_, lv_group_t *keyboard_group)
{
	parent = parent_;
	x = 0;
	y = 0;
	width = lv_obj_get_width(parent_);
	height = lv_obj_get_height(parent_);

	parent = lv_obj_create(parent_);
	lv_obj_set_pos(parent, 0, 0);
	lv_obj_set_size(parent, width, height);
	lv_obj_set_style_bg_color(parent, lv_color_black(), 0);
	lv_obj_set_style_radius(parent, 0, 0);
	lv_obj_set_style_border_side(parent, (lv_border_side_t)(0), LV_PART_MAIN);
	vfo_height = height / 5;

	guivfo = std::make_unique<gui_vfo>();
	guivfo->gui_vfo_init(parent, 0, 0, width, vfo_height, keyboard_group);

	spec = lv_obj_create(parent);
	lv_obj_set_pos(spec, 0, vfo_height);
	lv_obj_set_size(spec, width, height - vfo_height);
	lv_obj_set_style_bg_color(spec, lv_color_black(), 0);
	lv_obj_set_style_radius(spec, 0, 0);
	lv_obj_set_style_border_side(spec, (lv_border_side_t)(0), LV_PART_MAIN);

	lv_obj_clear_flag(parent, (lv_obj_flag_t)(LV_OBJ_FLAG_SCROLL_CHAIN | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_ONE));
}

void SecondScreen::deinit()
{
	if (SpectrumGraph_page)
		SpectrumGraph_page.reset();
	if (guivfo)
		guivfo.reset();
	if (parent)
		lv_obj_delete(parent);
}

void SecondScreen::DrawDisplay()
{
	if (SpectrumGraph_page)
	{
		SpectrumGraph_page->DrawDisplay();
		SpectrumGraph_page->set_pos(vfo.get_vfo_offset(vfo.get_active_vfo()));
	}
	if (guivfo)
	{
		if (vfo.get_active_vfo() == 0)
			guivfo->set_vfo_gui(0, vfo.get_vfo_frequency(vfo_activevfo::One),
								vfo.get_rx(), vfo.get_mode_no(0), vfo.get_band_no(0), vfo.getBandIndex(0));
		else
			guivfo->set_vfo_gui(1, vfo.get_vfo_frequency(vfo_activevfo::Two),
								vfo.get_rx(), vfo.get_mode_no(1), vfo.get_band_no(1), vfo.getBandIndex(1));
		guivfo->set_span(vfo.get_span() / 1000);
	}
}

void SecondScreen::ProcessWaterfall(const IQSampleVector &input)
{
	if (SpectrumGraph_page)
	{
		SpectrumGraph_page->ProcessWaterfall(input);
	}
}

void SecondScreen::set_s_meter(double value)
{
	if (guivfo)
	{
		guivfo->set_s_meter(value);
	}
}

void SecondScreen::setWaterfallSize(int waterfallsize)
{
	if (SpectrumGraph_page)
	{
		SpectrumGraph_page->setWaterfallSize(waterfallsize);
	}
}

void SecondScreen::init_vfo()
{

	if (SpectrumGraph_page == nullptr)
	{
		SpectrumGraph_page = std::make_unique<Spectrum>();
		SpectrumGraph_page->init(spec, 0, 0, width, height - vfo_height, ifrate);
	}

	if (guivfo)
	{

		guivfo->set_vfo_gui(0, vfo.get_vfo_frequency(vfo_activevfo::One),
								vfo.get_rx(), vfo.get_mode_no(0), vfo.get_band_no(0), vfo.getBandIndex(0));
	
		guivfo->set_vfo_gui(1, vfo.get_vfo_frequency(vfo_activevfo::Two),
								vfo.get_rx(), vfo.get_mode_no(1), vfo.get_band_no(1), vfo.getBandIndex(1));
		guivfo->set_span(vfo.get_span() / 1000);
	}
}

void SecondScreen::set_cursor_mode(int mode)
{
	if (SpectrumGraph_page)
	{
		SpectrumGraph_page->set_cursor_mode(mode);
	}
}