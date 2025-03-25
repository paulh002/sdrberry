#include "guiListWindows.h"
#include "CustomEvents.h"


const int windowsliderbar = 40;

guiListWindows::guiListWindows(lv_obj_t *parent, void *thisptr, std::string name, std::vector<std::string> elements, int val, lv_event_code_t eventNumber, int width, int height)
	: Parent(parent), thisPtr(thisptr), event(eventNumber), row_selected(val)
{
	if (listWindowObj == nullptr)
	{
		listWindowObj = lv_win_create(lv_scr_act(), windowsliderbar);
		lv_win_add_title(listWindowObj, name.c_str());
		lv_obj_t *btn = lv_win_add_btn(listWindowObj, LV_SYMBOL_CLOSE, 60);
		lv_obj_add_event_cb(btn, btnWindowObj_event_handler, LV_EVENT_CLICKED, (void *)this);
		lv_obj_t *btnok = lv_win_add_btn(listWindowObj, LV_SYMBOL_OK, 60);
		lv_obj_add_event_cb(btnok, btnokWindowObj_event_handler, LV_EVENT_CLICKED, (void *)this);
		lv_obj_set_size(listWindowObj, width, height);
		lv_obj_align(listWindowObj, LV_ALIGN_CENTER, 0, 0);
		// create list
		lv_obj_set_style_pad_top(listWindowObj, 0, LV_PART_MAIN);
		lv_obj_set_style_pad_bottom(listWindowObj, 0, LV_PART_MAIN);
		
		lv_obj_t *winAreaObject = lv_win_get_content(listWindowObj);
		lv_obj_update_layout(listWindowObj);
		lv_coord_t w = lv_obj_get_content_width(winAreaObject);
		lv_coord_t h = lv_obj_get_content_height(winAreaObject);
		lv_obj_set_style_pad_top(winAreaObject, 0, LV_PART_MAIN);
		lv_obj_set_style_pad_bottom(winAreaObject, 0, LV_PART_MAIN);
		lv_obj_set_style_pad_left(winAreaObject, 0, LV_PART_MAIN);
		lv_obj_set_style_pad_right(winAreaObject, 0, LV_PART_MAIN);
		
		list = lv_table_create(winAreaObject);
		lv_obj_add_event_cb(list, draw_part_event, LV_EVENT_DRAW_PART_BEGIN, (void *)this);
		lv_obj_add_event_cb(list, list_handler, LV_EVENT_VALUE_CHANGED, (void *)this);
		lv_obj_set_style_pad_top(list, 0, LV_PART_MAIN);
		lv_obj_set_style_pad_bottom(list, 0, LV_PART_MAIN);
		lv_obj_set_style_pad_ver(list, 0, LV_PART_ITEMS);
		lv_obj_set_style_pad_left(list, 0, LV_PART_ITEMS);
		lv_obj_set_style_pad_right(list, 0, LV_PART_ITEMS);
		lv_obj_align(list, LV_ALIGN_TOP_LEFT, 0, 0);
		lv_table_set_col_width(list, 0, width -20);
		lv_obj_set_size(list, width, height);
		//lv_obj_set_pos(list, 0, 0);
		
		int i = 0;
		for (auto col : elements)
		{
			lv_table_set_cell_value(list, i++, 0, col.c_str());
		}
	}
}

guiListWindows::~guiListWindows()
{
}

void guiListWindows::btnWindowObj_event_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_CLICKED)
	{
		lv_obj_del(listWindowObj);
		listWindowObj = nullptr;
		lv_event_send(Parent, customLVevents.getCustomEvent(LV_EVENT_STEPS_CUSTOM), NULL);
	}
}
void guiListWindows::btnokWindowObj_event_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_CLICKED)
	{
		lv_obj_del(listWindowObj);
		listWindowObj = nullptr;
		lv_event_send(Parent, customLVevents.getCustomEvent(LV_EVENT_STEPS_CUSTOM_OK), NULL);
	}
}

void guiListWindows::list_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	lv_table_t *table = (lv_table_t *)obj;
	uint16_t row, col;
	if (code == LV_EVENT_VALUE_CHANGED)
	{
		lv_table_get_selected_cell(obj, &row, &col);
		lv_event_send(Parent, event, (void *)(long)row);
		row_selected = row;
	}
}

void guiListWindows::draw_part_event_class(lv_event_t *e)
{
	lv_obj_t *obj = lv_event_get_target(e);
	lv_table_t *table = (lv_table_t *)obj;
	lv_obj_draw_part_dsc_t *dsc = (lv_obj_draw_part_dsc_t *)lv_event_get_param(e);
	/*If the cells are drawn...*/
	if (dsc->part == LV_PART_ITEMS)
	{
		dsc->label_dsc->align = LV_TEXT_ALIGN_RIGHT;
		int row = dsc->id / lv_table_get_col_cnt(obj);
		if (row_selected == row)
		{
			dsc->rect_dsc->bg_color = lv_color_mix(lv_palette_main(LV_PALETTE_ORANGE), dsc->rect_dsc->bg_color, LV_OPA_30);
			dsc->rect_dsc->bg_opa = LV_OPA_COVER;
		}
	}
}