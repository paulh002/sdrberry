#include "CatTcpServer.h"
#include "Catinterface.h"
#include <iostream>
#include <string>
#include <cstring>
#include <ctype.h>
#include "Settings.h"
#include "SharedQueue.h"
#include "lvgl_.h"
#include "sdrberry.h"
#include "gui_bar.h"
#include "debug_print.h"

const int BUFFER_SIZE = 80;

extern SharedQueue<GuiMessage> guiQueue;
bool cat_debug = false;

CatTcpComm::~CatTcpComm()
{

}

void CatTcpComm::Close()
{

}

bool CatTcpComm::begin() 
{
	cat_debug = Settings_file.get_int("CAT", "debug", 0);
	int opt = 1;
	port = Settings_file.get_int("tcp", "port", 5000);
	// Forcefully attaching socket to the port 5000
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed \n");
		return false;
	}

	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
	{
		printf("setsockopt error\n");
		return false;
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	// Forcefully attaching socket to the port 5000
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		printf("bind failed \n");
		return false;
	}
	if (listen(server_fd, 3) < 0)
	{
		printf("listen error\n");
		return false;
	}
	printf("TCP CAT interface listen on port %d\n", port);
	connected = false;
	return true;
}

void CatTcpComm::Send(std::string response)
{
	if (connected && server_fd)
	{
		send(tcp_socket, response.c_str(), response.length(), 0);
		if (Settings_file.get_int("CAT", "debug", 0))
			printf("Cat response %s\n", response.c_str());
	}
}

int CatTcpComm::Read(char c, std::string &message)
{
	char buffer[BUFFER_SIZE] = {0};

	if (!connected)
	{
		int addrlen = sizeof(address);
		if ((tcp_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
		{
			std::cout << "accept error" << std::endl;
			usleep(1000);
			return 0;
		}
		printf("TCP CAT interface accept connection on port %d\n", port);
		connected = true;
	}

	int i = 0;
	char chr{0};

	//memset(buffer, 0, BUFFER_SIZE);
	do
	{
		//printf("TCP CAT read %x\n", chr);
		int valread = read(tcp_socket, &chr, 1);
		if (valread <= 0)
		{
			close(tcp_socket);
			connected = false;
			std::cout << "Client disconnected" << std::endl;
			return -1;
		}
		if (!isascii(chr))
			continue;
		if (chr == '\n' || chr == '\r')
			continue;
		message.push_back((char)chr);
	} while (chr != c);

	if (message.size() && Settings_file.get_int("CAT", "debug", 0))
		printf("Cat message %s\n", message.c_str());
	return message.size();
}

bool CatTcpComm::available()
{
	return server_fd > 0;
}

void CatTcpComm::SendInformation(int info)
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
			std::vector<int> bands;
			std::string s;

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
			std::vector<std::string> filters;
			std::string s;

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

bool CatTcpComm::IsCommuncationPortOpen()
{
	return connected;
}

bool CatTcpServer::StartServer()
{
	vfo_a = 50260000UL;
	vfo_b = 50260000UL;
	filter = gbar.get_filter_frequency(mode);
	mda = Settings_file.convert_mode(Settings_file.get_string("VFO1", "Mode"));
	mdb = Settings_file.convert_mode(Settings_file.get_string("VFO2", "Mode"));
	rit_onoff = 0;
	rit_delta = 0;
	split_onoff = 0;
	stop_flag = false;
	filter = 0;
	if (cattcpcomm.begin())
	{
		cat_message.begin(true, &cattcpcomm, true);
		return true;
	}
	return false;
}

void CatTcpServer::SetST(int st)
{
	split_onoff = st;
	cat_message.SetST(st);
	DEBUG_CAT_PRINTF("set split %d\n", st);
}

void CatTcpServer::SetNA(int ft)
{
	filter = ft;
	cat_message.SetNA(filter);
	DEBUG_CAT_PRINTF("set filter %d\n", filter);
}

void CatTcpServer::SetMDA(int mode)
{
	mda = encode_mode(mode);
	cat_message.SetMDA(mda);
	DEBUG_CAT_PRINTF("set MDA %d\n", mda);
}

void CatTcpServer::SetMDB(int mode)
{
	mdb = encode_mode(mode);
	cat_message.SetMDB(mdb);
	DEBUG_CAT_PRINTF("set MDB %d\n", mdb);
}

void CatTcpServer::StopServer()
{
}

void CatTcpServer::operator()()
{
	int count;
	bpause_cat = false;
	stop_flag = false;
	while (!stop_flag)
	{
		int ret = cat_message.CheckCAT(false);
		if (ret > 0 && !bpause_cat.load())
		{
			count = cat_message.GetFA();
			if (count && vfo_a != count)
			{
				char str[80];

				vfo_a = count;
				sprintf(str, "%d", count);
				if (vfo.get_active_vfo() == vfo_activevfo::One)
					guiQueue.push_back(GuiMessage(GuiMessage::action::setvfo, str));
				else
					guiQueue.push_back(GuiMessage(GuiMessage::action::setvfo1, str));
			}
			
			count = cat_message.GetFB();
			if (count && vfo_b != count)
			{
				char str[80];

				vfo_b = count;
				sprintf(str, "%d", count);
				if (vfo.get_active_vfo() == vfo_activevfo::Two)
					guiQueue.push_back(GuiMessage(GuiMessage::action::setvfo, str));
				else
					guiQueue.push_back(GuiMessage(GuiMessage::action::setvfo2, str));
			}

			count = cat_message.GetNA();
			if (count && filter != count)
			{
				filter = count;
				printf("NA CAT filter %d \n", filter);
				guiQueue.push_back(GuiMessage(GuiMessage::action::filter, count));
			}
			count = cat_message.GetMDA();
			if (count && mda != count)
			{
				mda = count;
				guiQueue.push_back(GuiMessage(GuiMessage::action::setmode_vfo_a, decode_mode(count)));
			}
			count = cat_message.GetMDB();
			if (count && mdb != count)
			{
				mdb = count;
				guiQueue.push_back(GuiMessage(GuiMessage::action::setmode_vfo_b, decode_mode(count)));
			}
			
			count = cat_message.GetST();
			if (split_onoff != count)
			{
				split_onoff = count;
				guiQueue.push_back(GuiMessage(GuiMessage::action::split_onoff, split_onoff));
			}
			
			count = cat_message.GetRT();
			if (rit_onoff != count)
			{
				rit_onoff = count;
				guiQueue.push_back(GuiMessage(GuiMessage::action::rit_onoff, rit_onoff));
			}
			count = cat_message.GetRD();
			if (rit_delta != count)
			{
				rit_delta = count;
				guiQueue.push_back(GuiMessage(GuiMessage::action::rit_delta, rit_delta));
			}

			count = cat_message.GetBand();
			if (vfo.get_band_no(vfo.get_active_vfo()) != count && count != 0)
			{
				guiQueue.push_back(GuiMessage(GuiMessage::action::setband, count));
			}

			if (!(mode == mode_ft8 || mode == mode_ft4)) // TX
			{
				int rxtxCatMessage = cat_message.GetTX();
				if (m_mode != rxtxCatMessage)
				{
					m_mode = rxtxCatMessage;
					gui_mutex.lock();
					switch (m_mode)
					{
					case TX_OFF:
						select_mode(mode);
						break;
					case TX_MAN:
					case TX_CAT:
						select_mode_tx(mode, audioTone::NoTone, TX_CAT);
						break;
					case TX_TUNE_CAT:
						select_mode_tx(mode, audioTone::SingleTone, TX_TUNE_CAT);
						break;
					}
					gui_mutex.unlock();
				}
			}
		}
		else
		{
			usleep(10000);
		}
	}
}