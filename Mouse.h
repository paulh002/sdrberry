#pragma once
#include "Settings.h"
#include "vfo.h"
#include <chrono>
#include <cstdio>
#include <fcntl.h>
#include <linux/input.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

extern const int screenWidth;
extern const int screenHeight;

struct MouseState
{
	lv_coord_t x;
	lv_coord_t y;
	bool pressed;
	bool MouseActivity;
	bool doubleclick;
	int Rotated;
};

class Mouse
{
  public:
	void init_mouse();
	Mouse();
	Mouse(int mousefd);

	int count();
	void step_vfo();
	MouseState GetMouseState();
	bool GetMouseAttached();
	
  private:
	int fd;
	char mouseDev[80];
	bool bstep;
	input_event mouse_event;
	int step;

	bool read_mouse_event();
	std::string mouse_name;
	MouseState state{0, 0, LV_INDEV_STATE_REL, false};
	bool MouseActivity;
	std::chrono::time_point<std::chrono::system_clock> last_click_time;
	int click_count;

	std::string find_mouse();
};
