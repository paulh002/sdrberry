#pragma once
#include<string>
#include<memory>
#include "CivetServer.h"

class WebServer
{
  public:
	WebServer();
	~WebServer();
	void StartServer();
	void AddHandler(const std::string &uri, CivetHandler& handler);

  private:
	//const char *options[] = {"document_root", DOCUMENT_ROOT, "listening_ports", PORT, 0};
	std::vector<std::string> options;
	std::unique_ptr<CivetServer> Server;
};

