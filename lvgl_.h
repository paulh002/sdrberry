#pragma once
#include "lvgl.h"

template <typename Class, void (Class::*MemberFunc)(lv_event_t *)>
struct EventHandler
{
	static void staticHandler(lv_event_t *e)
	{
		(static_cast<Class *>(e->user_data)->*MemberFunc)(e);
	}
};