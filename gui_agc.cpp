#include "gui_agc.h"

Gui_agc gagc;

static void agc_button_handler(lv_event_t * e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e); 
	
	if (code == LV_EVENT_CLICKED) {
	
		for (int i = 0; i < gagc.getbuttons(); i++)
		{
			if ((obj != gagc.get_button_obj(i)) && (lv_obj_has_flag(gagc.get_button_obj(i), LV_OBJ_FLAG_CHECKABLE)))
			{
				lv_obj_clear_state(gagc.get_button_obj(i), LV_STATE_CHECKED);
			}
			else
				gagc.set_agc_mode(i);
		}
	}
}

void Gui_agc::set_agc_mode(int m)
{
	int atack, release;
	
	agc_mode = m;
	switch (m)
	{
	case 1:
		// fast
		Settings_file.getagc_preset("fast", atack, release);
		set_atack_slider(atack);
		set_release_slider(release);
		break;
	case 2:
		// medium
		Settings_file.getagc_preset("medium", atack, release);
		set_atack_slider(atack);
		set_release_slider(release);
		break;
	case 3:
		// slow
		Settings_file.getagc_preset("slow", atack, release);
		set_atack_slider(atack);
		set_release_slider(release);
		break;
	}
}

static void threshold_slider_event_cb(lv_event_t * e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *slider = lv_event_get_target(e); 
	
	char	str[80];
	sprintf(str, "threshold %d db", (lv_slider_get_value(slider) - 100)/5);
	lv_label_set_text(gagc.get_threshold_slider_label(), str);
}

static void atack_slider_event_cb(lv_event_t * e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *slider = lv_event_get_target(e); 
	
	char	str[80];
	sprintf(str, "atack %d ms", lv_slider_get_value(slider));
	lv_label_set_text(gagc.get_atack_slider_label(), str);
}

static void release_slider_event_cb(lv_event_t * e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *slider = lv_event_get_target(e); 
	
	char	str[80];
	sprintf(str, "release %d ms", lv_slider_get_value(slider));
	lv_label_set_text(gagc.get_release_slider_label(), str);
}
	
static void ratio_slider_event_cb(lv_event_t * e)
{
	lv_obj_t * slider = lv_event_get_target(e);
	char buf[20];
	sprintf(buf, "ratio %d", lv_slider_get_value(slider));
	lv_label_set_text(gagc.get_ratio_slider_label(), buf);
}

void Gui_agc::init(lv_obj_t* o_tab, lv_coord_t w)
{

	const lv_coord_t x_margin  = 10;
	const lv_coord_t y_margin  = 20;
	const int x_number_buttons = 5;
	const int y_number_buttons = 4;
	const lv_coord_t tab_margin  = 20;
	
	int button_width_margin = ((w - tab_margin) / x_number_buttons);
	int button_width = ((w - tab_margin) / x_number_buttons) - x_margin;
	int button_height = 40;
	int button_height_margin = button_height + y_margin;
	int	ibutton_x = 0, ibutton_y = 0;
	int i = 0;
	
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
	m_button_group = lv_group_create();
	ibuttons = number_of_buttons;
	for (i = 0; i < ibuttons; i++)
	{
		char	str[80];
		
		button[i] = lv_btn_create(o_tab);
		lv_group_add_obj(m_button_group, button[i]);
		lv_obj_add_style(button[i], &style_btn, 0); 
		lv_obj_add_event_cb(button[i], agc_button_handler, LV_EVENT_CLICKED, NULL);
		lv_obj_align(button[i], LV_ALIGN_TOP_LEFT, ibutton_x * button_width_margin, ibutton_y * button_height_margin);
		lv_obj_set_size(button[i], button_width, button_height);
		lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);		
			
		lv_obj_t* lv_label = lv_label_create(button[i]);
		switch (i)
		{
		case 0:
			strcpy(str, "Off");
			lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
			lv_obj_set_user_data(button[i], (void *)mode_usb);
			break;
		case 1:
			strcpy(str, "Fast");
			lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
			lv_obj_set_user_data(button[i], (void *)mode_lsb);
			break;
		case 2:
			strcpy(str, "Med");
			lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
			lv_obj_set_user_data(button[i], (void *)mode_cw);
			break;
		case 3:
			strcpy(str, "Slow");
			lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
			lv_obj_set_user_data(button[i], (void *)mode_narrowband_fm);
			break;
		}
		lv_label_set_text(lv_label, str);
		lv_obj_center(lv_label);
		
		ibutton_x++;
		if (ibutton_x >= x_number_buttons)
		{
			ibutton_x = 0;
			ibutton_y++;
		}
	}
	agc_mode = Settings_file.getagc("mode");
	if (agc_mode > 3 || agc_mode < 0)
		agc_mode = 0;
	lv_obj_add_state(get_button_obj(agc_mode), LV_STATE_CHECKED);
	ibutton_y++;
	ratio_slider = lv_slider_create(o_tab);
	lv_obj_set_width(ratio_slider, w / 2 - 50);
	lv_slider_set_range(ratio_slider, 1, 100);
	lv_obj_align_to(ratio_slider, o_tab, LV_ALIGN_TOP_LEFT, 0, ibutton_y * button_height_margin + 10);
	lv_obj_add_event_cb(ratio_slider, ratio_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	lv_group_add_obj(m_button_group, ratio_slider);

	ratio_slider_label = lv_label_create(o_tab);
	lv_label_set_text(ratio_slider_label, "ratio 10");
	lv_obj_align_to(ratio_slider_label, ratio_slider, LV_ALIGN_TOP_MID, 0,-20);
	set_ratio_slider(Settings_file.getagc("ratio"));

	//lv_obj_align_to(threshold_slider_label, o_tab, LV_ALIGN_CENTER, 0, -40);
	threshold_slider_label = lv_label_create(o_tab);
	lv_label_set_text(threshold_slider_label, "threshold -10 db");
	
	threshold_slider = lv_slider_create(o_tab);
	lv_obj_set_width(threshold_slider, w / 2 - 50); 
	lv_slider_set_range(threshold_slider, 0, 200);
	lv_obj_align_to(threshold_slider, o_tab, LV_ALIGN_TOP_LEFT, w / 2, ibutton_y * button_height_margin + 10);
	lv_obj_add_event_cb(threshold_slider, threshold_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	set_threshold_slider(Settings_file.getagc("threshold"));
	lv_group_add_obj(m_button_group, threshold_slider);
	lv_obj_align_to(threshold_slider_label, threshold_slider, LV_ALIGN_TOP_MID, 0, -20);

	int atack, release;
	Settings_file.getagc_preset("fast", atack, release);
	
	atack_slider_label = lv_label_create(o_tab);
	lv_label_set_text(atack_slider_label, "atack ");
	lv_obj_align_to(atack_slider_label, ratio_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	atack_slider = lv_slider_create(o_tab);
	lv_slider_set_range(atack_slider, 0, 100);
	lv_obj_set_width(atack_slider, w / 2 - 50); 
	lv_obj_align_to(atack_slider, atack_slider_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	lv_obj_add_event_cb(atack_slider, atack_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	set_atack_slider(atack);
	lv_group_add_obj(m_button_group, atack_slider);

	release_slider_label = lv_label_create(o_tab);
	lv_label_set_text(release_slider_label, "release ");
	lv_obj_align_to(release_slider_label, threshold_slider, LV_ALIGN_OUT_BOTTOM_MID, -25, 10);
	release_slider = lv_slider_create(o_tab);
	lv_group_add_obj(m_button_group, release_slider);
	set_release_slider(release);
	lv_slider_set_range(release_slider, 0, 1000);
	
	lv_obj_set_width(release_slider, w / 2 - 50); 
	lv_obj_align_to(release_slider, release_slider_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	lv_obj_add_event_cb(release_slider, release_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	set_release_slider(Settings_file.getagc("release"));
	lv_group_add_obj(m_button_group, lv_tabview_get_tab_btns(tabview_mid));
}

void Gui_agc::set_group()
{
	lv_indev_set_group(encoder_indev_t, m_button_group);
	lv_group_focus_obj(button[0]);
}

void Gui_agc::set_atack_slider(int t)
{
	char	str[80];
	
	lv_slider_set_value(atack_slider, t, LV_ANIM_ON);
	sprintf(str, "atack %d ms", t);
	lv_label_set_text(atack_slider_label, str);		
}

void Gui_agc::set_threshold_slider(int t)
{
	char	str[80];
	int t1 = (t*5) + 100;
	lv_slider_set_value(threshold_slider, abs(t1), LV_ANIM_ON);
	sprintf(str, "threshold -%d db", t);
	lv_label_set_text(threshold_slider_label, str);	
}

void Gui_agc::set_release_slider(int t)
{
	char	str[80];
	
	lv_slider_set_value(release_slider, t, LV_ANIM_ON);
	sprintf(str, "release %d ms", t);
	lv_label_set_text(release_slider_label, str);	
}

void Gui_agc::set_ratio_range(int min, int max)
{
	lv_slider_set_range(ratio_slider, min, max);
}

void Gui_agc::set_ratio_slider(int gain)
{
	char buf[80];
	
	sprintf(buf, "ratio %d", gain);
	lv_label_set_text(ratio_slider_label, buf);		
	lv_slider_set_value(ratio_slider, gain, LV_ANIM_ON); 
}

