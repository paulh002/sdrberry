#include "sdrberry.h"
#include "Catinterface.h"
#include "vfo.h"

Comm::~Comm()
{
	if (serialport > 0)
		serialClose(serialport);
}


bool Comm::begin()
{
	device = Settings_file.find_cat("USB");
	if (device.length() == 0)
		device = "/dev/serial/by-id/usb-Silicon_Labs_CP2102_USB_to_UART_Bridge_Controller_0001-if00-port0";
	speed = 115200;
	serialport = serialOpen(device.c_str(), speed);
	if (serialport < 0)
	{
		bserial = false;
		return false;
	}
	bserial = true;
	printf("Connect to ESP32 CAT interface\n");
	return true;
}

void Comm::Send(std::string s) 
{
	serialPuts(serialport, (const char *)s.c_str());
}

void Comm::Read(char c, std::string& s)
{
	char chr;
	s.clear();
	do
	{
		chr = serialGetchar(serialport);
		if (chr == '\n' || chr == '\r')
			continue;
		s.push_back(chr);
	} while (chr != c);
}

bool Comm::available()
{
	if (serialDataAvail(serialport))
		return true;
	else
		return false;
}

void	Catinterface::begin()
{
	bcomm_port = comm_port.begin();
	cat_message.begin(true, &comm_port, true);	
	m_mode = 0;
}

void	Catinterface::checkCAT()
{
	if (!bcomm_port)
		return;
	if (cat_message.CheckCAT(false))
	{
		// do something
		int count = cat_message.GetFT();
		if (count)
		{
			vfo.step_vfo(count, true);	
			cat_message.SetFA(vfo.get_active_vfo());
		}
		count = cat_message.GetAG();
		if (count)
			set_vol_slider(count);
		count = cat_message.GetRG();
		if (count)
			set_gain_slider(count);
		count = cat_message.GetTX();
		if (m_mode != count)
		{
			m_mode = count;
			switch (m_mode)
			{
			case 0:
				select_mode(mode);
				break;
			case 1:
				select_mode_tx(mode);
				break;
			case 2:
				select_mode_tx(mode, 1);
				break;
			}
		}
	}
}

void Catinterface::operator()()
{
	while (1)
	{
		checkCAT();	
	}
}

void Catinterface::SetBand(uint16_t band)	
{
	cat_message.SetBand(band);
}