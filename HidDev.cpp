#include "HidDev.h"
#include "gui_setup.h"


/* Jabra Vendor Id */
#define JABRA_VID ((__u16)0x0B0E)
#define TelephonyUsagePage ((__u16)0x000B)
#define ConsumerUsagePage ((__u16)0x000C)
#define LEDUsagePage ((__u16)0x0008)
#define ButtonUsagePage ((__u16)0x0009)

/* HID Usage Id definitions:  Telephony usage page (0x0B) */
#define Tel_Hook_Switch ((__u16)0x0020)
#define Tel_Flash ((__u16)0x0021)
#define Tel_Feature ((__u16)0x0022)
#define Tel_Hold ((__u16)0x0023)
#define Tel_Redial ((__u16)0x0024)
#define Tel_Transfer ((__u16)0x0025)
#define Tel_Drop ((__u16)0x0026)
#define Tel_Park ((__u16)0x0027)
#define Tel_Forward ((__u16)0x0028)
#define Tel_Alternate ((__u16)0x0029)
#define Tel_Line ((__u16)0x002A)
#define Tel_Speaker ((__u16)0x002B)

extern void strupr(char *str);
extern HidDev HidDev_dev;

HidDev::HidDev()
{
	m_fd = -1;
	numIndex = 2;
	step = 5;// steps of 100 Hz
	bstep = false;
	last_val = 0;
	speed = 0;
	value = 0;
	last_time = std::chrono::high_resolution_clock::now();
	usb_hid = false;
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
	if (mouse_name == "GN Audio A/S Jabra Evolve2 30 Consumer Control")
	{
		usb_hid = true;
		HidName = mouse_name;
	}
	transform(HidName.begin(), HidName.end(), HidName.begin(), ::toupper);
	numIndex = 0;
	do
	{
		if (usb_hid)
			sprintf(HidDevName, "/dev/usb/hiddev%d", numIndex);
		else
			sprintf(HidDevName, "/dev/input/event%d", numIndex);
		m_fd = open(HidDevName, O_RDONLY | O_NONBLOCK);
		if (m_fd > 0 && !usb_hid)
		{
			char name[256] = "Unknown";
			ioctl(m_fd, EVIOCGNAME(sizeof(name)), name);
			printf("Input device name: \"%s\"\n", name);
			strupr(name);
			char *ptr = strstr(name, HidName.c_str());
			if (ptr == NULL)
			{
				close(m_fd);
				m_fd = -1;
			}
		}
		if (m_fd > 0 && usb_hid)
		{
			struct hiddev_devinfo devinfo;

			ioctl(m_fd, HIDIOCGDEVINFO, &devinfo);
			if (devinfo.vendor != JABRA_VID)
			{
				close(m_fd);
				m_fd = -1;
			}	
		}
	if (m_fd == -1)
		numIndex++;
	} while (numIndex < 30 && m_fd == -1);
}

bool HidDev::read_event()
{
	int bytes;
	if (m_fd > 0 && !usb_hid)
	{
		bytes = read(m_fd, (void *)&in_event, sizeof(struct input_event));
		if (bytes == -1)
			return false;
		if (bytes == sizeof(struct input_event) )
		{
			printf("%s type %d code %d, value %d\n", HidName.c_str(),in_event.type, in_event.code, in_event.value);
			return true;
		}
	}
	if (m_fd > 0 && usb_hid)
	{
		bytes = read(m_fd, (void *)&hid_event, sizeof(struct hiddev_event));
		if (bytes == -1)
			return false;
		if (bytes == sizeof(struct hiddev_event))
		{
			printf("%s hid %d, value %d\n", HidName.c_str(), hid_event.hid, hid_event.value);
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

	if (bevent && usb_hid)
	{
		unsigned int a = hid_event.hid >> 16;
		unsigned int b = hid_event.hid & 0xFFFF;

		if (a == TelephonyUsagePage && hid_event.value == 1 && b == Tel_Hook_Switch)
		{	
		if (SdrDevices.get_tx_channels(default_radio))
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
				gbar.set_tx(txstate);
			}
		}
		return;
	}

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
			{
				gsetup.toggle_cw();
			}
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
			if (in_event.value == 1 && SdrDevices.get_tx_channels(default_radio))
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
				gbar.set_tx(txstate);
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