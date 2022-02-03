#include "HidDev.h"
#include "gui_setup.h"

extern void strupr(char *str);
extern HidDev HidDev_dev;

HidDev::HidDev()
{
	m_fd = -1;
	numIndex = 2;
	step = 10;
	bstep = false;
	last_val = 0;
	speed = 0;
	value = 0;
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
			//printf("CONTOUR DESIGN type %d code %d, value %d\n", in_event.type, in_event.code, in_event.value);
			return true;
		}
	}
	return false;
}


void HidDev::step_vfo()
{
	// Check if a shuttle event is happening
	// If no event within 10 sec clear status for jog so the default is resored

	bool bevent = read_event();
	// Check for key press
	if (bevent && in_event.type == EV_KEY)
	{
		switch (in_event.code)
		{
			// first key switches to rotery encoding function
		case 260:
			if (in_event.value == 1)
			{
				if (encoder)
					encoder = false;
				else
					encoder = true;
			}
			break;
		case 261:
			// 2nd key hotkey for CW decoding
			if (in_event.value == 1)
				gsetup.toggle_cw();
			break;
		case 262:
			// Middle button is used as keypress for rotery encoder option
			if (in_event.value == 1)
				enc_pressed = true;
			else
				enc_pressed = false;
			break;
		case 263:
			break;
		case 264:
			// Txset_tx_state
			if (in_event.value == 1)
			{
				if (!txstate)
				{
					txstate = true;
					select_mode_tx(mode);
				}
				else
				{
					txstate = false;
					select_mode(mode);
				}
			}
			break;
		}
	}

	if (bevent && in_event.type == EV_REL && in_event.code == 11)
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

	if (bevent && in_event.type == EV_REL && in_event.code == 11 && (in_event.value == 120 || in_event.value == -120))
		value = 0;
		
	if (value != 0)
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

	if (bevent && in_event.type == EV_REL && in_event.code == 7)
	{
		rotate(in_event.value);
		if (!encoder)
			rotate_vfo();
	}
}

void HidDev::rotate_vfo()
{
	if (enc_moved > 0)
		vfo.step_vfo(step, false);
	if (enc_moved < 0)
		vfo.step_vfo(-1 * step, false);
	enc_moved = 0;
}

// translate the 1 to 255 to direction clock wise enc_moved =1
// anti clocke wise enc_moved = -1

void HidDev::rotate(int value)
{
	if (value == last_val)
		return;
	if (last_val == 0 && (value == 1 || value == 255)) // initial
	{ // This is the initial move when shuttle starts up
		if (value == 1)
			enc_moved = 1;
		else
			enc_moved = -1;
		last_val = value;
		return;
	}

	if (last_val < 255 && last_val != 1)
	{
		if (value > last_val)
			enc_moved = 1;
		else
			enc_moved = -1;
		last_val = value;
		return;
	}

	if (value == 1 && last_val == 255)
	{
		enc_moved = 1;
		last_val = value;
		return;
	}

	if (value == 255 && last_val == 1)
	{
		enc_moved = -1;
		last_val = value;
	}
	last_val = value;
}

lv_indev_state_t HidDev::encoder_key_press()
{
	if (!encoder)
		return LV_INDEV_STATE_REL;
	if (enc_pressed)
		return LV_INDEV_STATE_PRESSED;
	return LV_INDEV_STATE_REL;
}

int HidDev::encoder_rotate()
{
	if (encoder)
	{
		int i = enc_moved;
		enc_moved = 0;
		return i;
	}
	return 0;
}