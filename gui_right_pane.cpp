#include "lvgl/lvgl.h"
#include "gui_right_pane.h"
#include "sdrberry.h"
#include <alsa/asoundlib.h>
#include "AudioOutput.h"
#include "devices.h"
#include <functional>



const int top_y {20};
const int max_volume {100};
const int max_agc {4};
const int min_agc {0};


static void gain_slider_event_cb(lv_event_t * e);
static void agc_slider_event_cb(lv_event_t * e);
static void vol_slider_event_cb(lv_event_t * e);
static void mode_button_event(lv_event_t * e);
static void fil_slider_event_cb(lv_event_t * e);

Gui_right_pane	grp;
	
void	Gui_right_pane::setup_right_pane(lv_obj_t* scr)
{
	static lv_style_t right_style;
		
	lv_style_init(&right_style);
	lv_style_set_radius(&right_style, 0);
	//lv_style_set_bg_color(&right_style, lv_palette_main(LV_PALETTE_GREEN));
	
	bg_right = lv_obj_create(scr);
	lv_obj_add_style(bg_right, &right_style, 0);
	lv_obj_set_pos(bg_right, LV_HOR_RES - (rightWidth + 3), topHeight + tunerHeight);
	lv_obj_set_size(bg_right, rightWidth - 3, screenHeight - topHeight - tunerHeight);
	lv_obj_clear_flag(bg_right, LV_OBJ_FLAG_SCROLLABLE);
	
	gain_slider_label = lv_label_create(bg_right);
	lv_label_set_text(gain_slider_label, "gain 0 db");
	lv_obj_align(gain_slider_label, LV_ALIGN_TOP_MID, 0, top_y);
	gain_slider = lv_slider_create(bg_right);
	lv_obj_set_width(gain_slider, rightWidth - 40); 
	lv_slider_set_range(gain_slider, 0, 100);
	lv_obj_align_to(gain_slider, gain_slider_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	lv_obj_add_event_cb(gain_slider, gain_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

	agc_slider_label = lv_label_create(bg_right);
	lv_label_set_text(agc_slider_label, "agc offb");
	lv_obj_align_to(agc_slider_label, gain_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	agc_slider = lv_slider_create(bg_right);
	lv_slider_set_range(agc_slider, 0, 3);
	lv_obj_set_width(agc_slider, rightWidth - 40); 
	lv_obj_align_to(agc_slider, agc_slider_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	// lv_obj_center(agc_slider);
	lv_obj_add_event_cb(agc_slider, agc_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	set_agc_slider(Settings_file.agc());
	
	vol_slider_label = lv_label_create(bg_right);
	lv_label_set_text(vol_slider_label, "volume");
	lv_obj_align_to(vol_slider_label, agc_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	vol_slider = lv_slider_create(bg_right);
	lv_slider_set_range(vol_slider, 0, 100);
	lv_obj_set_width(vol_slider, rightWidth - 40); 
	lv_obj_align_to(vol_slider, vol_slider_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	lv_obj_add_event_cb(vol_slider, vol_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

	fil_slider_label = lv_label_create(bg_right);
	lv_label_set_text(fil_slider_label, "3.5 Khz");
	lv_obj_align_to(fil_slider_label, vol_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	
	fil_slider = lv_slider_create(bg_right);
	lv_slider_set_range(fil_slider, 0, 7);
	lv_obj_set_width(fil_slider, rightWidth - 40); 
	lv_obj_align_to(fil_slider, fil_slider_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	lv_obj_add_event_cb(fil_slider, fil_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	lv_slider_set_value(fil_slider, 5, LV_ANIM_ON);

}

static void gain_slider_event_cb(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target(e);
    char buf[20];
	sprintf(buf,"gain %d db", lv_slider_get_value(slider));
	lv_label_set_text(grp.gain_slider_label, buf);
	lv_obj_align(grp.gain_slider_label, LV_ALIGN_TOP_MID, 0, top_y);
	soapy_devices[0].sdr->setGain(SOAPY_SDR_RX, soapy_devices[0].rx_channel, lv_slider_get_value(slider));
}

static void fil_slider_event_cb(lv_event_t * e)
{
	lv_obj_t * slider = lv_event_get_target(e);
	char buf[20];
	
	
	switch (lv_slider_get_value(slider))
	{
	case 0:
		strcpy(buf, "0.5 Khz");
		break;
	case 1:
		strcpy(buf, "1 Khz");
		break;
	case 2:
		strcpy(buf, "1.5 Khz");
		break;
	case 3:
		strcpy(buf, "2 Khz");
		break;
	case 4:
		strcpy(buf, "2.5 Khz");
		break;
	case 5:
		strcpy(buf, "3 Khz");
		break;		
	case 6:
		strcpy(buf, "3.5 Khz");
		break;		
	case 7:
		strcpy(buf, "4 Khz");
		break;
	}
	lv_label_set_text(grp.fil_slider_label, buf);
	lv_obj_align_to(grp.fil_slider_label, grp.vol_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);	
	select_filter((int)lv_slider_get_value(slider));
}


void Gui_right_pane::set_filter_slider(int filter)
{
	if (filter < 0 || filter > 6) 
		filter = 0;
	lv_slider_set_value(fil_slider, filter, LV_ANIM_ON);
	char buf[20];
	switch (lv_slider_get_value(fil_slider))
	{
	case 0:
		strcpy(buf, "0.5 Khz");
		break;
	case 1:
		strcpy(buf, "1 Khz");
		break;
	case 2:
		strcpy(buf, "1.5 Khz");
		break;
	case 3:
		strcpy(buf, "2 Khz");
		break;
	case 4:
		strcpy(buf, "2.5 Khz");
		break;
	case 5:
		strcpy(buf, "3 Khz");
		break;		
	case 6:
		strcpy(buf, "3.5 Khz");
		break;		
	case 7:
		strcpy(buf, "4 Khz");
		break;
	}
	lv_label_set_text(fil_slider_label, buf);
	lv_obj_align_to(fil_slider_label, vol_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	select_filter(filter);
}

void Gui_right_pane::set_agc_slider(int pos)
{
	if (pos > max_agc)
		pos = max_agc;
	if (pos < min_agc)
		pos = min_agc;
	char buf[20];
	switch (pos)
	{
	case 0:
		strcpy(buf, "agc off");
		break;
	case 1:
		strcpy(buf, "agc fast");
		break;
	case 2:
		strcpy(buf, "agc medium");
		break;
	case 3:
		strcpy(buf, "agc slow");
		break;		
	}
	lv_label_set_text(agc_slider_label, buf);
	lv_obj_align_to(agc_slider_label, gain_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	lv_slider_set_value(agc_slider, pos, LV_ANIM_ON); 
}

int Gui_right_pane::get_agc_slider()
{
	return lv_slider_get_value(agc_slider);
}

static void agc_slider_event_cb(lv_event_t * e)
{
	lv_obj_t * slider = lv_event_get_target(e);
	char buf[20];
	switch (lv_slider_get_value(slider))
	{
		case 0:
			strcpy(buf, "agc off");
			break;
		case 1:
			strcpy(buf, "agc fast");
			break;
		case 2:
			strcpy(buf, "agc medium");
			break;
		case 3:
			strcpy(buf, "agc slow");
			break;		
	}
	lv_label_set_text(grp.agc_slider_label, buf);
	lv_obj_align_to(grp.agc_slider_label, grp.gain_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
}

void Gui_right_pane::set_gain_range(int min, int max)
{
	lv_slider_set_range(gain_slider, min, max);
}

void Gui_right_pane::set_gain_slider(int gain)
{
	char buf[20];
	
	double max_gain = soapy_devices[0].channel_structure_rx[soapy_devices[0].rx_channel].full_gain_range.maximum();
	if (gain > max_gain)
		gain = max_gain;
	
	sprintf(buf, "gain %d db", gain);
	lv_label_set_text(gain_slider_label, buf);
	lv_obj_align(gain_slider_label, LV_ALIGN_TOP_MID, 0, top_y);		
	lv_slider_set_value(gain_slider, gain, LV_ANIM_ON); 
	soapy_devices[0].sdr->setGain(SOAPY_SDR_RX, 0, (double)gain);
}

void Gui_right_pane::step_gain_slider(int step)
{
	char buf[20];
	
	int gain = lv_slider_get_value(gain_slider) + step;
	int max_gain = (int)soapy_devices[0].channel_structure_rx[soapy_devices[0].rx_channel].full_gain_range.maximum();
	int min_gain = (int)soapy_devices[0].channel_structure_rx[soapy_devices[0].rx_channel].full_gain_range.minimum();
	if (gain > max_gain)
		gain = max_gain;
	if (gain < min_gain)
		gain = min_gain;
	sprintf(buf, "gain %d db", gain);
	lv_label_set_text(gain_slider_label, buf);
	lv_obj_align(gain_slider_label, LV_ALIGN_TOP_MID, 0, top_y);		
	lv_slider_set_value(gain_slider, gain, LV_ANIM_ON); 
	soapy_devices[0].sdr->setGain(SOAPY_SDR_RX, 0, (double)gain);
}

void Gui_right_pane::set_gain_range()
{
	int max_gain = (int)soapy_devices[0].channel_structure_rx[soapy_devices[0].rx_channel].full_gain_range.maximum();
	int min_gain = (int)soapy_devices[0].channel_structure_rx[soapy_devices[0].rx_channel].full_gain_range.minimum();
	lv_slider_set_range(gain_slider, min_gain, max_gain);
}

void Gui_right_pane::hide_agc_slider(void)
{
	lv_obj_add_flag(agc_slider, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(agc_slider_label, LV_OBJ_FLAG_HIDDEN);
}

static void vol_slider_event_cb(lv_event_t * e)
{
	char buf[20];
	
	lv_obj_t * slider = lv_event_get_target(e);
	sprintf(buf, "volume %d", lv_slider_get_value(slider));
	lv_label_set_text(grp.vol_slider_label, buf);
	lv_obj_align_to(grp.vol_slider_label, grp.agc_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	audio_output->set_volume(lv_slider_get_value(slider));
}

void Gui_right_pane::step_vol_slider(int step)
{
	set_vol_slider(lv_slider_get_value(vol_slider) + step);
}

void Gui_right_pane::set_vol_slider(int volume)
{
	if (volume < 0)
		volume = 0;
	if (volume > max_volume)
		volume = max_volume;
	lv_slider_set_value(vol_slider, volume, LV_ANIM_ON);
	
	char buf[20];
	
	sprintf(buf, "volume %d", volume);
	lv_label_set_text(vol_slider_label, buf);
	lv_obj_align_to(vol_slider_label, agc_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	audio_output->set_volume(volume);
}

int Gui_right_pane::get_vol_range()
{
	return max_volume;
}

void Gui_right_pane::get_gain_range(int &max_gain, int &min_gain)
{
	max_gain = (int)soapy_devices[0].channel_structure_rx[soapy_devices[0].rx_channel].full_gain_range.maximum();
	min_gain = (int)soapy_devices[0].channel_structure_rx[soapy_devices[0].rx_channel].full_gain_range.minimum();
	return ;
}


void Gui_right_pane::get_filter_range(vector<string> &filters)
{
	filters.push_back("0.5 Khz");
	filters.push_back("1 Khz");
	filters.push_back("1.5 Khz");
	filters.push_back("2 Khz");
	filters.push_back("2.5 Khz");
	filters.push_back("3 Khz");
	filters.push_back("3.5 Khz");
	filters.push_back("4 Khz");
}