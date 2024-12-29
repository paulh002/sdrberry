#pragma once
#include<string>
#include<memory>
#include "CivetServer.h"
#include <nlohmann/json.hpp>

class WebServer
{
  public:
	WebServer();
	~WebServer();
	void StartServer();
	void AddHandler(const std::string &uri, CivetHandler& handler);
	bool isEnabled();
	void SendMessage(nlohmann::json message);
	void StopServer();

  private:
	// const char *options[] = {"document_root", DOCUMENT_ROOT, "listening_ports", PORT, "enable_keep_alive","yes",0};
	std::vector<std::string> options;
	std::unique_ptr<CivetServer> Server;
	bool enabled{false};
};

extern WebServer webserver;
