
#include <atomic>
#include <cmath>
#include <complex>
#include <condition_variable>
#include <cstdio>
#include <liquid/liquid.h>
#include <mutex>
#include <vector>
#include <algorithm>
#include "Spectrum.h"
#include "DouglasPeucker.h"
#include "gui_setup.h"
#include "gui_sdr.h"
#include "gui_bar.h"
#include "gui_rx.h"
#include "gui_bar.h"
#include <linux/input.h>
#include "lv_event_private.h"
#include "lv_evdev_private.h"
#include "lv_indev_private.h"
#include "lv_draw_private.h"
#include "lv_sprintf.h"
#include "screen.h"
#include "SharedQueue.h"

Spectrum SpectrumGraph;
int nfft_samples{1240};
const int excludeMargin = 12;

std::pair<bool,int> Spectrum::cursor_intersect(lv_point_t p)
{
	int i = 0;
	for (auto mark : markers)
	{
		if (mark != nullptr && abs(mark->pos.x - p.x) < 10)
			return std::make_pair(true,i);
		i++;
	}
	return std::make_pair(false,0);
}

void Spectrum::pressing_event_cb_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);

	lv_indev_t *indev = lv_indev_get_act();
	lv_indev_type_t indev_type = lv_indev_get_type(indev);

	if (indev_type == LV_INDEV_TYPE_POINTER && code == LV_EVENT_RELEASED && indev->pointer.btn_id == LV_INDEV_BTN_RIGHT)
		{
		drag_marker_rightbutton = 0;
		vfo.set_frequency_to_left(newspanstartfreq, vfo.get_active_vfo(), true);
		vfo.set_vfo(vfo.get_frequency());
		//SpectrumGraph.SetFftParts();
		gbar.updateweb();
	}

	if (indev_type == LV_INDEV_TYPE_POINTER && code == LV_EVENT_PRESSING && indev->pointer.btn_id == LV_INDEV_BTN_RIGHT)

		{
		lv_point_t p;
		lv_indev_get_point(indev, &p);

		if (indev->pointer.btn_id == LV_INDEV_BTN_RIGHT && p.x != p_drag.x)
		{
			p_drag = p;
			long long df{0LL}, spanfreq;
			int span = vfo.get_span();
			spanfreq = vfo.get_sdr_frequency(); // vfo.get_sdr_span_frequency(); // f max left
			df = p.x * (span / width);
			if (!drag_marker_rightbutton)
			{
				drag_frequency_shift = df;
				drag_marker_rightbutton = 1;
			}
			else
			{
				drag_frequency = df - drag_frequency_shift;
				drag_frequency_shift = df;
			}
			//printf("freq %lld df %lld\n", drag_frequency, df);
			newspanstartfreq = spanfreq - drag_frequency;
			vfo.set_frequency_to_left(newspanstartfreq, vfo.get_active_vfo(), false);
			vfo.set_vfo(vfo.get_frequency());
		}
		return;
	}

	//if (indev_type == LV_INDEV_TYPE_POINTER && code == LV_EVENT_RELEASED && (btn_id == BTN_LEFT || btn_id == 0))
	if (indev_type == LV_INDEV_TYPE_POINTER && code == LV_EVENT_RELEASED && (indev->pointer.btn_id == LV_INDEV_BTN_LEFT || indev->pointer.btn_id == LV_INDEV_BTN_NONE))
	{
		// make sure only the marker is dragged, fast mouse movements will skipp multiple x possitions
		drag_marker = 0;
	}

	//if (indev_type == LV_INDEV_TYPE_POINTER && code == LV_EVENT_PRESSING && (btn_id == BTN_LEFT || btn_id == 0))
	if (indev_type == LV_INDEV_TYPE_POINTER && code == LV_EVENT_PRESSING && (indev->pointer.btn_id == LV_INDEV_BTN_LEFT || indev->pointer.btn_id == LV_INDEV_BTN_NONE))
	{
		lv_point_t p;
		lv_indev_get_point(indev, &p);

		if (p.x > 0)
		{
			auto ret = cursor_intersect(p);
			if (ret.first || drag_marker) // close to marker or drag mode
			{
				if (drag_marker) // make sure the same marker is dragged
				{
					set_marker(drag_marker - 1, (data_set.size() * p.x) / width);
				}
				else
				{
					set_marker(ret.second, (data_set.size() * p.x) / width);
					drag_marker = ret.second + 1;
				}
			}
			else
			{
				long long f;
				int span = vfo.get_span();
				f = vfo.get_sdr_span_frequency();
				f = p.x * (span / width) + f;
				if (vfo.get_frequency() != f)
				{
					f = f / gbar.get_step_value();
					f = f * gbar.get_step_value();
					vfo.set_vfo(f);
				}
			}
		}
	}
}

void Spectrum::draw_event_cb_class(lv_event_t *e)
{
	lv_draw_task_t *draw_task = lv_event_get_draw_task(e);
	lv_draw_dsc_base_t *base_dsc = (lv_draw_dsc_base_t *)lv_draw_task_get_draw_dsc(draw_task);

	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);

	//if (dsc->part == LV_PART_ITEMS && dsc->sub_part_ptr == ser)
	if (base_dsc->part == LV_PART_ITEMS && lv_draw_task_get_type(draw_task) == LV_DRAW_TASK_TYPE_LINE)
	{
		lv_draw_task_t *draw_task = lv_event_get_draw_task(e);
		lv_draw_dsc_base_t *base_dsc = (lv_draw_dsc_base_t *)lv_draw_task_get_draw_dsc(draw_task);

		if (base_dsc->part == LV_PART_ITEMS && lv_draw_task_get_type(draw_task) == LV_DRAW_TASK_TYPE_LINE)
		{
			lv_obj_t *obj = lv_event_get_target_obj(e);
			lv_area_t coords;
			lv_obj_get_coords(obj, &coords);

			const lv_chart_series_t *ser = lv_chart_get_series_next(obj, NULL);
			lv_color_t ser_color = lv_chart_get_series_color(obj, ser);

			lv_draw_line_dsc_t *draw_line_dsc = (lv_draw_line_dsc_t *)lv_draw_task_get_draw_dsc(draw_task);

			/*Draw rectangle below the triangle*/
			lv_draw_rect_dsc_t rect_dsc;
			lv_draw_rect_dsc_init(&rect_dsc);
			rect_dsc.bg_color = lv_palette_main(LV_PALETTE_RED); // ser_color;
			rect_dsc.bg_opa = LV_OPA_20;

			lv_area_t rect_area;
			rect_area.x1 = (int32_t)draw_line_dsc->p1.x;
			rect_area.x2 = (int32_t)draw_line_dsc->p2.x - 1;
			rect_area.y1 = LV_MIN(draw_line_dsc->p1.y, draw_line_dsc->p2.y);
			rect_area.y2 = (int32_t)coords.y2;
			lv_draw_rect(base_dsc->layer, &rect_dsc, &rect_area);
		}
	}
	if (base_dsc->part == LV_PART_CURSOR && lv_draw_task_get_type(draw_task) == LV_DRAW_TASK_TYPE_LINE)
	{
		lv_draw_dsc_base_t *base_dsc = (lv_draw_dsc_base_t *)lv_draw_task_get_draw_dsc(draw_task);
		int i = 1;
		for (auto cursor : markers)
		{
			if (cursor != nullptr && base_dsc->id1 == i)
			{
				draw_marker_label(cursor, draw_task);
			}
			i++;
		}
		if (base_dsc->id1 == 0 && active_markers > 0)
			draw_marker_label(FrequencyCursor, draw_task);
	}
}

void Spectrum::scroll_event_cb_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
	int16_t *scroll_x = (int16_t *)lv_event_get_param(e);
	int steps;
	
	LV_LOG_INFO("Scrolled to: x=%d", *scroll_x);
	steps = *scroll_x / scroll_factor;
	guiQueue.push_back(GuiMessage(GuiMessage::action::step, steps));
}

void Spectrum::init(lv_obj_t *scr, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h, float ifrate)
{
	const int scale_size = 25;
	int hor_lines = hor_lines_large;
	int waterfallsize = Settings_file.get_int("Radio", "waterfallsize", 3);

	parent = scr;
	height = h;
	width = w;
	xx = x;
	yy = y;
	if (screenWidth < 1200)
		nfft_samples = 800;
	
	lv_point_t size;
	lv_txt_get_size(&size, "7074", LV_FONT_DEFAULT, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
	fontsize = size.y;
	cursor_txt.push_back("");
	if (waterfallsize < 0 || waterfallsize > 10)
		waterfallsize = 0;
	scroll_factor = Settings_file.get_int("input", "scroll", -15);
	if (scroll_factor == 0 || scroll_factor < -100 || scroll_factor > 100)
		scroll_factor = 1;
	lv_obj_set_style_pad_hor(parent, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(parent, 0, LV_PART_MAIN);

	/*Create a container*/
	lv_obj_t *chart_container = lv_obj_create(parent);
	lv_obj_set_pos(chart_container, x, y);
	lv_obj_set_size(chart_container, w, h);
	lv_obj_set_style_radius(chart_container, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_all(chart_container, 0, LV_PART_MAIN);
	//lv_obj_set_style_pad_ver(chart_container, 0, LV_PART_MAIN);
	//lv_obj_set_style_bg_color(chart_container, lv_color_black(), 0);
	lv_obj_clear_flag(chart_container, LV_OBJ_FLAG_SCROLLABLE);
	
	lv_style_init(&Spectrum_style);
	lv_style_set_radius(&Spectrum_style, 0);
	lv_style_set_bg_color(&Spectrum_style, lv_color_black());
	lv_style_set_line_width(&Spectrum_style, 2);

	chart = lv_chart_create(chart_container);
	lv_obj_add_style(chart, &Spectrum_style, 0);
	lv_obj_add_style(chart, &Spectrum_style, LV_PART_ITEMS);
	// lv_obj_set_style_line_width(chart, 1, LV_PART_ITEMS);
	
	heightChart = h;
	heightWaterfall = 0;
	if (waterfallsize)
	{
		lv_obj_set_pos(chart, x, fontsize);
		heightChart = h - (h * waterfallsize) / 10;
		heightWaterfall = (h * waterfallsize) / 10 - fontsize;
		if (waterfallsize == 1)
		{
			heightWaterfall = 5;
			heightChart -= 5;
		}
		lv_obj_set_size(chart, w, heightChart);
		//lv_chart_set_axis_tick(chart, LV_CHART_AXIS_SECONDARY_X, 0, 0, vert_lines, 1, true, 100);
	}
	else
	{
		lv_obj_set_pos(chart, x, fontsize);
		lv_obj_set_size(chart, w, h - fontsize);
		//lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_X, 0, 0, vert_lines, 1, true, 100);
	}
	if (waterfallsize > 3)
		hor_lines = hor_lines_small;
	lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, -50, 50);
	lv_obj_set_style_pad_all(chart, 0, LV_PART_MAIN);
	lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
	lv_obj_clear_flag(chart, LV_OBJ_FLAG_SCROLLABLE);

	//LV_CHART_AXIS_PRIMARY_X
	//lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_Y, 0, 0, 6, 1, true, 80);
	lv_chart_set_div_line_count(chart, hor_lines, vert_lines);
	//lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_X, 0, 0, vert_lines, 1, true, 100);
	lv_obj_add_event_cb(chart, draw_event_cb, LV_EVENT_DRAW_TASK_ADDED, (void *)this);
	lv_obj_add_flag(chart, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS);
	lv_obj_add_event_cb(chart, pressing_event_cb, LV_EVENT_PRESSING, (void *)this);
	lv_obj_add_event_cb(chart, pressing_event_cb, LV_EVENT_RELEASED, (void *)this);
	lv_obj_add_event_cb(chart, scroll_event_cb, LV_EVENT_SCROLL, (void *)this);
	scroll_group = lv_group_create();
	set_mouse_axis_group(scroll_group);
	lv_group_add_obj(scroll_group, chart);
	FrequencyCursor = lv_chart_add_cursor((lv_obj_t *)chart, lv_palette_main(LV_PALETTE_BLUE), (lv_dir_t)(LV_DIR_BOTTOM | LV_DIR_TOP | LV_DIR_LEFT_FIX));
	lv_obj_set_style_line_width(chart, 2, LV_PART_CURSOR);
	
	scale = lv_scale_create(chart_container);
	lv_scale_set_mode(scale, LV_SCALE_MODE_HORIZONTAL_TOP);
	lv_obj_set_size(scale, w - 40, scale_size);
	lv_obj_set_pos(scale, x + 20, 0);
	lv_scale_set_total_tick_count(scale, vert_lines);
	lv_scale_set_major_tick_every(scale, 1);
	lv_obj_set_style_line_width(scale, 0, LV_PART_MAIN);
	lv_obj_set_style_line_width(scale, 0, LV_PART_ITEMS);
	lv_obj_set_style_line_width(scale, 0, LV_PART_INDICATOR);
	lv_obj_set_style_text_align(scale, LV_TEXT_ALIGN_LEFT, LV_PART_INDICATOR);
	lv_obj_set_style_pad_all(scale, 0, LV_PART_MAIN);
	lv_obj_set_style_radius(scale, 0, LV_PART_MAIN);

	lv_obj_add_event_cb(scale, scale_event_cb, LV_EVENT_DRAW_TASK_ADDED, (void *)this);
	lv_obj_add_event_cb(scale, scale_clicked_event_cb, LV_EVENT_CLICKED, (void *)this);
	lv_obj_add_flag(scale, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS);
	lv_scale_set_range(scale, 0, 9);

	lv_obj_set_style_size(chart, 0, 0, LV_PART_INDICATOR);
	lv_obj_set_style_size(chart, 8, 8, LV_PART_CURSOR);
	lv_obj_set_style_width(chart, 100, LV_PART_CURSOR);
	lv_obj_set_style_height(chart, heightChart, LV_PART_CURSOR);
	lv_obj_set_style_bg_color(chart, lv_palette_main(LV_PALETTE_BLUE), LV_PART_CURSOR);
	lv_obj_set_style_bg_opa(chart, LV_OPA_50, LV_PART_CURSOR);

	ser = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
	for (auto &col : markers_location)
		col = 0;

	lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
	for (int i = 0; i < nfft_samples / 2; i++)
	{
		data_set.push_back(0);
		data_set_peak.push_back(0);
		data_set_nonfiltered.push_back(0);
	}
	lv_chart_set_point_count(chart, data_set.size());
	lv_chart_set_ext_y_array(chart, ser, (lv_coord_t *)data_set.data());
	avg_filter.resize(nfft_samples);
	fft = std::make_unique<FastFourier>(nfft_samples, 0, 0);
	if (waterfallsize > 0)
	{
		waterfall = std::make_unique<Waterfall>(scr, x, heightChart + fontsize, w, heightWaterfall, 0.0, down, allparts, 12);
	}
	SetFftParts();
	enable_processing = true; // saveguard race condition
}

void Spectrum::setWaterfallSize(int waterfallsize)
{
	if (waterfallsize)
	{
		int h;
		
		lv_obj_set_pos(chart, xx, yy + fontsize);
		heightChart = height - (height * waterfallsize) / 10;
		heightWaterfall = (height * waterfallsize) / 10;
		lv_obj_set_size(chart, width, heightChart);
		//lv_chart_set_axis_tick(chart, LV_CHART_AXIS_SECONDARY_X, 0, 0, vert_lines, 1, true, 100);
		lv_scale_set_total_tick_count(scale, vert_lines);
		lv_scale_set_major_tick_every(scale, 1);
		if (heightWaterfall - fontsize > 0)
			h = heightWaterfall - fontsize;
		else
		{
			h = 5;
			heightChart -= h;
		}
		if (waterfall != nullptr)
			waterfall->Size(heightChart + fontsize, h);
		else
			waterfall = std::make_unique<Waterfall>(parent, xx, heightChart + fontsize, width, h, 0.0, down, allparts, 12);
	}
	
	if (waterfallsize == 0 && waterfall != nullptr)
	{
		waterfall.reset();
		lv_obj_set_pos(chart, xx, yy);
		lv_obj_set_size(chart, width, height - fontsize);
		//lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_X, 0, 0, vert_lines, 1, true, 100);
		lv_scale_set_total_tick_count(scale, vert_lines);
		lv_scale_set_major_tick_every(scale, 1);
	}

}

void Spectrum::DrawDisplay()
{
	upload_fft();
	load_data();
	if (waterfall)
		waterfall->Draw(guirx.get_waterfallgain());
}

void Spectrum::SetFftParts()
{
	std::pair<vfo_spansetting, double> span_ex = vfo.compare_span_ex();
	if (fft)
	{
		switch (span_ex.first)
		{
		case span_is_ifrate:
			fft->SetParameters(nfft_samples);
			break;
		case span_between_ifrate:
			fft->SetParameters(nfft_samples, (float)vfo.get_span() / (float)ifrate,  0.5f * ((float)ifrate - (float)vfo.get_span()) / ifrate);
			break;
		case span_lower_halfrate:
			fft->SetParameters(nfft_samples, ((float)vfo.get_span() * 2.0f) / ifrate);
			break;
		}
	}
	
	if (waterfall)
	{
		switch (span_ex.first)
		{
		case span_is_ifrate:
			waterfall->SetPartial(allparts);
			break;
		case span_between_ifrate:
			waterfall->SetPartial(regionpart, (float)vfo.get_span() / (float)ifrate, 0.5f * ((float)ifrate - (float)vfo.get_span()) / ifrate);
			break;
		case span_lower_halfrate:
			waterfall->SetPartial(lowerpart, ((float)vfo.get_span() * 2.0f) / ifrate);
			break;
		}
	}
}

void Spectrum::ProcessWaterfall(const IQSampleVector &input)
{
	if (enable_processing)
	{
		fft->Process(input);
		if (waterfall)
			waterfall->Process(input);
	}
}

void Spectrum::disable_processing()
{
	enable_processing = false;
}

void Spectrum::set_pos(int32_t offset)
{
	int pos;
	float d;
	int span = vfo.get_span();

	if (!data_set.size())
		return;

	d = (float)(data_set.size() * ((float)(offset + vfo.get_minoffset())) / (float)span);
	pos = d - 1;
	if (pos < 0)
		pos = 0;
	if (pos >= data_set.size())
		pos = data_set.size() - 1;
	lv_chart_set_cursor_point(chart, FrequencyCursor, ser, pos);
	lv_obj_invalidate(scale);
	//	printf("Pos: freq: %ld sdr %ld offset %d pos: %d span %d \n", (long)vfo.get_frequency(), (long)vfo.get_sdr_frequency(), offset, pos, span);
}

void Spectrum::set_cursor_mode(int mode)
{
	lv_dir_t dir;

	switch (mode)
	{
	case mode_lsb:
		dir =  (lv_dir_t)(LV_DIR_BOTTOM | LV_DIR_TOP | LV_DIR_LEFT_FIX);
		break;

	case mode_usb:
	case mode_ft8:
	case mode_ft4:
	case mode_cw:
	case mode_wspr:
	case mode_echo:
	case mode_freedv:
	case mode_rtty:
		dir = (lv_dir_t)(LV_DIR_BOTTOM | LV_DIR_TOP | LV_DIR_RIGHT_FIX);
		break;

	case mode_am:
	case mode_dsb:
	case mode_broadband_fm:
	case mode_narrowband_fm:
		dir = (lv_dir_t)(LV_DIR_BOTTOM | LV_DIR_TOP | LV_DIR_MID_FIX);
		break;
	}
	
	lv_chart_set_direction(FrequencyCursor, dir);
	int32_t f = gbar.get_filter_frequency(mode);
	int32_t s = vfo.get_span();
	int32_t ii = (width - 2 * excludeMargin);
	int32_t w = f / (s / ii);
	lv_obj_set_style_width(chart, w, LV_PART_CURSOR);
	//printf("span %d filter %d, screen %d width %d\n", s, f, ii, w);
}

void Spectrum::enable_second_data_series(bool enable)
{
	hold_peak = enable;
	for (auto &value : data_set_peak)
		value = -1000;
	if (hold_peak)
	{
		peak_ser = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_PRIMARY_Y);
		for (auto m : markers)
		{
			if (m != nullptr)
				m->ser = peak_ser;
		}
	}
	else
	{
		if (peak_ser)
		{
			lv_chart_remove_series(chart, peak_ser);
			peak_ser = nullptr;
		}
		for (auto m : markers)
		{
			if (m != nullptr)
				m->ser = ser;
		}
	}
}

void Spectrum::load_data()
{
	int i = data_set.size();
	if (i > 0)
	{
		lv_chart_set_point_count(chart, i);
		lv_chart_set_ext_y_array(chart, ser, (lv_coord_t *)data_set.data());
		if (peak_ser != nullptr)
		{
			lv_chart_set_ext_y_array(chart, peak_ser, (lv_coord_t *)data_set_peak.data());
		}
		lv_chart_refresh(chart);
	}
}

void Spectrum::upload_fft()
{
	int i{0};
	int noisefloor = guirx.get_spectrumgain();
	std::pair<vfo_spansetting, double> span_ex = vfo.compare_span_ex();

	switch (span_ex.first)
	{
	case span_is_ifrate:
	case span_between_ifrate:
	{
			int32_t ii{}, value{};
			std::vector<float> fft_output = fft->GetLineatSquaredBins();
			data_set.resize(fft_output.size() / 2);
			data_set_peak.resize(fft_output.size() / 2);
			data_set_nonfiltered.resize(fft_output.size() / 2);
			finder.uploadData(fft_output);
			for (auto &col : fft_output)
			{
				value = noisefloor + (lv_coord_t)(20.0 * log10(col));
				// if (value < -100.0)
				//	value = -100.0;
				if (value > (float)s_points_max)
					value = (float)s_points_max;
				if (i % 2)
				{
					data_set_nonfiltered[ii] = value;
					data_set[ii] = avg_filter[ii](value);
					data_set_peak[ii] = std::max(data_set_peak[ii], (lv_coord_t)value);
					ii++;
				}
				i++;
			}
		}
		break;
	case span_lower_halfrate:
		{
			int value{};
			std::vector<float> fft_output = fft->GetSquaredBins();
			data_set.resize(fft_output.size() / 2);
			data_set_peak.resize(fft_output.size() / 2);
			data_set_nonfiltered.resize(fft_output.size() / 2);
			finder.uploadData(fft_output);
			for (auto &col : fft_output)
			{
				if (i == (fft_output.size() / 2))
					break;
				value = noisefloor + (lv_coord_t)(20.0 * log10(col));
				//if (value < -100.0)
				//	value = -100.0;
				if (value > (float)s_points_max)
					value = (float)s_points_max;
				data_set[i] = avg_filter[i](value);
				data_set_peak[i] = std::max(data_set_peak[i], (lv_coord_t)value);
				data_set_nonfiltered[i] = value;
				i++;
			}
		}
	break;
	}
}

float Spectrum::getSuppression()
{
	return finder.GetSuppression();
}

void Spectrum::enable_marker(int marker, bool enable)
{
	if (marker < markers.size())
	{
		if (markers[marker] == nullptr && enable)
		{
			markers[marker] = lv_chart_add_cursor(chart, lv_palette_main(LV_PALETTE_LIME), (lv_dir_t)(LV_DIR_BOTTOM | LV_DIR_TOP));
			lv_chart_set_cursor_point(chart, markers[marker], (peak_ser == nullptr) ? ser : peak_ser, markers_location[marker]);
			active_markers++;
			if (cursor_txt.size() < (active_markers + 1))
				cursor_txt.push_back("");
		}
		if (markers[marker] != nullptr && !enable)
		{
			lv_chart_remove_cursor(chart, markers[marker]);
			markers[marker] = nullptr;
			active_markers--;
		}
	}
}

void Spectrum::set_marker(int marker, int32_t offset)
{
	if (marker < markers.size())
	{
		markers_location[marker] = offset;
		if (markers[marker] != nullptr)
			lv_chart_set_cursor_point(chart, markers[marker], (peak_ser == nullptr) ? ser : peak_ser, markers_location[marker]);
	}
}

void Spectrum::scale_event_cb_class(lv_event_t *e)
{
	lv_draw_task_t *draw_task = lv_event_get_draw_task(e);
	lv_draw_dsc_base_t *base_dsc = (lv_draw_dsc_base_t *)lv_draw_task_get_draw_dsc(draw_task);
		

	if (base_dsc->part == LV_PART_INDICATOR && lv_draw_task_get_type(draw_task) == LV_DRAW_TASK_TYPE_LABEL)
	{
		lv_draw_label_dsc_t *label_dsc = lv_draw_task_get_label_dsc(draw_task);

		if (label_dsc && label_dsc->text)
		{
			std::pair<vfo_spansetting, double> span_ex = vfo.compare_span_ex();
			int span = guisdr.get_span();
			int ii;
			double offset{0}, f{};

			switch (span_ex.first)
			{
			case span_is_ifrate:
				f = (double)vfo.get_sdr_frequency() - (double)(span / 2.0);
				ii = span / (vert_lines - 1);
				break;
			case span_between_ifrate:
				f = (double)vfo.get_sdr_frequency() - (double)vfo.get_minoffset() ;
				ii = span / (vert_lines - 1);
				break;
			case span_lower_halfrate:
				offset = vfo.get_vfo_offset() / span;
				f = (double)vfo.get_sdr_frequency() + offset * (double)span;
				ii = span / (vert_lines - 1);
				break;
			}
			f = f + (label_dsc->base.id2) * ii;
			long l = (long)round(f / 1000.0);
			char str[80];
			lv_snprintf(str, 19, "%ld", l);
			lv_free((void *)label_dsc->text);
			label_dsc->text_length = strlen(str);
			label_dsc->text = lv_strndup(str, label_dsc->text_length);
			
			lv_point_t size;
			lv_txt_get_size(&size, (char *)label_dsc->text, LV_FONT_DEFAULT, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
			draw_task->area.x2 += size.x;
			if (draw_task->area.x1 > (size.x / 2))
			{
				draw_task->area.x2 += size.x / 2;
				draw_task->area.x1 -= size.x / 2;
			}
		}
	}
}

void Spectrum::scale_clicked_event_cb_class(lv_event_t *e)
{
	lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
	lv_point_t p;

	lv_indev_t *indev = lv_indev_get_act();
	lv_indev_type_t indev_type = lv_indev_get_type(indev);
	if (indev_type == LV_INDEV_TYPE_POINTER)
	{
		lv_indev_get_point(indev, &p);
		if (p.x > 90 * width / 100)
			vfo.setVfoFrequency(1);
		if (p.x < 10 * width / 100)
			vfo.setVfoFrequency(-1);

		lv_obj_invalidate(obj);
	}
}

void Spectrum::draw_marker_label(lv_chart_cursor_t *cursor, lv_draw_task_t *draw_task)
{
	lv_draw_dsc_base_t *base_dsc = (lv_draw_dsc_base_t *)lv_draw_task_get_draw_dsc(draw_task);
	lv_draw_line_dsc_t *draw_line_dsc = (lv_draw_line_dsc_t *)lv_draw_task_get_draw_dsc(draw_task);
	lv_coord_t *data_array = lv_chart_get_y_array(chart, cursor->ser);
	lv_coord_t v = data_array[cursor->point_id];
	char buf[16];

	lv_snprintf(buf, sizeof(buf), "%d db", v);
	cursor_txt.at(base_dsc->id1) = std::string(buf);
	
	lv_point_t size;
	lv_txt_get_size(&size, buf, LV_FONT_DEFAULT, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);

	lv_area_t a;

	a.x1 = (int32_t)draw_line_dsc->p1.x + 10;
	a.x2 = a.x1 + size.x + 10;

	a.y1 = (int32_t)draw_line_dsc->p1.y + 10;
	a.y2 = a.y1 + size.y + 10;

	lv_draw_label_dsc_t draw_label_dsc;
	lv_draw_label_dsc_init(&draw_label_dsc);
	draw_label_dsc.color = lv_color_white();
	draw_label_dsc.text = cursor_txt.at(base_dsc->id1).c_str();
	a.x1 += 5;
	a.x2 -= 5;
	a.y1 += 5;
	a.y2 -= 5;
	lv_draw_label(base_dsc->layer, &draw_label_dsc, &a);
}