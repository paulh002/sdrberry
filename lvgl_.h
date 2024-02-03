#pragma once
#include "lvgl.h"
#include <cassert>

template <typename Class, void (Class::*MemberFunc)(lv_event_t *)>
struct EventHandler
{
	static void staticHandler(lv_event_t *e)
	{
		(static_cast<Class *>(e->user_data)->*MemberFunc)(e);
	}
};

template <typename Class, void (Class::*MemberFuncMsg)(void *, lv_msg_t *m)>
struct EventHandlerMsg
{
	static void staticHandler(void *e, lv_msg_t *m)
	{
		(static_cast<Class *>(m->user_data)->*MemberFuncMsg)(e, m);
	}
};