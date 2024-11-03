#pragma once
#include "Settings.h"
#include <cstdio>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "Mouse.h"
#include "KeyboardTranslator.h"
#include <queue>

#define KEYBOARD_BUFFER_SIZE 256

class Keyboard
{
  public:
	void init_keyboard();
	Keyboard();
	~Keyboard();
	MouseState GetMouseState();
	bool MouseAttached() { return mouse_attached; }
	bool Attached();
	std::string GetKeys();

  private:
	int fd;
	int Index;
	char KeyboardName[80];
	bool mouse_attached;
	int click_count;
	input_event keyboard_event;
	KeyboardTranslator kbTranslator;
	MouseState state{0, 0, LV_INDEV_STATE_REL, false};
	std::chrono::time_point<std::chrono::system_clock> last_click_time;
	std::queue<MouseState> mousestates;
	struct libevdev *dev; 
};

