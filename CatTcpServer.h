#pragma once
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "FT891_CAT.h"

class CatTcpComm : public Cat_communicator
{
  public:
	bool begin();
	void Send(std::string s) override;
	int Read(char c, std::string &s) override;
	bool available() override;
	void SendInformation(int info) override;
	bool IsCommuncationPortOpen() override;

	~CatTcpComm() override;
	void Close();

  private:
	int server_fd, tcp_socket;
	int port;
	struct sockaddr_in address;
	bool connected;
};

class CatTcpServer
{
  public:
	bool StartServer();
	void StopServer();

	void operator()();
	FT891_CAT *operator->() { return &cat_message; }
	
  private:
	CatTcpComm cattcpcomm;
	FT891_CAT cat_message;
	uint32_t vfo_a;
};

