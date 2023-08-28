#include "Mouse.h"


#define test_bit(bit, array) (array[bit / 8] & (1 << (bit % 8)))
#define NBITS(x) ((((x)-1) / (sizeof(long) * 8)) + 1)

std::chrono::milliseconds double_click_threshold(300); 

Mouse::Mouse()
{
	m_fd = -1;
	numMouseIndex = 2;
	step = 10;
	bstep = false;
	MouseActivity = false;
	last_click_time = std::chrono::system_clock::now();
	click_count = 0;
}

Mouse::Mouse(int mousefd)
{
	m_fd = mousefd;
	numMouseIndex = 2;
	step = 10;
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

void Mouse::init_mouse(string mouse_name)
{
	// first find a mouse
	//if (mouse_name.size() == 0)
	//	mouse_name = "MOUSE";
	transform(mouse_name.begin(), mouse_name.end(), mouse_name.begin(), ::toupper);
	numMouseIndex = 0;
	do
	{
		sprintf(mouseDev, "/dev/input/event%d", numMouseIndex);
		m_fd = open(mouseDev, O_RDONLY | O_NONBLOCK);
		if (m_fd > 0)
		{
			unsigned long ev_bits[NBITS(EV_MAX)];

			char name[256] = "Unknown";
			ioctl(m_fd, EVIOCGNAME(sizeof(name)), name);
			printf("Input device name: \"%s\"\n", name);
			strupr(name);
			char *ptr = strstr(name, mouse_name.c_str());

			if (ioctl(m_fd, EVIOCGBIT(0, sizeof(ev_bits)), ev_bits) == -1)
			{
				printf("ioctl error\n");
				close(m_fd);
				numMouseIndex++;
				m_fd = -1;
			}
			
			if (test_bit(EV_KEY, ev_bits) && test_bit(EV_REL, ev_bits))
			{
				printf("Input device is a mouse\n");
				if (ptr == NULL && mouse_name.length() > 0)
				{
					close(m_fd);
					numMouseIndex++;
					m_fd = -1;
				}
			}
			else
			{
				printf("Input device is not a mouse\n");
				close(m_fd);
				numMouseIndex++;
				m_fd = -1;
			}
		}
		else
			numMouseIndex++;
	} while (numMouseIndex < 30 && m_fd == -1);
	printf("end mouse \n");
}

bool Mouse::GetMouseAttached()
{
	if (m_fd > 0)
		return true;
	return false;
}

int Mouse::count()
{
	read_mouse_event();
	//if (mouse_event.type > 0)
	//	printf("type %d code %d value %d\n", mouse_event.type, mouse_event.code, mouse_event.value);
	if (mouse_event.type == EV_REL && mouse_event.code == 11)
		return mouse_event.value;
	else
		return 0;
}

bool Mouse::read_mouse_event()
{
	int bytes;
	if (m_fd > 0)
	{
		bytes = read(m_fd, (void *)&mouse_event, sizeof(struct input_event));
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

	state.MouseActivity = false;
	state.Rotated = 0;
	if (m_fd > 0)
	{
		bytes = read(m_fd, (void *)&mouse_event, sizeof(struct input_event));
		while (bytes > 0)
		{
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
				printf("type %d code %d value %d\n", mouse_event.type, mouse_event.code, mouse_event.value);

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
		bytes = read(m_fd, (void *)&mouse_event, sizeof(struct input_event));
		}
	}
	return state;
}

void Mouse::step_vfo()
{
	int i = count();
	if (mouse_event.type == EV_KEY && mouse_event.code == 272 && mouse_event.value == 1 && bstep == false & step < 100000)
	{
		bstep = true;
		step = step * 10;
	}
	if (mouse_event.type == EV_KEY && mouse_event.code == 272 && mouse_event.value == 0)
		bstep = false;

	if (mouse_event.type == EV_KEY && mouse_event.code == 273 && mouse_event.value == 1 && bstep == false && step > 1)
	{
		bstep = true;
		step = step / 10;
	}
	if (mouse_event.type == EV_KEY && mouse_event.code == 273 && mouse_event.value == 0)
		bstep = false;

	if (i > 0)
		vfo.step_vfo(step);
	if (i < 0)
		vfo.step_vfo(-1 * step);
}