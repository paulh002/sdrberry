#include "CustomEvents.h"

bool CustomEvents::initialized = false;
std::vector<uint32_t> CustomEvents::custom_event;

CustomEvents::CustomEvents()
{
	if (!initialized)
	{
		initialized = true;
		for (int i = 0; i < (int)LV_EVENT_CUSTOM_LAST; i++)
		{
			custom_event.push_back(lv_event_register_id());
		}
	}
}
