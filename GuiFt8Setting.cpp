#include <stdio.h>
#include "GuiFt8Setting.h"

GuiFt8Setting guift8setting;

static void textarea_event_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *ta = lv_event_get_target(e);

	if (code == LV_EVENT_CLICKED || code == LV_EVENT_FOCUSED || code == LV_EVENT_VALUE_CHANGED)
	{
		/*Focus on the clicked text area*/
		//printf("text event\n");
	}
	//LV_UNUSED(ta);
	//LV_LOG_USER("Enter was pressed. The current text is: %s", lv_textarea_get_text(ta));
}

void GuiFt8Setting::init(lv_obj_t *o_tab, lv_group_t *keyboard_group)
{
	Textfield = lv_textarea_create(o_tab);
	lv_textarea_set_one_line(Textfield, true);
	lv_obj_align(Textfield, LV_ALIGN_TOP_MID, 0, 10);
	lv_obj_add_event_cb(Textfield, textarea_event_handler, LV_EVENT_ALL, Textfield);
	lv_obj_add_state(Textfield, LV_STATE_FOCUSED); /*To be sure the cursor is visible*/
	lv_group_add_obj(keyboard_group, Textfield);
}


