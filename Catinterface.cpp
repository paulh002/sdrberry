#include "sdrberry.h"
#include "Catinterface.h"
#include "vfo.h"

Comm::~Comm()
{
	if (serialport > 0)
		serialClose(serialport);
}

void Comm::Close()
{
	if (serialport > 0)
		serialClose(serialport);
	serialport = 0;
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
		serialport = 0;
		return false;
	}
	printf("Connect to ESP32 CAT interface\n");
	return true;
}

void Comm::Send(std::string s) 
{
	serialPuts(serialport, (const char *)s.c_str());
}

int Comm::Read(char c, std::string& s)
{
	int chr; int i = 0;
	s.clear();
	do
	{
		//chr = serialGetchar(serialport);
		int ret = serialReadchar(serialport, &chr);
		if (ret < 0)
			return -1;
		if (ret == 1)
		{
			if (chr == '\n' || chr == '\r')
				continue;
			s.push_back((char)chr);
		}		
		i++;
	} while (chr != c && i < 80);
	return s.length();
}

bool Comm::available()
{
	if (serialDataAvail(serialport))
		return true;
	else
		return false;
}

/* this methode is intened to send range details over volume, gain, bands etc*/

void Comm::SendInformation(int info)
{
	switch (info)
	{
	case 0:
		// Volume
		{
			char str[20];
			int range = gbar.get_vol_range();
			sprintf(str, "GT0%d", range);
			Send((std::string) str);
		}
		break;
	case 1:
		// Gain
		{
			char str[20];
			int  max_gain, min_gain;
			
			gagc.get_gain_range(max_gain, min_gain);
			sprintf(str, "GT1%2d,%2d", max_gain, min_gain);
			Send((std::string) str);
		}
		break;
	case 2:
		// Band
		{
			char			str[20];
			vector<int>		bands;
			string			s;
			
			strcpy(str, "GT2");
			s = str;
			vfo.return_bands(bands);
			for (auto it : bands)
			{
				sprintf(str, ",%2d", it);
				s.append(str);
			}
			s.push_back(';');
			Send((std::string) s);
		}
		break;
	case 3:
		// Filter
		{
			char			str[20];
			vector<string>	filters;
			string			s;
			
			strcpy(str, "GT3");
			s = str;
			gbar.get_filter_range(filters);
			for (auto it : filters)
			{
				s.push_back(',');
				s.append(it);
			}
			s.push_back(';');
			Send((std::string) s);
		}
		break;
	}
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
	int ret = cat_message.CheckCAT(false);
	if(ret < 0)
	{
		comm_port.Close();
		usleep(1000000);
		if (!comm_port.begin())
		{
			usleep(1000000);
			return;
		}
	}
	if (ret == 1)
	{
		// do something
		int count = cat_message.GetFT();
		if (count)
		{
			vfo.step_vfo(count, true);	
			cat_message.SetFA(vfo.get_active_vfo_freq());
		}
		count = cat_message.GetAG();
		if (count)
			gbar.set_vol_slider(count);
		count = cat_message.GetRG();
		if (count)
			gagc.set_gain_slider(count);
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
		count = cat_message.GetSH();
		gbar.set_filter_slider(count);	
		count = cat_message.GetBand();
		if (vfo.get_band_no(vfo.get_active_vfo()) != count && count != 0)
		{
			gui_band_instance.set_gui(count);
			int index  = getIndex(Settings_file.meters, count);
			if (index >= 0)
			{
				long f_low = Settings_file.f_low.at(index);
				int f_band = Settings_file.meters.at(index);
				vfo.set_band(f_band, f_low);				
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

void Catinterface::SetAG(uint8_t volume)	
{
	cat_message.SetAG(volume);
}	

void Catinterface::SetSH(int bandwidth)	
{
	cat_message.SetSH(0,bandwidth);
}	

void Catinterface::SetFA(uint32_t freq)	
{
	cat_message.SetFA(freq);
}	
