#pragma once
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "FT891_CAT.h"
#include <atomic>

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
	void SetNA(int ft);
	void SetMDA(int md);
	void SetMDB(int mode);
	void Pause_Cat(bool pause) { bpause_cat = pause; }
	void operator()();
	FT891_CAT *operator->() { return &cat_message; }
	void signal_stop() { stop_flag = true; }

  private:
	CatTcpComm cattcpcomm;
	FT891_CAT cat_message;
	uint32_t vfo_a, vfo_b;
	int filter;
	int m_mode{0};
	int mda, mdb;
	int rit_onoff, rit_delta;
	std::atomic<bool> bpause_cat, stop_flag;
};

extern CatTcpServer cattcpserver;