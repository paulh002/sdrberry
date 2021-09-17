#pragma once
#include <linux/input.h>
#include <cstdio>
#include <string.h>
#include "Settings.h"
#include "sdrberry.h"
#include "vfo.h"

class Mouse
{
public:
	void	init_mouse(string input_dev);
			Mouse();
	int		count();
	void	step_vfo();
	
	
private:
	int				m_fd;
	int				numMouseIndex;
	char			mouseDev[80];
	bool			bstep;
	input_event		mouse_event;
	int				step;
	
	bool			read_mouse_event();
	string			mouse_name;
};

