#pragma once
#include "Settings.h"
#include "vfo.h"
#include <cstdio>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

extern const int screenWidth;
extern const int screenHeight;

struct MouseState
{
	lv_coord_t x;
	lv_coord_t y;
	bool pressed;
	bool MouseActivity;
	int Rotated;
};

class Mouse
{
  public:
	void init_mouse(string input_dev);
	Mouse();
	Mouse(int mousefd);

	int count();
	void step_vfo();
	MouseState GetMouseState();
	bool GetMouseAttached();

  private:
	int m_fd;
	int numMouseIndex;
	char mouseDev[80];
	bool bstep;
	input_event mouse_event;
	int step;

	bool read_mouse_event();
	string mouse_name;
	MouseState state{0,0, LV_INDEV_STATE_REL, false};
	bool MouseActivity;
};
