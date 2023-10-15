#include "HexKeyboardWindow.h"
#include <memory>

static std::shared_ptr<HexKeyboardWindow> HexKeyboardPtr;

static const char *btnm_map[] = {"1", "2", "3", "\n",
								 "4", "5", "6", "\n",
								 "7", "8", "9", "\n",
								 "A", "B", "C", "\n", 
								 "D", "E", "F", "\n",
								 LV_SYMBOL_BACKSPACE, "0", LV_SYMBOL_NEW_LINE, ""};

void CreateHexKeyboardWindow(lv_obj_t *parent)
{
	if (HexKeyboardPtr != nullptr)
		return;
	HexKeyboardPtr = std::make_shared<HexKeyboardWindow>(parent);
}

static void btn_event_handler(lv_event_t *e)
{
	lv_obj_t *obj = lv_event_get_target(e);
	lv_obj_del(HexKeyboardPtr->getWin());
	HexKeyboardPtr.reset();
}

static void btnm_event_handler(lv_event_t *e)
{
	lv_obj_t *obj = lv_event_get_target(e);
	lv_obj_t *ta = (lv_obj_t *)lv_event_get_user_data(e);
	const char *txt = lv_btnmatrix_get_btn_text(obj, lv_btnmatrix_get_selected_btn(obj));

	if (strcmp(txt, LV_SYMBOL_BACKSPACE) == 0)
	{
		lv_textarea_del_char(ta);
		const char *ptr = lv_textarea_get_text(ta);
		if (strlen(ptr) == 3)
		{
			lv_textarea_del_char(ta);
		}
	}
	else if (strcmp(txt, LV_SYMBOL_NEW_LINE) == 0)
	{
		lv_event_send(ta, LV_EVENT_READY, NULL);
		const char *ptr = lv_textarea_get_text(ta);
		lv_msg_send(MSG_TEXTMESSAGE, (const void *)ptr);
		lv_textarea_set_text(ta, "");
	}
	else
	{
		const char *ptr = lv_textarea_get_text(ta);
		if (strlen(ptr) == 2)
		{
			lv_textarea_add_text(ta, ",");
		}
		lv_textarea_add_text(ta, txt);
		ptr = lv_textarea_get_text(ta);
		if (strlen(ptr) == 2)
		{
			lv_textarea_add_text(ta, ",");
		}
	}
}

HexKeyboardWindow::HexKeyboardWindow(lv_obj_t *parent)
{
	win = lv_win_create(parent, 30);
	lv_obj_set_size(win, 250, 400);
	
	closeBtn = lv_win_add_btn(win, LV_SYMBOL_CLOSE, 60);
	lv_obj_add_event_cb(closeBtn, btn_event_handler, LV_EVENT_CLICKED, NULL);

	lv_obj_t *cont = lv_win_get_content(win);
	lv_obj_t *ta = lv_textarea_create(cont);
	lv_textarea_set_max_length(ta,5);
	lv_textarea_set_one_line(ta, true);
	lv_obj_set_width(ta, 200);
	lv_obj_align(ta, LV_ALIGN_TOP_MID, 0, 0);
	lv_obj_add_state(ta, LV_STATE_FOCUSED); /*To be sure the cursor is visible*/

	lv_obj_t *btnm = lv_btnmatrix_create(cont);
	lv_obj_set_size(btnm, 200, 250);
	lv_obj_align_to(btnm, ta, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
	lv_obj_add_event_cb(btnm, btnm_event_handler, LV_EVENT_VALUE_CHANGED, ta);
	lv_obj_clear_flag(btnm, LV_OBJ_FLAG_CLICK_FOCUSABLE); /*To keep the text area focused on button clicks*/
	lv_btnmatrix_set_map(btnm, btnm_map);
}

HexKeyboardWindow::~HexKeyboardWindow()
{
	
}
