#include "Mouse.h"
#include "sdrberry.h"
#include <iostream>
#include <fstream>
#include <dirent.h>
#include "SharedQueue.h"

#define test_bit(bit, array) (array[bit / 8] & (1 << (bit % 8)))
#define NBITS(x) ((((x)-1) / (sizeof(long) * 8)) + 1)

static std::chrono::milliseconds double_click_threshold(300); 

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

bool Mouse::find_mouse(const char *device_path)
{
	fd = open(device_path, O_RDONLY | O_NONBLOCK);
	if (fd < 0)
	{
		perror("Failed to open device");
		return 0;
	}
	
	char name[256] = "Unknown";
	ioctl(fd, EVIOCGNAME(sizeof(name)), name);
	printf("Input device name: \"%s\"\n", name);
	if (strstr(name, "hdmi") != nullptr)
	{
		close(fd);
		fd = -1;
		return 0;
	}
	
	// To detect if it's a mouse, we use the ioctl function with EVIOCGBIT.
	unsigned long evbit[(EV_MAX + 7) / 8] = {0};
	if (ioctl(fd, EVIOCGBIT(0, EV_MAX), evbit) < 0)
	{
		perror("ioctl failed");
		close(fd);
		fd = -1;
		return 0;
	}

	// Check if the device supports EV_REL (relative movement events) and
	// REL_X and REL_Y (axis for mouse movement).
	int is_mouse = evbit[EV_REL / 8] & (1 << (EV_REL % 8));
	if (is_mouse)
	{
		unsigned long relbit[(REL_MAX + 7) / 8] = {0};
		if (ioctl(fd, EVIOCGBIT(EV_REL, REL_MAX), relbit) >= 0)
		{
			if ((relbit[REL_X / 8] & (1 << (REL_X % 8))) &&
				(relbit[REL_Y / 8] & (1 << (REL_Y % 8))))
			{
				int keyBitmask[KEY_MAX / (sizeof(int) * 8) + 1];
				if (ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keyBitmask)), keyBitmask) == -1)
				{
					is_mouse = 0;
				}
				else
				{
					if (keyBitmask[KEY_A / (sizeof(int) * 8)] & (1 << (KEY_A % (sizeof(int) * 8))))
					{
						//printf("Device is also a keyboard\n");
						is_mouse = 0;
					}
					else
					{
						is_mouse = 1;
					}
				}
			}
			else
			{
				is_mouse = 0;
			}
		}
	}

	if (!is_mouse)
	{
		close(fd);
		fd = -1;
	}
	else
	{
		printf("Mouse device name: \"%s\"\n", name);
	}
	return is_mouse;
}

void Mouse::init_mouse()
{
	struct dirent *entry;
	DIR *dp = opendir("/dev/input");

	if (dp == NULL)
	{
		perror("opendir failed");
		return;
	}

	while ((entry = readdir(dp)))
	{
		// Only consider "event" devices
		if (strncmp(entry->d_name, "event", 5) == 0)
		{
			char device_path[256];
			snprintf(device_path, sizeof(device_path), "/dev/input/%s", entry->d_name);
			if (find_mouse(device_path))
			{
				break;
			}
		}
	}
	closedir(dp);
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
	int rel_1, rel_2, screen_w, screen_h;

	state.MouseActivity = false;
	state.Rotated = 0;
	if (IsScreenRotated())
	{
		rel_2 = REL_X;
		rel_1 = REL_Y;
		screen_h = screenWidth;
		screen_w = screenHeight;
	}
	else
	{
		rel_1 = REL_X;
		rel_2 = REL_Y;
		screen_w = screenWidth;
		screen_h = screenHeight;
	}

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
				guiQueue.push_back(GuiMessage(GuiMessage::action::change_step, count));
			}
			
			if (mouse_event.type == EV_REL && mouse_event.code == REL_WHEEL)
			{
				vfo.step_vfo(step * mouse_event.value);
				state.Rotated = mouse_event.value;
			}

			if (mouse_event.type == EV_REL && mouse_event.code == rel_1 )
			{
				if (IsScreenRotated() == 1) // || IsScreenRotated() == 3
					state.x = state.x + mouse_event.value * -1;
				else
					state.x = state.x + mouse_event.value;
				if (state.x >= screen_w)
					state.x = screen_w - 1;
				if (state.x < 0)
					state.x = 0;
				//printf("value %d, x %d\n", mouse_event.value, state.x);
				state.MouseActivity = true;
			}
			if (mouse_event.type == EV_REL && mouse_event.code == rel_2)
			{
				if (IsScreenRotated() == 3) // added this to invert the values
					state.y = state.y + mouse_event.value * -1;
				else
					state.y = state.y + mouse_event.value;
				if (state.y >= screen_h)
					state.y = screen_h - 1;
				if (state.y < 0)
					state.y = 0;
				//printf("value %d, y %d\n", mouse_event.value, state.y);
				state.MouseActivity = true;	
			}

			if (mouse_event.type == EV_REL && (mouse_event.code == REL_Y || mouse_event.code == REL_X))
			{
				if (IsScreenRotated())
				{
					//state.x = screenWidth - state.x;
					//state.y = screenHeight - state.y;
				}
			}

			if (mouse_event.type == EV_KEY && mouse_event.code == BTN_RIGHT)
			{
				//printf("type %d code %d value %d\n", mouse_event.type, mouse_event.code, mouse_event.value);
				if (mouse_event.value == 1)
				{
					state.pressed = LV_INDEV_STATE_PR;
					state.MouseActivity = true;
					state.btn_id = BTN_RIGHT;
				}

				if (mouse_event.value == 0)
				{
					state.pressed = LV_INDEV_STATE_REL;
					state.MouseActivity = true;
					state.btn_id = BTN_RIGHT;
				}
			}

			if (mouse_event.type == EV_KEY && mouse_event.code == BTN_LEFT)
			{
				//printf("type %d code %d value %d\n", mouse_event.type, mouse_event.code, mouse_event.value);

				if (mouse_event.value == 1)
				{
					state.btn_id = BTN_LEFT;
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
				if (mouse_event.value == 0)
				{
					state.pressed = LV_INDEV_STATE_REL;
					state.MouseActivity = true;
					state.btn_id = BTN_LEFT;
				}
			}
		bytes = read(fd, (void *)&mouse_event, sizeof(struct input_event));
		}
	}
	//if (state.MouseActivity)
	//	printf("x %d y %d pressed %d activity %d \n", state.x, state.y, state.pressed, state.MouseActivity);
	return state;
}
