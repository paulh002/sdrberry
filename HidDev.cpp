#include "HidDev.h"
#include "gui_setup.h"

extern void strupr(char *str);

HidDev::HidDev()
{
	m_fd = -1;
	numIndex = 2;
	step = 10;
	bstep = false;
	last_val = 0;
	speed = 0;
	value = 0;
	status = 0;
	last_time = std::chrono::high_resolution_clock::now();
}

HidDev::~HidDev()
{
	if (m_fd > 0)
	{
		close(m_fd);
	}
}

void HidDev::init(string mouse_name)
{
	// first find a mouse
	if (HidName.size() == 0)
		HidName = "CONTOUR DESIGN SHUTTLEXPRESS";
	transform(HidName.begin(), HidName.end(), HidName.begin(), ::toupper);

	numIndex = 0;
	do
	{
		sprintf(HidDevName, "/dev/input/event%d", numIndex);
		m_fd = open(HidDevName, O_RDONLY | O_NONBLOCK);
		if (m_fd > 0)
		{
			char name[256] = "Unknown";
			ioctl(m_fd, EVIOCGNAME(sizeof(name)), name);
			printf("Input device name: \"%s\"\n", name);
			strupr(name);
			char *ptr = strstr(name, HidName.c_str());
			if (ptr == NULL)
			{
				close(m_fd);
				numIndex++;
				m_fd = -1;
			}
		}
		else
			numIndex++;
	} while (numIndex < 10 && m_fd == -1);
}

bool HidDev::read_event()
{
	int bytes;
	if (m_fd > 0)
	{
		bytes = read(m_fd, (void *)&in_event, sizeof(struct input_event));
		if (bytes == -1)
			return false;
		if (bytes == sizeof(struct input_event))
		{
			printf("CONTOUR DESIGN type %d code %d, value %d\n", in_event.type, in_event.code, in_event.value);
			return true;
		}
	}
	return false;
}


void HidDev::step_vfo()
{
	if (!read_event())
	{
		auto now = std::chrono::high_resolution_clock::now();
		const auto timePassed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time);
		if (timePassed.count() > 10000 && status > 0)
		{
			status = 0;
		}
	}
	
	if (in_event.type == EV_KEY && in_event.value == 1)
	{
		switch (in_event.code)
		{
		case 260:
			gsetup.toggle_cw();
			break;
		case 261:
			break;
		case 262:
			break;
		case 263:
			break;
		case 264:
			status++;
			if (status > 2)
				status = 0;
			break;
		}
	}

	if (in_event.type == EV_REL && in_event.code == 11)
	{
		switch (in_event.value)
		{
		case -240:
			value = -1;
			break;
		case -360:
			value = -2;
			break;
		case -480:
			value = -3;
			break;
		case 240:
			value = 1;
			break;
		case 360:
			value = 2;
			break;
		case 480:
			value = 3;
			break;
		}
	}
	
	if (in_event.type == EV_REL && in_event.code == 11 && (in_event.value == 120 || in_event.value == -120))
		value = 0;
	
	if (value != 0 && status == 1)
	{
		auto now = std::chrono::high_resolution_clock::now();
		const auto timePassed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time);
		if (timePassed.count() > 20 && value == 1)
		{
			gbar.step_vol_slider(1);
			last_time = now;
		}
		if (timePassed.count() > 10 && value == 2)
		{
			gbar.step_vol_slider(1);
			last_time = now;
		}
		if (timePassed.count() > 5 && value == 3)
		{
			gbar.step_vol_slider(1);
			last_time = now;
		}
		if (timePassed.count() > 1 && value == 4)
		{
			gbar.step_vol_slider(1);
			last_time = now;
		}
		if (timePassed.count() > 20 && value == -1)
		{
			gbar.step_vol_slider(-1);
			last_time = now;
		}
		if (timePassed.count() > 10 && value == -2)
		{
			gbar.step_vol_slider(-1);
			last_time = now;
		}
		if (timePassed.count() > 5 && value == -3)
		{
			gbar.step_vol_slider(-1);
			last_time = now;
		}
		if (timePassed.count() > 1 && value == -4)
		{
			gbar.step_vol_slider(-1);
			last_time = now;
		}
		return;
	}

	if (value != 0 && status == 2)
	{
		auto now = std::chrono::high_resolution_clock::now();
		const auto timePassed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time);
		if (timePassed.count() > 20 && value == 1)
		{
			gbar.step_gain_slider(1);
			last_time = now;
		}
		if (timePassed.count() > 10 && value == 2)
		{
			gbar.step_gain_slider(1);
			last_time = now;
		}
		if (timePassed.count() > 5 && value == 3)
		{
			gbar.step_gain_slider(1);
			last_time = now;
		}
		if (timePassed.count() > 1 && value == 4)
		{
			gbar.step_gain_slider(1);
			last_time = now;
		}
		if (timePassed.count() > 20 && value == -1)
		{
			gbar.step_gain_slider(-1);
			last_time = now;
		}
		if (timePassed.count() > 10 && value == -2)
		{
			gbar.step_gain_slider(-1);
			last_time = now;
		}
		if (timePassed.count() > 5 && value == -3)
		{
			gbar.step_gain_slider(-1);
			last_time = now;
		}
		if (timePassed.count() > 1 && value == -4)
		{
			gbar.step_gain_slider(-1);
			last_time = now;
		}
		return;
	}
	
	if (value != 0 && status == 0)
	{
		auto now = std::chrono::high_resolution_clock::now();
		const auto timePassed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time);
		if (timePassed.count() > 20 && value == 1)
		{
			vfo.step_vfo(step, false);
			last_time = now;
		}
		if (timePassed.count() > 10 && value == 2)
		{
			vfo.step_vfo(step, false);
			last_time = now;
		}
		if (timePassed.count() > 1 && value == 3)
		{
			vfo.step_vfo(step, false);
			last_time = now;
		}
		if (timePassed.count() > 20 && value == -1)
		{
			vfo.step_vfo(-1 * step, false);
			last_time = now;
		}
		if (timePassed.count() > 10 && value == -2)
		{
			vfo.step_vfo(-1 * step, false);
			last_time = now;
		}
		if (timePassed.count() > 1 && value == -3)
		{
			vfo.step_vfo(-1 * step, false);
			last_time = now;
		}
		return;
	}
	
	if (in_event.type == EV_REL && in_event.code == 7 && in_event.value > last_val)
		vfo.step_vfo(step, false);
	if (in_event.type == EV_REL && in_event.code == 7 && in_event.value < last_val)
		vfo.step_vfo(-1 * step, false);
	if (in_event.type == EV_REL && in_event.code == 7)
		last_val = in_event.value;
}