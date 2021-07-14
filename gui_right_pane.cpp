#include "lvgl/lvgl.h"
#include "gui_right_pane.h"

static lv_style_t style_btn;

lv_obj_t*	bUsb, *bLsb, *bAM, *bFM, *bCW, *bFT8, *bg_right; 

static const int nobuttons = 4;
static const int bottombutton_width = (rightWidth / nobuttons) - 2;
static const int bottombutton_width1 = (rightWidth / nobuttons);
static int button_height , button_margin = 18;

	
void	setup_right_pane(lv_obj_t* scr )
{
	static lv_style_t right_style;
		
	lv_style_init(&right_style);
	lv_style_set_radius(&right_style, 0);
	lv_style_set_bg_color(&right_style, lv_palette_main(LV_PALETTE_GREEN));
	
	bg_right = lv_obj_create(scr);
	lv_obj_add_style(bg_right, &right_style, 0);
	lv_obj_set_pos(bg_right, LV_HOR_RES - (rightWidth + 3), topHeight + tunerHeight);
	lv_obj_set_size(bg_right, rightWidth - 3, screenHeight - topHeight - tunerHeight);
	
	
	lv_style_init(&style_btn);

	lv_style_set_radius(&style_btn, 10);
	lv_style_set_bg_color(&style_btn, lv_color_make(0x60, 0x60, 0x60));
	lv_style_set_bg_grad_color(&style_btn, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_bg_grad_dir(&style_btn, LV_GRAD_DIR_VER);
	lv_style_set_bg_opa(&style_btn, 255);
	lv_style_set_border_color(&style_btn, lv_color_make(0x9b, 0x36, 0x36));   // lv_color_make(0x2e, 0x44, 0xb2)
	lv_style_set_border_width(&style_btn, 2);
	lv_style_set_border_opa(&style_btn, 255);
	lv_style_set_outline_color(&style_btn, lv_color_black());
	lv_style_set_outline_opa(&style_btn, 255);
	
	bUsb = lv_btn_create(bg_right);
	lv_obj_add_style(bUsb, &style_btn, 0); 
	//lv_obj_set_event_cb(vfo1_button, mode_button_vfo);
	lv_obj_align(bUsb, LV_ALIGN_TOP_LEFT, 0 * bottombutton_width1, button_margin);
	//lv_btn_set_checkable(vfo1_button, true);
	//lv_btn_toggle(vfo1_button);
	lv_obj_set_size(bUsb, bottombutton_width, bottomHeight);
	lv_obj_t*  label = lv_label_create(bUsb);
	lv_label_set_text(label, "Usb");
	lv_obj_center(label);
	
	bLsb = lv_btn_create(bg_right);
	lv_obj_add_style(bLsb, &style_btn, 0); 
	//lv_obj_set_event_cb(vfo1_button, mode_button_vfo);
	lv_obj_align(bLsb, LV_ALIGN_TOP_LEFT, 1 * bottombutton_width1, button_margin);
	//lv_btn_set_checkable(vfo1_button, true);
	//lv_btn_toggle(vfo1_button);
	lv_obj_set_size(bLsb, bottombutton_width, bottomHeight);
	label = lv_label_create(bLsb);
	lv_label_set_text(label, "Lsb");
	lv_obj_center(label);
	
	button_height = bottomHeight + button_margin + 4;  //lv_obj_get_height(bLsb);
	
	bCW = lv_btn_create(bg_right);
	lv_obj_add_style(bCW, &style_btn, 0); 
	//lv_obj_set_event_cb(vfo1_button, mode_button_vfo);
	lv_obj_align(bCW, LV_ALIGN_TOP_LEFT, 2 * bottombutton_width1, button_margin);
	//lv_btn_set_checkable(vfo1_button, true);
	//lv_btn_toggle(vfo1_button);
	lv_obj_set_size(bCW, bottombutton_width, bottomHeight);
	label = lv_label_create(bCW);
	lv_label_set_text(label, "CW");
	lv_obj_center(label);
	
	bFM = lv_btn_create(bg_right);
	lv_obj_add_style(bFM, &style_btn, 0); 
	//lv_obj_set_event_cb(vfo1_button, mode_button_vfo);
	lv_obj_align(bFM, LV_ALIGN_TOP_LEFT, 0 * bottombutton_width1, button_height);
	//lv_btn_set_checkable(vfo1_button, true);
	//lv_btn_toggle(vfo1_button);
	lv_obj_set_size(bFM, bottombutton_width, bottomHeight);
	label = lv_label_create(bFM);
	lv_label_set_text(label, "FM");
	lv_obj_center(label);
	
	bAM = lv_btn_create(bg_right);
	lv_obj_add_style(bAM, &style_btn, 0); 
	//lv_obj_set_event_cb(vfo1_button, mode_button_vfo);
	lv_obj_align(bAM, LV_ALIGN_TOP_LEFT, 1 * bottombutton_width1, button_height);
	//lv_btn_set_checkable(vfo1_button, true);
	//lv_btn_toggle(vfo1_button);
	lv_obj_set_size(bAM, bottombutton_width, bottomHeight);
	label = lv_label_create(bAM);
	lv_label_set_text(label, "AM");
	lv_obj_center(label);
	
	bFT8 = lv_btn_create(bg_right);
	lv_obj_add_style(bFT8, &style_btn, 0); 
	//lv_obj_set_event_cb(vfo1_button, mode_button_vfo);
	lv_obj_align(bFT8, LV_ALIGN_TOP_LEFT, 2 * bottombutton_width1, button_height);
	//lv_btn_set_checkable(vfo1_button, true);
	//lv_btn_toggle(vfo1_button);
	lv_obj_set_size(bFT8, bottombutton_width, bottomHeight);
	label = lv_label_create(bFT8);
	lv_label_set_text(label, "FT8");
	lv_obj_center(label);
	
	label = lv_label_create(bg_right);
	lv_label_set_text(label, "Mode");
	lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 0);
	lv_obj_clear_flag(label, LV_OBJ_FLAG_SCROLLABLE);
}

