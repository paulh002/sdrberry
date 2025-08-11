#include "CatTcpServer.h"
#include <iostream>
#include <string>
#include <cstring>
#include <ctype.h>
#include "Settings.h"
#include "SharedQueue.h"

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
	accepted = false;
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
		accepted = true;
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

	if (cattcpcomm.begin())
	{
		cat_message.begin(true, &cattcpcomm, true);
		return true;
	}
	return false;
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
		if (ret > 0)
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
		}
	}
}