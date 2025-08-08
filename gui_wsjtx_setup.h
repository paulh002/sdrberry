#pragma once
#include "lvgl_.h"

class gui_wsjtx_setup
{
  private:
	lv_style_t style_btn, settings_style, text_style;
	lv_obj_t *filter_cq, *text_label, *qra_textarea, *locator_textarea;
	lv_obj_t *kb{nullptr};
	void filter_cq_event_cb_class(lv_event_t *e);
	void qra_textarea_event_handler_class(lv_event_t *e);

  public:
	void init(lv_obj_t *o_tab, lv_group_t *keyboard_group, lv_coord_t w, lv_coord_t h, lv_group_t *bg);

	static constexpr auto qra_textarea_event_handler = EventHandler<gui_wsjtx_setup, &gui_wsjtx_setup::qra_textarea_event_handler_class>::staticHandler;
	static constexpr auto filter_cq_event_cb = EventHandler<gui_wsjtx_setup, &gui_wsjtx_setup::filter_cq_event_cb_class>::staticHandler;
};

extern gui_wsjtx_setup guiwsjtxsetup;