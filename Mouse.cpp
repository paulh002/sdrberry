#include "Mouse.h"
#include <iostream>
#include <fstream>

#define test_bit(bit, array) (array[bit / 8] & (1 << (bit % 8)))
#define NBITS(x) ((((x)-1) / (sizeof(long) * 8)) + 1)

std::chrono::milliseconds double_click_threshold(300); 

Mouse::Mouse()
{
	fd = -1;
	step = 1;
	bstep = false;
	MouseActivity = false;
	last_click_time = std::chrono::system_clock::now();
	click_count = 0;
}

Mouse::Mouse(int mousefd)
{
	fd = mousefd;
	step = 1;
	bstep = false;
	MouseActivity = false;
}

void strupr(char *str)
{
	for (int i = 0; i < strlen(str); i++)
	{
		str[i] = toupper(str[i]);
	}
}

std::string Mouse::find_mouse()
{
	std::ifstream device("/proc/bus/input/devices");
	std::string line;
	std::string mouse;

	if (device.is_open())
	{
		while (std::getline(device, line))
		{
			transform(line.begin(), line.end(), line.begin(), ::toupper);
			// Look for lines containing "mouse" indicating a mouse device
			if (line.find("MOUSE") != std::string::npos && line.find("NAME") != std::string::npos)
			{
				while (std::getline(device, line))
				{
					if (line.find("H:") != std::string::npos)
					{
						int i = line.find("event");
						mouse = line.substr(i);
						break;
					}
				}
				break;
			}
		}
		device.close();
	}
	mouse.erase(std::remove(mouse.begin(), mouse.end(), ' '), mouse.end());
	return mouse;
}

void Mouse::init_mouse()
{
	std::string mouse = find_mouse();
	if (mouse.size() > 0)
	{
		strcpy(mouseDev,  "/dev/input/");
		strcat(mouseDev, mouse.c_str());
		fd = open(mouseDev, O_RDONLY | O_NONBLOCK);
		if (fd > 0)
		{
			printf("Input device is a mouse %s\n", mouse.c_str());
			unsigned long ev_bits[NBITS(EV_MAX)];

			char name[256] = "Unknown";
			ioctl(fd, EVIOCGNAME(sizeof(name)), name);
			printf("Input device name: \"%s\"\n", name);
			strupr(name);
			char *ptr = strstr(name, mouse_name.c_str());

			if (ioctl(fd, EVIOCGBIT(0, sizeof(ev_bits)), ev_bits) == -1)
			{
				printf("ioctl error\n");
				close(fd);
				fd = -1;
			}

			if (test_bit(EV_KEY, ev_bits) && test_bit(EV_REL, ev_bits))
			{
				printf("Input device is a mouse\n");
				if (ptr == NULL && mouse_name.length() > 0)
				{
					close(fd);
					fd = -1;
				}
			}
		}
	}
}


bool Mouse::GetMouseAttached()
{
	if (fd > 0)
		return true;
	return false;
}

bool Mouse::read_mouse_event()
{
	int bytes;
	if (fd > 0)
	{
		bytes = read(fd, (void *)&mouse_event, sizeof(struct input_event));
		if (bytes == -1)
			return false;
		if (bytes == sizeof(struct input_event))
			return true;
	}
	return false;
}

MouseState Mouse::GetMouseState()
{
	int bytes, count = 0;
	char name[256];

	state.MouseActivity = false;
	state.Rotated = 0;

	if (fd > 0 && ioctl(fd, EVIOCGNAME(sizeof(name)), name) < 0)
	{
		fd = 0;
		printf("Mouse Removed \n");
	}

	if (fd <= 0)
		init_mouse();
	else
	{
		bytes = read(fd, (void *)&mouse_event, sizeof(struct input_event));
		while (bytes > 0)
		{
			//printf("type %d code %d value %d\n", mouse_event.type, mouse_event.code, mouse_event.value);

			if (mouse_event.type == EV_KEY && mouse_event.code == BTN_MIDDLE && mouse_event.value == 1)
			{
					step = step * 10;
					if (step > 100)
						step = 1;
			}
			
			if (mouse_event.type == EV_REL && mouse_event.code == REL_WHEEL)
			{
				vfo.step_vfo(step * mouse_event.value);
				state.Rotated = mouse_event.value;
			}

			if (mouse_event.type == EV_REL && mouse_event.code == REL_X )
			{
				state.x = state.x + mouse_event.value;
				if (state.x >= screenWidth)
					state.x = screenWidth - 1;
				if (state.x < 0)
					state.x = 0;
				//printf("value %d, x %d\n", mouse_event.value, state.x);
				state.MouseActivity = true;
			}
			if (mouse_event.type == EV_REL && mouse_event.code == REL_Y)
			{
				state.y = state.y + mouse_event.value;
				if (state.y >= screenHeight)
					state.y = screenHeight - 1;
				if (state.y < 0)
					state.y = 0;
				//printf("value %d, y %d\n", mouse_event.value, state.y);
				state.MouseActivity = true;
			}

			if (mouse_event.type == EV_KEY && mouse_event.code == BTN_LEFT)
			{
				//printf("type %d code %d value %d\n", mouse_event.type, mouse_event.code, mouse_event.value);

				if (mouse_event.value == 1)
				{
					state.pressed = LV_INDEV_STATE_PR;
					state.MouseActivity = true;
					std::chrono::time_point<std::chrono::system_clock> current_time = std::chrono::system_clock::now();
					if (current_time - last_click_time < double_click_threshold)
					{
						click_count++;
						if (click_count == 2)
						{
							//cout << "Double click detected!" << endl;
							click_count = 0;
							state.doubleclick = true;
						}
					}
					else
					{
						click_count = 1;
						state.doubleclick = false;
					}
					last_click_time = current_time;
				}
				else
				{
					state.pressed = LV_INDEV_STATE_REL;
					state.MouseActivity = true;
				}
			}
		bytes = read(fd, (void *)&mouse_event, sizeof(struct input_event));
		}
	}
	return state;
}
