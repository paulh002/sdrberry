#include "VfoKeyPad.h"
#include "SharedQueue.h"
#include <memory>
#include <regex>

static std::shared_ptr<VfoKeyPad> VfoKeyPadPtr;

static const char *btnm_map[] = {"1", "2", "3", "\n",
								 "4", "5", "6", "\n",
								 "7", "8", "9", "\n",
								 "Khz", "0", "Mhz", "\n",
								 LV_SYMBOL_BACKSPACE, ".", LV_SYMBOL_NEW_LINE, ""};

void CreateVfoKeyPadWindow(lv_obj_t *parent ,lv_group_t *keyboard_group)
{
	if (VfoKeyPadPtr != nullptr)
		return;
	VfoKeyPadPtr = std::make_shared<VfoKeyPad>(parent, keyboard_group);
}

static void btn_event_handler(lv_event_t *e)
{
	lv_obj_t *obj = lv_event_get_target(e);
	lv_obj_del(VfoKeyPadPtr->getWin());
	VfoKeyPadPtr.reset();
}

void VfoKeyPad::textarea_event_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *ta = lv_event_get_target(e);
	std::regex pattern("^[0-9MmKkhz. ]+$");

	if (code == LV_EVENT_KEY )
	{
		uint32_t c = *((uint32_t *)lv_event_get_param(e));
		if (c == 13)
		{
			lv_event_send(ta, LV_EVENT_READY, NULL);
			const char *ptr = lv_textarea_get_text(ta);
			guiQueue.push_back(GuiMessage(GuiMessage::action::setvfo, ptr));
			lv_textarea_set_text(ta, "");
		}
		if (c == 27)
		{
			lv_obj_del(VfoKeyPadPtr->getWin());
			VfoKeyPadPtr.reset();
		}
	}
	
	if (code == LV_EVENT_CLICKED || code == LV_EVENT_FOCUSED || code == LV_EVENT_VALUE_CHANGED)
	{
		const char *txt = lv_textarea_get_text(ta);
		if (!std::regex_match(txt, pattern))
		{
			lv_textarea_del_char(ta);
		}
	}
}

static void btnm_event_handler(lv_event_t *e)
{
	std::regex pattern("^[0-9MK.]+$");
	lv_obj_t *obj = lv_event_get_target(e);
	lv_obj_t *ta = (lv_obj_t *)lv_event_get_user_data(e);
	const char *txt = lv_btnmatrix_get_btn_text(obj, lv_btnmatrix_get_selected_btn(obj));

	if (strcmp(txt, LV_SYMBOL_BACKSPACE) == 0)
	{
		lv_textarea_del_char(ta);
		const char *ptr = lv_textarea_get_text(ta);
		if (strlen(ptr) == 8)
		{
			lv_textarea_del_char(ta);
		}
	}
	else if (strcmp(txt, LV_SYMBOL_NEW_LINE) == 0)
	{
		lv_event_send(ta, LV_EVENT_READY, NULL);
		const char *ptr = lv_textarea_get_text(ta);
		//lv_msg_send(MSG_TEXTMESSAGE, (const void *)ptr);
		guiQueue.push_back(GuiMessage(GuiMessage::action::setvfo, ptr));
		lv_textarea_set_text(ta, "");
	}
	else if (strcmp(txt, "Mhz") == 0 || strcmp(txt, "Khz") == 0)
	{
		lv_textarea_add_text(ta, " ");
		lv_textarea_add_text(ta, txt);
	}
	else
	{
		if (std::regex_match(txt, pattern))
		{
			const char *ptr = lv_textarea_get_text(ta);
			lv_textarea_add_text(ta, txt);
			ptr = lv_textarea_get_text(ta);
		}
	}
}

VfoKeyPad::VfoKeyPad(lv_obj_t *parent, lv_group_t * keyboard_group)
{
	win = lv_win_create(parent, 30);
	lv_obj_set_size(win, 250, 400);
	lv_obj_center(win);

	closeBtn = lv_win_add_btn(win, LV_SYMBOL_CLOSE, 60);
	lv_obj_add_event_cb(closeBtn, btn_event_handler, LV_EVENT_CLICKED, NULL);

	lv_obj_t *cont = lv_win_get_content(win);
	lv_obj_t *ta = lv_textarea_create(cont);
	lv_textarea_set_max_length(ta, 20);
	lv_textarea_set_one_line(ta, true);
	lv_obj_set_width(ta, 200);
	lv_obj_align(ta, LV_ALIGN_TOP_MID, 0, 0);
	lv_obj_add_state(ta, LV_STATE_FOCUSED); /*To be sure the cursor is visible*/
	if (keyboard_group != nullptr)
		lv_group_add_obj(keyboard_group, ta);
	lv_obj_add_event_cb(ta, textarea_event_handler, LV_EVENT_ALL, (void *) this);

	lv_obj_t *btnm = lv_btnmatrix_create(cont);
	lv_obj_set_size(btnm, 200, 250);
	lv_obj_align_to(btnm, ta, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
	lv_obj_add_event_cb(btnm, btnm_event_handler, LV_EVENT_VALUE_CHANGED, ta);
	lv_obj_clear_flag(btnm, LV_OBJ_FLAG_CLICK_FOCUSABLE); /*To keep the text area focused on button clicks*/
	lv_btnmatrix_set_map(btnm, btnm_map);
	lv_obj_add_state(ta, LV_STATE_FOCUSED); /*To be sure the cursor is visible*/
	lv_group_focus_obj(ta);
}

VfoKeyPad::~VfoKeyPad()
{
}
