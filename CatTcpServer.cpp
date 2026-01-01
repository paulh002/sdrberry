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

const int BUFFER_SIZE = 80;

extern SharedQueue<GuiMessage> guiQueue;


CatTcpComm::~CatTcpComm()
{

}

void CatTcpComm::Close()
{

}

bool CatTcpComm::begin() 
{
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
	if (connected)
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
	filter = 0;
	if (cattcpcomm.begin())
	{
		cat_message.begin(true, &cattcpcomm, true);
		return true;
	}
	return false;
}

void CatTcpServer::SetNA(int ft)
{
	filter = ft;
	cat_message.SetNA(filter);
	printf("set filter %d\n", filter);
}

void CatTcpServer::SetMDA(int mode)
{
	mda = encode_mode(mode);
	cat_message.SetMDA(mda);
	printf("set MDA %d\n", mda);
}

void CatTcpServer::SetMDB(int mode)
{
	mdb = encode_mode(mode);
	cat_message.SetMDB(mdb);
	printf("set MDB %d\n", mdb);
}

void CatTcpServer::StopServer()
{
}

void CatTcpServer::operator()()
{
	int count;
	while (1)
	{
		int ret = cat_message.CheckCAT(false);
		if (ret < 0)
		{
			printf("Error reading tcp socket \n");
		}
		else if (ret > 0)
		{
			count = cat_message.GetFA();
			if (count && vfo_a != count)
			{
				char str[80];

				vfo_a = count;
				sprintf(str, "%d", count);
				guiQueue.push_back(GuiMessage(GuiMessage::action::setvfo, str));
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
			if (!(mode == mode_ft8 || mode == mode_ft4)) // TX
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
					case TX_MAN:
					case TX_CAT:
						select_mode_tx(mode, audioTone::NoTone, TX_CAT);
						break;
					case TX_TUNE_CAT:
						select_mode_tx(mode, audioTone::SingleTone, TX_TUNE_CAT);
						break;
					}
				}
			}
		}
	}
}