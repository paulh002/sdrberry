#pragma once
#include <string>
const inline int small_res = 800;
extern int screenWidth;
extern int screenHeight;
extern const int bottomHeight;
extern const int topHeight;
extern int tunerHeight;
extern const int rightWidth;
extern int barHeight;
extern int barHeightft8;
extern const int MorseHeight;
extern const int nobuttons;
extern const int bottombutton_width;
extern const int bottombutton_width1;
extern const int buttonHeight;
extern int tabHeight;
extern int screenfontthresshold_1;
extern int screenfontthresshold_2;
extern int screenfontthresshold_3;

extern lv_obj_t *get_main_screen();
extern lv_obj_t *get_2nd_screen();

extern lv_display_t *disp_0;
extern lv_display_t *disp_1;
extern lv_obj_t *main_screen;
extern lv_obj_t *second_screen;

lv_obj_t *create_display(lv_display_t **disp, const std::string device);
