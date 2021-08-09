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
	
	lv_style_init(&style_btn);

	lv_style_set_radius(&style_btn, 10);
	lv_style_set_bg_color(&style_btn, lv_color_make(0x60, 0x60, 0x60));
	lv_style_set_bg_grad_color(&style_btn, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_bg_grad_dir(&style_btn, LV_GRAD_DIR_VER);
	lv_style_set_bg_opa(&style_btn, 255);
	lv_style_set_border_color(&style_btn, lv_color_make(0x9b, 0x36, 0x36));   // lv_color_make(0x2e, 0x44, 0xb2)
	lv_style_set_border_width(&style_btn, 2);
	lv_style_set_border_opa(&style_btn, 255);
	lv_style_set_outline_color(&style_btn, lv_color_black());
	lv_style_set_outline_opa(&style_btn, 255);
	
	bUsb = lv_btn_create(bg_right);
	lv_obj_set_user_data(bUsb, (void *)mode_usb);
	lv_obj_add_style(bUsb, &style_btn, 0); 
	lv_obj_add_event_cb(bUsb, mode_button_event, LV_EVENT_CLICKED, NULL);
	lv_obj_align(bUsb, LV_ALIGN_TOP_LEFT, 0 * bottombutton_width1, button_margin);
	//lv_btn_set_checkable(vfo1_button, true);
	//lv_btn_toggle(vfo1_button);
	lv_obj_set_size(bUsb, bottombutton_width, bottomHeight);
	lv_obj_t*  label = lv_label_create(bUsb);
	lv_label_set_text(label, "Usb");
	lv_obj_center(label);
	
	bLsb = lv_btn_create(bg_right);
	lv_obj_set_user_data(bLsb, (void *)mode_lsb);
	lv_obj_add_style(bLsb, &style_btn, 0); 
	lv_obj_add_event_cb(bLsb, mode_button_event, LV_EVENT_CLICKED, NULL);
	lv_obj_align(bLsb, LV_ALIGN_TOP_LEFT, 1 * bottombutton_width1, button_margin);
	//lv_btn_set_checkable(vfo1_button, true);
	//lv_btn_toggle(vfo1_button);
	lv_obj_set_size(bLsb, bottombutton_width, bottomHeight);
	label = lv_label_create(bLsb);
	lv_label_set_text(label, "Lsb");
	lv_obj_center(label);
	
	button_height = bottomHeight + button_margin + 4;  //lv_obj_get_height(bLsb);
	
	bCW = lv_btn_create(bg_right);
	lv_obj_set_user_data(bCW, (void *)mode_cw);
	lv_obj_add_style(bCW, &style_btn, 0); 
	lv_obj_add_event_cb(bCW, mode_button_event, LV_EVENT_CLICKED, NULL);
	lv_obj_align(bCW, LV_ALIGN_TOP_LEFT, 2 * bottombutton_width1, button_margin);
	//lv_btn_set_checkable(vfo1_button, true);
	//lv_btn_toggle(vfo1_button);
	lv_obj_set_size(bCW, bottombutton_width, bottomHeight);
	label = lv_label_create(bCW);
	lv_label_set_text(label, "CW");
	lv_obj_center(label);
	
	bFM = lv_btn_create(bg_right);
	lv_obj_set_user_data(bFM, (void *)mode_broadband_fm);
	lv_obj_add_style(bFM, &style_btn, 0); 
	lv_obj_add_event_cb(bFM, mode_button_event, LV_EVENT_CLICKED, NULL);
	lv_obj_align(bFM, LV_ALIGN_TOP_LEFT, 0 * bottombutton_width1, button_height);
	//lv_btn_set_checkable(vfo1_button, true);
	//lv_btn_toggle(vfo1_button);
	lv_obj_set_size(bFM, bottombutton_width, bottomHeight);
	label = lv_label_create(bFM);
	lv_label_set_text(label, "FM");
	lv_obj_center(label);
	
	bAM = lv_btn_create(bg_right);
	lv_obj_set_user_data(bAM, (void *)mode_am);
	lv_obj_add_style(bAM, &style_btn, 0); 
	lv_obj_add_event_cb(bAM, mode_button_event, LV_EVENT_CLICKED, NULL);
	lv_obj_align(bAM, LV_ALIGN_TOP_LEFT, 1 * bottombutton_width1, button_height);
	//lv_btn_set_checkable(vfo1_button, true);
	//lv_btn_toggle(vfo1_button);
	lv_obj_set_size(bAM, bottombutton_width, bottomHeight);
	label = lv_label_create(bAM);
	lv_label_set_text(label, "AM");
	lv_obj_center(label);
	
	bFT8 = lv_btn_create(bg_right);
	lv_obj_set_user_data(bFT8, (void *)mode_ft8);
	lv_obj_add_style(bFT8, &style_btn, 0); 
	lv_obj_add_event_cb(bFT8, mode_button_event, LV_EVENT_CLICKED, NULL);
	lv_obj_align(bFT8, LV_ALIGN_TOP_LEFT, 2 * bottombutton_width1, button_height);
	//lv_btn_set_checkable(vfo1_button, true);
	//lv_btn_toggle(vfo1_button);
	lv_obj_set_size(bFT8, bottombutton_width, bottomHeight);
	label = lv_label_create(bFT8);
	lv_label_set_text(label, "FT8");
	lv_obj_center(label);
	
	label = lv_label_create(bg_right);
	lv_label_set_text(label, "Mode");
	lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 0);
	lv_obj_clear_flag(label, LV_OBJ_FLAG_SCROLLABLE);
	
	
	gain_slider = lv_slider_create(bg_right);
	lv_obj_set_width(gain_slider, rightWidth - 40); 
	lv_obj_center(gain_slider);
	lv_obj_add_event_cb(gain_slider, gain_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	gain_slider_label = lv_label_create(bg_right);
	lv_label_set_text(gain_slider_label, "gain 0 db");
	lv_obj_align_to(gain_slider_label, gain_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	
	agc_slider = lv_slider_create(bg_right);
	lv_slider_set_range(agc_slider, 0, 3);
	lv_obj_set_width(agc_slider, rightWidth - 40); 
	lv_obj_align_to(agc_slider, gain_slider_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	// lv_obj_center(agc_slider);
	lv_obj_add_event_cb(agc_slider, agc_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	agc_slider_label = lv_label_create(bg_right);
	lv_label_set_text(agc_slider_label, "agc offb");
	lv_obj_align_to(agc_slider_label, agc_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	
	vol_slider = lv_slider_create(bg_right);
	lv_slider_set_range(vol_slider, 0, 100);
	lv_obj_set_width(vol_slider, rightWidth - 40); 
	lv_obj_align_to(vol_slider, agc_slider_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	// lv_obj_center(agc_slider);
	lv_obj_add_event_cb(vol_slider, vol_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	vol_slider_label = lv_label_create(bg_right);
	lv_label_set_text(vol_slider_label, "volume");
	lv_obj_align_to(vol_slider_label, vol_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	
	fil_slider = lv_slider_create(bg_right);
	lv_slider_set_range(fil_slider, 0, 5);
	lv_obj_set_width(fil_slider, rightWidth - 40); 
	lv_obj_align_to(fil_slider, vol_slider_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	// lv_obj_center(agc_slider);
	lv_obj_add_event_cb(fil_slider, fil_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	fil_slider_label = lv_label_create(bg_right);
	lv_label_set_text(fil_slider_label, "3.5 Khz");
	lv_obj_align_to(fil_slider_label, fil_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	lv_slider_set_value(fil_slider, 5, LV_ANIM_ON);

}

static void gain_slider_event_cb(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target(e);
    char buf[20];
    lv_snprintf(buf, sizeof(buf), "gain %d db", lv_slider_get_value(slider));
    lv_label_set_text(gain_slider_label, buf);
    lv_obj_align_to(gain_slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
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
		strcpy(buf, "1.5 Khz");
		break;
	case 2:
		strcpy(buf, "2 Khz");
		break;
	case 3:
		strcpy(buf, "2.5 Khz");
		break;
	case 4:
		strcpy(buf, "3 Khz");
		break;		
	case 5:
		strcpy(buf, "3.5 Khz");
		break;		
	}
	lv_label_set_text(fil_slider_label, buf);
	lv_obj_align_to(fil_slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
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
	lv_obj_align_to(agc_slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
}

void set_gain_range(int min, int max)
{
	lv_slider_set_range(gain_slider, min, max);
}

void set_gain_slider(int gain)
{
	lv_slider_set_value(gain_slider, gain, LV_ANIM_ON); 
	char buf[20];
	lv_snprintf(buf, sizeof(buf), "gain %d db", gain);
	lv_label_set_text(gain_slider_label, buf);
	lv_obj_align_to(gain_slider_label, gain_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);		
	soapy_devices[0].sdr->setGain(SOAPY_SDR_RX, 0, (double)gain);
}

void hide_agc_slider(void)
{
	lv_obj_add_flag(agc_slider, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(agc_slider_label, LV_OBJ_FLAG_HIDDEN);
}



static void vol_slider_event_cb(lv_event_t * e)
{
	lv_obj_t * slider = lv_event_get_target(e);
	char buf[20];
	lv_snprintf(buf, sizeof(buf), "volume %d", lv_slider_get_value(slider));
	lv_label_set_text(vol_slider_label, buf);
	lv_obj_align_to(vol_slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	audio_output->set_volume(lv_slider_get_value(slider));
}

void set_vol_slider(int volume)
{
	lv_slider_set_value(vol_slider, volume, LV_ANIM_ON);
	
	char buf[20];
	lv_snprintf(buf, sizeof(buf), "volume %d", volume);
	lv_label_set_text(vol_slider_label, buf);
	lv_obj_align_to(vol_slider_label, vol_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	audio_output->set_volume(volume);
}

void select_mode(int s_mode);

static void mode_button_event(lv_event_t * e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t * btn = lv_event_get_target(e);
	int bmode = (int)lv_obj_get_user_data(btn);
		
	if (code == LV_EVENT_CLICKED) {
		LV_LOG_USER("Clicked");
		
		select_mode(bmode);
		
		switch (bmode)
		{
		case mode_broadband_fm:
		case mode_lsb:
		case mode_usb:
		case mode_am:
		case mode_dsb:
		case mode_cw:
		case mode_ft8:
			break;
		}
	
	}
}