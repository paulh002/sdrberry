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

#define KEYBOARD_BUFFER_SIZE 256

class Keyboard
{
  public:
	void init_keyboard();
	Keyboard();
	~Keyboard();
	MouseState GetMouseState();
	bool Attached();
	std::string GetKeys();

  private:
	int fd;
	int Index;
	char KeyboardName[80];
	input_event keyboard_event;
	unique_ptr<Mouse> mouse;
	KeyboardTranslator kbTranslator;
	struct libevdev *dev; 
};

