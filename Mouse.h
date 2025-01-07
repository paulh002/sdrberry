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
#include "screen.h"

struct MouseState
{
	lv_coord_t x;
	lv_coord_t y;
	bool pressed;
	bool MouseActivity;
	bool doubleclick;
	int Rotated;
	int btn_id;
};

class Mouse
{
  public:
	void init_mouse();
	Mouse();
	Mouse(int mousefd);
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

	//std::string find_mouse();
	bool find_mouse(const char *device_path);
};
