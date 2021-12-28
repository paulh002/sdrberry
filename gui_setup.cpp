#include "gui_setup.h"

gui_setup	gsetup;
extern 		void switch_sdrreceiver(std::string receiver);

static void receivers_button_handler(lv_event_t * e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e); 
	if (code == LV_EVENT_VALUE_CHANGED) 
	{
		char	buf[80];
		std::string receiver;
		
		receiver.reserve(80);
		int selection = lv_dropdown_get_selected(obj);
		lv_dropdown_get_selected_str(obj,buf, 79);
		switch_sdrreceiver(std::string(buf));
	}
}

static void span_slider_event_cb(lv_event_t * e)
{
	char buf[20]; 
	
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e); 

	int i = lv_slider_get_value(obj) * 50;
	if (i > 0)
	{		
		gsetup.set_span_value(i * 1000);
	}
}
	

int gui_setup::get_sample_rate(int rate)
{
	if (rate >= 0 && rate < sample_rates.size())
	{
		return sample_rates.at(rate);
	}
	return 0;
}

static void samplerate_button_handler(lv_event_t * e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e); 
	if (code == LV_EVENT_VALUE_CHANGED) 
	{
		int rate = lv_dropdown_get_selected(obj);
		ifrate = gsetup.get_sample_rate(rate);
		gsetup.m_ifrate = ifrate;
		vfo.vfo_re_init((long)ifrate, pcmrate);
		stop_rxtx();
		try
		{
			SdrDevices.SdrDevices.at(default_radio)->setSampleRate(SOAPY_SDR_RX, default_rx_channel, ifrate);
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << endl;
			return;
		}
		gsetup.set_span_range(ifrate/2);
		gsetup.set_span_value(ifrate/2);
		select_mode(mode);
	}
}

void gui_setup::add_sample_rate(int samplerate)
{
	char	str[30];
	
	sample_rates.push_back(samplerate);
	sprintf(str, "%d Khz", samplerate / 1000);
	lv_dropdown_add_option(d_samplerate, str, LV_DROPDOWN_POS_LAST);
}

void gui_setup::set_sample_rate(int rate)
{
	int i;
	for (i = 0; i < sample_rates.size(); i++)
	{
		if (sample_rates[i] == rate)
			break;
	}
	lv_dropdown_set_selected(d_samplerate, i);
}

void gui_setup::clear_sample_rate()
{
	sample_rates.clear();
	lv_dropdown_clear_options(d_samplerate);
}

void gui_setup::set_radio(std::string name)
{
	int i = SdrDevices.get_device_index(name);
	lv_dropdown_set_selected(d_receivers, i);
}

void gui_setup::init(lv_obj_t* o_tab, lv_coord_t w)
{

	const lv_coord_t x_margin  = 10;
	const lv_coord_t y_margin  = 5;
	const int x_number_buttons = 5;
	const int y_number_buttons = 4;
	const lv_coord_t tab_margin  = 20;
	
	int button_width_margin = ((w - tab_margin) / x_number_buttons);
	int button_width = ((w - tab_margin) / x_number_buttons) - x_margin;
	int button_height = 50;
	int button_height_margin = button_height + y_margin;
	int	ibutton_x = 0, ibutton_y = 0;
	m_ifrate = ifrate;
	
	lv_style_init(&style_btn);
	lv_style_set_radius(&style_btn, 10);
	lv_style_set_bg_color(&style_btn, lv_color_make(0x60, 0x60, 0x60));
	lv_style_set_bg_grad_color(&style_btn, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_bg_grad_dir(&style_btn, LV_GRAD_DIR_VER);
	lv_style_set_bg_opa(&style_btn, 255);
	lv_style_set_border_color(&style_btn, lv_color_make(0x9b, 0x36, 0x36)); // lv_color_make(0x2e, 0x44, 0xb2)
	lv_style_set_border_width(&style_btn, 2);
	lv_style_set_border_opa(&style_btn, 255);
	lv_style_set_outline_color(&style_btn, lv_color_black());
	lv_style_set_outline_opa(&style_btn, 255);
	lv_obj_clear_flag(o_tab, LV_OBJ_FLAG_SCROLLABLE);
	
	d_samplerate = lv_dropdown_create(o_tab);
	lv_obj_align(d_samplerate, LV_ALIGN_TOP_LEFT, 0, y_margin + ibutton_y * button_height_margin);
	lv_dropdown_clear_options(d_samplerate);
	lv_obj_add_event_cb(d_samplerate, samplerate_button_handler, LV_EVENT_VALUE_CHANGED, NULL);
	
	d_receivers = lv_dropdown_create(o_tab);
	lv_obj_align(d_receivers, LV_ALIGN_TOP_LEFT, button_width_margin, y_margin + ibutton_y * button_height_margin);
	lv_dropdown_clear_options(d_receivers);
	lv_obj_add_event_cb(d_receivers, receivers_button_handler, LV_EVENT_VALUE_CHANGED, NULL);
	std::string def = Settings_file.find_sdr("default");
	for (auto& col : Settings_file.receivers)
	{
		lv_dropdown_add_option(d_receivers, col.c_str(), LV_DROPDOWN_POS_LAST);
	}
	
	
	int span_y = 15 + y_margin + button_height_margin;
	span_slider = lv_slider_create(o_tab);
	lv_obj_set_width(span_slider, w / 2 - 50); 
	lv_obj_center(span_slider);
	lv_obj_add_event_cb(span_slider, span_slider_event_cb, LV_EVENT_PRESSING, NULL);

	span_slider_label = lv_label_create(o_tab);
	lv_label_set_text(span_slider_label, "span");
	lv_obj_align_to(span_slider_label, span_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	
	string span = Settings_file.find_radio("span");
	int i = atoi(span.c_str());
	set_span_range(ifrate/2);
	set_span_value(i * 1000);
}

void gui_setup::set_span_value(int span)
{
	char	buf[30];
	
	int maxv = lv_slider_get_max_value(span_slider);
	int v = span / 50000;
	
	if(v < 0 || v > maxv)
		span = maxv;
	if (v > 0)
	{	// the highest span is limited by ifrate/2
		if (((m_ifrate / 2) - (double)span) < 0.1)
		{
			lv_slider_set_value(span_slider, maxv, LV_ANIM_ON);
			span = m_ifrate / 2;
			sprintf(buf, "span %d Khz", span / 1000);
			gui_vfo_inst.set_span(span / 1000);			
		}
		else
		{
			lv_slider_set_value(span_slider, v, LV_ANIM_ON);
			sprintf(buf, "span %d Khz", v * 50);
			gui_vfo_inst.set_span(v * 50);
		}
	}
	else
	{
		lv_slider_set_value(span_slider, 1, LV_ANIM_ON);
		sprintf(buf, "span %d Khz", span / 1000);		
		gui_vfo_inst.set_span(span / 1000);
	}
	lv_label_set_text(span_slider_label, buf);
	// store in atomic<int> so demodulator thread can request it
	m_span.store(span);	
}

void gui_setup::set_span_range(int span)
{
	char	buf[30];
	
	int v = span / 50000;
	int m = span % 50000;
	if (v < 0 || v > 80)
		span = 80;
	if (v == 0)
		span = 1;
	if (m > 0)
		v++;
	lv_slider_set_range(span_slider, 1, v);
}