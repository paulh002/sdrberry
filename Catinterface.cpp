#include "Catinterface.h"
#include "vfo.h"
#include "SharedQueue.h"

extern SharedQueue<GuiMessage> guiQueue;

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

bool Comm::IsCommuncationPortOpen()
{
	return serialport > 0;
}

int Comm::Read(char c, std::string &s)
{
	int chr;
	int i = 0;
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
			Send((std::string)str);
		}
		break;
	case 1:
		// Gain
		{
			char str[20];
			int max_gain, min_gain;

			gbar.get_gain_range(max_gain, min_gain);
			sprintf(str, "GT1%2d,%2d", max_gain, min_gain);
			Send((std::string)str);
		}
		break;
	case 2:
		// Band
		{
			char str[20];
			vector<int> bands;
			string s;

			strcpy(str, "GT2");
			s = str;
			vfo.return_bands(bands);
			for (auto it : bands)
			{
				sprintf(str, ",%2d", it);
				s.append(str);
			}
			s.push_back(';');
			Send((std::string)s);
		}
		break;
	case 3:
		// Filter
		{
			char str[20];
			vector<string> filters;
			string s;

			strcpy(str, "GT3");
			s = str;
			gbar.get_filter_range(filters);
			for (auto it : filters)
			{
				s.push_back(',');
				s.append(it);
			}
			s.push_back(';');
			Send((std::string)s);
		}
		break;
	}
}

void Catinterface::begin()
{
	comm_port.begin();
	cat_message.begin(true, &comm_port, true);
	channel = cat_message.OpenCatChannel();
	m_mode = 0;
	ifgain = 0;
	volume = 0;
	rfgain = 0;
}

void Catinterface::checkCAT()
{
	if (!comm_port.IsCommuncationPortOpen())
		return;
	int ret = cat_message.CheckCAT(false);
	if (ret < 0)
	{
		comm_port.Close();
		usleep(1000000);
		if (!comm_port.begin())
		{
			usleep(1000000);
			return;
		}
	}
	if (ret == 1 && !bpause_cat.load())
	{
		// do something
		int count = cat_message.GetFT();
		if (count)
		{
			//vfo.step_vfo(count, true);
			if (!muteFA.load())
			{
				guiQueue.push_back(GuiMessage(GuiMessage::action::step, count));
				cat_message.SetFA(vfo.get_active_vfo_freq());
			}
		}
		count = cat_message.GetIG();
		if (count && ifgain != count)
		{
			ifgain = count;
			guiQueue.push_back(GuiMessage(GuiMessage::action::setifgain, count));
		}
		count = cat_message.GetAG();
		if (count && volume != count)
		{
			volume = count;
			guiQueue.push_back(GuiMessage(GuiMessage::action::setvol, count));
		}
		count = cat_message.GetRG();
		if (count && rfgain)
		{
			rfgain = count;
			guiQueue.push_back(GuiMessage(GuiMessage::action::setifgain, count));
		}
		if (!(mode == mode_ft8 || mode == mode_ft4))
		{
			int rxtxCatMessage = cat_message.GetTX();
			if (m_mode != rxtxCatMessage)
			{
				m_mode = rxtxCatMessage;
				switch (m_mode)
				{
				case TX_OFF:
					select_mode(mode);
					break;
				case TX_CAT:
					select_mode_tx(mode, audioTone::NoTone, TX_CAT);
					break;
				case TX_TUNE_CAT:
					select_mode_tx(mode, audioTone::SingleTone, TX_TUNE_CAT);
					break;
				}
			}
		}
		count = cat_message.GetSH();
		guiQueue.push_back(GuiMessage(GuiMessage::action::filter, count));
		count = cat_message.GetBand();
		if (vfo.get_band_no(vfo.get_active_vfo()) != count && count != 0)
		{
			guiQueue.push_back(GuiMessage(GuiMessage::action::setband, count));
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
	cat_message.SetSH(0, bandwidth);
}

void Catinterface::SetFA(uint32_t freq)
{
	cat_message.SetFA(freq);
}

void Catinterface::SetSM(uint8_t sm)
{
	cat_message.SetSM(sm);
}

uint8_t Catinterface::GetIG()
{
	return cat_message.GetIG();
}

void Catinterface::SetIG(int ig)
{
	cat_message.SetIG((uint8_t)ig);
}

void Catinterface::SetRG(int rg)
{
	cat_message.SetRG((uint8_t)rg);
}

uint8_t Catinterface::GetRG()
{
	return cat_message.GetRG();
}

void Catinterface::SetTX(int tx)
{
	cat_message.SetTX((uint8_t)tx);
}