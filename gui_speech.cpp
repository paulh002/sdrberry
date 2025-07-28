#include "lvgl_.h"
#include "gui_speech.h"
#include "Settings.h"
#include "sdrberry.h"
#include "screen.h"

gui_speech gspeech;

static void speech_button_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);

	if (code == LV_EVENT_CLICKED)
	{
		for (int i = 0; i < gspeech.getbuttons(); i++)
		{
			if ((obj != gspeech.get_button_obj(i)) && (lv_obj_has_flag(gspeech.get_button_obj(i), LV_OBJ_FLAG_CHECKABLE)))
			{
				lv_obj_clear_state(gspeech.get_button_obj(i), LV_STATE_CHECKED);
			}
			else
			{
				gspeech.set_speech_mode(i);
			}
		}
	}
}

static void save_button_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);

	if (code == LV_EVENT_CLICKED)
	{
		lv_obj_clear_state(obj, LV_STATE_CHECKED);

		Settings_file.save_speech("treble", gspeech.get_treble());
		Settings_file.save_speech("bass", gspeech.get_bass());
		Settings_file.save();
		printf("save\n");
	}

}

void gui_speech::set_speech_mode(int m)
{
	int atack, release;

	speech_mode = m;
	switch (m)
	{
	case 1:
		// fast
		Settings_file.getspeech_preset("fast", atack, release);
		set_atack_slider(atack);
		set_release_slider(release);
		break;
	case 2:
		// medium
		Settings_file.getspeech_preset("medium", atack, release);
		set_atack_slider(atack);
		set_release_slider(release);
		break;
	case 3:
		// slow
		Settings_file.getspeech_preset("slow", atack, release);
		set_atack_slider(atack);
		set_release_slider(release);
		break;		
	}
}

static void threshold_slider_event_cb(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *slider = lv_event_get_target(e);

	char str[80];
	sprintf(str, "threshold %d db", (lv_slider_get_value(slider) - 100) / 5);
	lv_label_set_text(gspeech.get_threshold_slider_label(), str);
	gspeech.set_threshold((lv_slider_get_value(slider) - 100) / 5);
}

static void atack_slider_event_cb(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *slider = lv_event_get_target(e);

	char str[80];
	sprintf(str, "atack %d ms", lv_slider_get_value(slider));
	lv_label_set_text(gspeech.get_atack_slider_label(), str);
	gspeech.set_atack(lv_slider_get_value(slider) / 1000.f);
}

static void release_slider_event_cb(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *slider = lv_event_get_target(e);

	char str[80];
	sprintf(str, "release %d ms", lv_slider_get_value(slider));
	lv_label_set_text(gspeech.get_release_slider_label(), str);
	gspeech.set_release(lv_slider_get_value(slider) / 1000.f);
}

static void ratio_slider_event_cb(lv_event_t *e)
{
	lv_obj_t *slider = lv_event_get_target(e);
	char buf[20];
	sprintf(buf, "ratio %d", lv_slider_get_value(slider));
	lv_label_set_text(gspeech.get_ratio_slider_label(), buf);
	gspeech.set_ratio(lv_slider_get_value(slider));
}

static void bass_slider_event_cb(lv_event_t *e)
{
	lv_obj_t *slider = lv_event_get_target(e);
	char buf[20];
	sprintf(buf, "bass %d", lv_slider_get_value(slider));
	lv_label_set_text(gspeech.get_bass_slider_label(), buf);
	gspeech.set_bass(lv_slider_get_value(slider));
}

static void treble_slider_event_cb(lv_event_t *e)
{
	lv_obj_t *slider = lv_event_get_target(e);
	char buf[20];
	sprintf(buf, "treble %d", lv_slider_get_value(slider));
	lv_label_set_text(gspeech.get_treble_slider_label(), buf);
	gspeech.set_treble(lv_slider_get_value(slider));
}

void gui_speech::init(lv_obj_t *o_tab, lv_coord_t w)
{

	const lv_coord_t x_margin = 10;
	const lv_coord_t y_margin = 15;
	const int x_number_buttons = 10;
	const int y_number_buttons = 4;
	const lv_coord_t tab_margin = 20;
	const int y1_margin = 7;

	int button_width_margin = ((w - tab_margin) / x_number_buttons);
	int button_width = ((w - tab_margin) / x_number_buttons) - x_margin;
	int button_height = 30;
	int button_height_margin = button_height + y_margin;
	int ibutton_x = 0, ibutton_y = 0;
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
		char str[80];

		button[i] = lv_btn_create(o_tab);
		lv_group_add_obj(m_button_group, button[i]);
		lv_obj_add_style(button[i], &style_btn, 0);
		lv_obj_add_event_cb(button[i], speech_button_handler, LV_EVENT_CLICKED, NULL);
		lv_obj_align(button[i], LV_ALIGN_TOP_LEFT, ibutton_x * button_width_margin, ibutton_y * button_height_margin);
		lv_obj_set_size(button[i], button_width, button_height);
		lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);

		lv_obj_t *lv_label = lv_label_create(button[i]);
		switch (i)
		{
		case 0:
			strcpy(str, "Off");
			lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
			//lv_obj_set_user_data(button[i], (void *)mode_usb);
			break;
		case 1:
			strcpy(str, "Fast");
			lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
			//lv_obj_set_user_data(button[i], (void *)mode_lsb);
			break;
		case 2:
			strcpy(str, "Med");
			lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
			//lv_obj_set_user_data(button[i], (void *)mode_cw);
			break;
		case 3:
			strcpy(str, "Slow");
			lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
			//lv_obj_set_user_data(button[i], (void *)mode_narrowband_fm);
			break;
		case 4:
			strcpy(str, "Save");
			lv_obj_remove_event_cb(button[i], speech_button_handler);
			lv_obj_add_event_cb(button[i], save_button_handler, LV_EVENT_CLICKED, NULL);
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
	
	// save button not part of row
	ibuttons--;
	
	speech_mode = Settings_file.getspeech("mode");
	if (speech_mode > 3 || speech_mode < 0)
		speech_mode = 0;
	lv_obj_add_state(get_button_obj(speech_mode), LV_STATE_CHECKED);
	ibutton_y++;
	ratio_slider = lv_slider_create(o_tab);
	lv_obj_set_width(ratio_slider, w / 2 - 50);
	lv_slider_set_range(ratio_slider, 1, 100);
	lv_obj_align_to(ratio_slider, o_tab, LV_ALIGN_TOP_LEFT, 0, ibutton_y * button_height_margin + 10);
	lv_obj_add_event_cb(ratio_slider, ratio_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	lv_group_add_obj(m_button_group, ratio_slider);

	ratio_slider_label = lv_label_create(o_tab);
	lv_label_set_text(ratio_slider_label, "ratio 10");
	lv_obj_align_to(ratio_slider_label, ratio_slider, LV_ALIGN_TOP_MID, 0, -20);
	set_ratio_slider(Settings_file.getspeech("ratio"));

	//lv_obj_align_to(threshold_slider_label, o_tab, LV_ALIGN_CENTER, 0, -40);
	threshold_slider_label = lv_label_create(o_tab);
	lv_label_set_text(threshold_slider_label, "threshold -10 db");

	threshold_slider = lv_slider_create(o_tab);
	lv_obj_set_width(threshold_slider, w / 2 - 50);
	lv_slider_set_range(threshold_slider, 0, 200);
	lv_obj_align_to(threshold_slider, o_tab, LV_ALIGN_TOP_LEFT, w / 2, ibutton_y * button_height_margin + 10);
	lv_obj_add_event_cb(threshold_slider, threshold_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	set_threshold_slider(Settings_file.getspeech("threshold"));
	lv_group_add_obj(m_button_group, threshold_slider);
	lv_obj_align_to(threshold_slider_label, threshold_slider, LV_ALIGN_TOP_MID, 0, -20);

	int atack, release;
	Settings_file.getspeech_preset("fast", atack, release);
	
	atack_slider_label = lv_label_create(o_tab);
	lv_label_set_text(atack_slider_label, "atack ");
	lv_obj_align_to(atack_slider_label, ratio_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, y1_margin);
	atack_slider = lv_slider_create(o_tab);
	lv_slider_set_range(atack_slider, 0, 500);
	lv_obj_set_width(atack_slider, w / 2 - 50);
	lv_obj_align_to(atack_slider, atack_slider_label, LV_ALIGN_OUT_BOTTOM_MID, 0, y1_margin);
	lv_obj_add_event_cb(atack_slider, atack_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	set_atack_slider(atack);
	lv_group_add_obj(m_button_group, atack_slider);

	release_slider_label = lv_label_create(o_tab);
	lv_label_set_text(release_slider_label, "release ");
	lv_obj_align_to(release_slider_label, threshold_slider, LV_ALIGN_OUT_BOTTOM_MID, -25, y1_margin);
	release_slider = lv_slider_create(o_tab);
	lv_group_add_obj(m_button_group, release_slider);
	set_release_slider(release);
	lv_slider_set_range(release_slider, 0, 1000);
	lv_obj_set_width(release_slider, w / 2 - 50);
	lv_obj_align_to(release_slider, release_slider_label, LV_ALIGN_OUT_BOTTOM_MID, 0, y1_margin);
	lv_obj_add_event_cb(release_slider, release_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	
	bass_slider_label = lv_label_create(o_tab);
	lv_label_set_text(bass_slider_label, "bass");
	lv_obj_align_to(bass_slider_label, atack_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, y1_margin);
	bass_slider = lv_slider_create(o_tab);
	lv_slider_set_range(bass_slider, -100, 100);
	lv_obj_set_width(bass_slider, w / 2 - 50);
	lv_obj_align_to(bass_slider, bass_slider_label, LV_ALIGN_OUT_BOTTOM_MID, 0, y1_margin);
	lv_obj_add_event_cb(bass_slider, bass_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	set_bass_slider(Settings_file.getspeech("bass"));
	lv_group_add_obj(m_button_group, bass_slider);

	treble_slider_label = lv_label_create(o_tab);
	lv_label_set_text(treble_slider_label, "treble ");
	lv_obj_align_to(treble_slider_label, release_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, y1_margin);
	treble_slider = lv_slider_create(o_tab);
	lv_group_add_obj(m_button_group, treble_slider);
	set_treble_slider(Settings_file.getspeech("treble"));
	lv_slider_set_range(treble_slider, -100, 100);
	lv_obj_set_width(treble_slider, w / 2 - 50);
	lv_obj_align_to(treble_slider, treble_slider_label, LV_ALIGN_OUT_BOTTOM_MID, 0, y1_margin);
	lv_obj_add_event_cb(treble_slider, treble_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	
	lv_group_add_obj(m_button_group, lv_tabview_get_tab_btns(tabview_mid));
}

void gui_speech::set_group()
{
	lv_indev_set_group(encoder_indev_t, m_button_group);
	lv_group_focus_obj(button[0]);
}

void gui_speech::set_atack_slider(int t)
{
	char str[80];

	lv_slider_set_value(atack_slider, t, LV_ANIM_ON);
	sprintf(str, "atack %d ms", t);
	lv_label_set_text(atack_slider_label, str);
	set_atack(lv_slider_get_value(atack_slider) / 1000.f);
}

void gui_speech::set_bass_slider(int t)
{
	char str[80];

	lv_slider_set_value(bass_slider, t, LV_ANIM_ON);
	sprintf(str, "bass %d", t);
	lv_label_set_text(bass_slider_label, str);
	set_bass(lv_slider_get_value(bass_slider));
}

void gui_speech::set_treble_slider(int t)
{
	char str[80];

	lv_slider_set_value(treble_slider, t, LV_ANIM_ON);
	sprintf(str, "treble %d", t);
	lv_label_set_text(treble_slider_label, str);
	set_treble(lv_slider_get_value(treble_slider));
}

void gui_speech::set_threshold_slider(int t)
{
	char str[80];
	int t1 = (t * 5) + 100;
	lv_slider_set_value(threshold_slider, abs(t1), LV_ANIM_ON);
	sprintf(str, "threshold -%d db", t);
	lv_label_set_text(threshold_slider_label, str);
	set_threshold((lv_slider_get_value(threshold_slider) - 100) / 5);
}

void gui_speech::set_release_slider(int t)
{
	char str[80];

	lv_slider_set_value(release_slider, t, LV_ANIM_ON);
	sprintf(str, "release %d ms", t);
	lv_label_set_text(release_slider_label, str);
	set_release(lv_slider_get_value(release_slider) / 1000.f);
}

void gui_speech::set_ratio_range(int min, int max)
{
	lv_slider_set_range(ratio_slider, min, max);
}

void gui_speech::set_ratio_slider(int gain)
{
	char buf[80];

	sprintf(buf, "ratio %d", gain);
	lv_label_set_text(ratio_slider_label, buf);
	lv_slider_set_value(ratio_slider, gain, LV_ANIM_ON);
	set_ratio(lv_slider_get_value(ratio_slider));
}

/*
int gui_speech::get_threshold()
{
	return (lv_slider_get_value(threshold_slider) - 100) / 5; // - max_threshold;
}

float gui_speech::get_atack()
{
	return (float)lv_slider_get_value(atack_slider) / 1000.f;
}

float gui_speech::get_release()
{
	return (float)lv_slider_get_value(release_slider) / 1000.f;
}

float gui_speech::get_ratio()
{
	return (float)lv_slider_get_value(ratio_slider);
}

int gui_speech::get_treble()
{
	return lv_slider_get_value(treble_slider);
}

int gui_speech::get_bass()
{
	return lv_slider_get_value(bass_slider);
}
*/