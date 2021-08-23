#include "lvgl/lvgl.h"
#include "gui_right_pane.h"
#include "sdrberry.h"
#include <alsa/asoundlib.h>
#include "AudioOutput.h"
#include "devices.h"

static lv_style_t style_btn;

lv_obj_t*	bUsb, *bLsb, *bAM, *bFM, *bCW, *bFT8, *bg_right; 
lv_obj_t*	agc_slider, *agc_slider_label;
lv_obj_t*	gain_slider, *gain_slider_label;
lv_obj_t*	vol_slider, *vol_slider_label;
lv_obj_t*	fil_slider, *fil_slider_label;

static const int nobuttons = 4;
static const int bottombutton_width = (rightWidth / nobuttons) - 2;
static const int bottombutton_width1 = (rightWidth / nobuttons);
static int button_height , button_margin = 18;
const int top_y {20};

static void gain_slider_event_cb(lv_event_t * e);
static void agc_slider_event_cb(lv_event_t * e);
static void vol_slider_event_cb(lv_event_t * e);
static void mode_button_event(lv_event_t * e);
static void fil_slider_event_cb(lv_event_t * e);

	
void	setup_right_pane(lv_obj_t* scr )
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
    lv_label_set_text(gain_slider_label, buf);
	lv_obj_align(gain_slider_label, LV_ALIGN_TOP_MID, 0, top_y);
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
	lv_label_set_text(fil_slider_label, buf);
	lv_obj_align_to(fil_slider_label, vol_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);	
	select_filter((int)lv_slider_get_value(slider));
}

void set_filter_slider(int filter)
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
	lv_label_set_text(agc_slider_label, buf);
	lv_obj_align_to(agc_slider_label, gain_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
}

void set_gain_range(int min, int max)
{
	lv_slider_set_range(gain_slider, min, max);
}

void set_gain_slider(int gain)
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

void step_gain_slider(int step)
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

void set_gain_range()
{
	int max_gain = (int)soapy_devices[0].channel_structure_rx[soapy_devices[0].rx_channel].full_gain_range.maximum();
	int min_gain = (int)soapy_devices[0].channel_structure_rx[soapy_devices[0].rx_channel].full_gain_range.minimum();
	lv_slider_set_range(gain_slider, min_gain, max_gain);
}

void hide_agc_slider(void)
{
	lv_obj_add_flag(agc_slider, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(agc_slider_label, LV_OBJ_FLAG_HIDDEN);
}

static void vol_slider_event_cb(lv_event_t * e)
{
	char buf[20];
	
	lv_obj_t * slider = lv_event_get_target(e);
	sprintf(buf, "volume %d", lv_slider_get_value(slider));
	lv_label_set_text(vol_slider_label, buf);
	lv_obj_align_to(vol_slider_label, agc_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	audio_output->set_volume(lv_slider_get_value(slider));
}

void step_vol_slider(int step)
{
	set_vol_slider(lv_slider_get_value(vol_slider) + step);
}

void set_vol_slider(int volume)
{
	if (volume < 0)
		volume = 0;
	if (volume > 100)
		volume = 100;
	lv_slider_set_value(vol_slider, volume, LV_ANIM_ON);
	
	char buf[20];
	
	sprintf(buf, "volume %d", volume);
	lv_label_set_text(vol_slider_label, buf);
	lv_obj_align_to(vol_slider_label, agc_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	audio_output->set_volume(volume);
}