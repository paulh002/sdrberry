#include "Mouse.h"


Mouse::Mouse()
{
	m_fd = -1;
	numMouseIndex = 2;
	step = 10;
	bstep = false;
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
	if(mouse_name.size() == 0)
		mouse_name = "MOUSE";
	transform(mouse_name.begin(), mouse_name.end(), mouse_name.begin(), ::toupper);
	
	numMouseIndex = 0;
	do
	{
		sprintf(mouseDev, "/dev/input/event%d", numMouseIndex);
		m_fd = open(mouseDev, O_RDONLY | O_NONBLOCK);
		if (m_fd > 0)
		{
			char name[256] = "Unknown";
			ioctl(m_fd, EVIOCGNAME(sizeof(name)), name);
			printf("Input device name: \"%s\"\n", name);
			strupr(name);
			char *ptr = strstr(name, mouse_name.c_str());
			if (ptr == NULL)
			{
				close(m_fd);
				numMouseIndex++;
				m_fd = -1;
			}
		}
		else
			numMouseIndex++;	
	} while (numMouseIndex < 4 && m_fd == -1);	
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


bool  Mouse::read_mouse_event()
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
	
	if (mouse_event.type == EV_KEY && mouse_event.code == 273  && mouse_event.value == 1 && bstep == false && step > 1)
	{
		bstep = true;
		step = step / 10;
	}
	if (mouse_event.type == EV_KEY && mouse_event.code == 273 && mouse_event.value == 0)
		bstep = false;
	
	if (i > 0)
		vfo.step_vfo(step, true);
	if (i < 0)
		vfo.step_vfo(-1 * step, true);
}