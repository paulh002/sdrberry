#include "CustomEvents.h"

CustomEvents::CustomEvents()
{
	for (int i = 0; i < (int)LV_EVENT_CUSTOM_LAST; i++)
	{
		custom_event.push_back(lv_event_register_id());
	}
}

CustomEvents customLVevents;