#pragma once
#include "WebServer.h"
#include <nlohmann/json.hpp>
#include <map>
#include <mutex>
#include <condition_variable>

class WebRestHandler : public CivetHandler
{
  public:
	bool handlePost(CivetServer *server, struct mg_connection *conn);
	bool handleGet(CivetServer *server, struct mg_connection *conn);
  private:
	
};

class WebRestHandlerVfo : public CivetHandler
{
  public:
	bool handleGet(CivetServer *server, struct mg_connection *conn);
	bool handlePost(CivetServer *server, struct mg_connection *conn);
	void NewData();

  private:
	std::map<std::string, nlohmann::json> identifier;
	std::mutex longpoll;
	std::condition_variable new_data;
};

class WebRestHandlerSelectMessage : public CivetHandler
{
  public:
	bool handlePost(CivetServer *server, struct mg_connection *conn);

  private:
};

class WebRestHandlerQso : public CivetHandler
{
  public:
	bool handleGet(CivetServer *server, struct mg_connection *conn);

  private:
};

class WebRestHandlerCq : public CivetHandler
{
  public:
	bool handleGet(CivetServer *server, struct mg_connection *conn);

  private:
};

class WebRestHandlerWsjtxFrq : public CivetHandler
{
  public:
	bool handleGet(CivetServer *server, struct mg_connection *conn);

  private:
};

class WebRestHandlerButtonMessage : public CivetHandler
{
  public:
	bool handleGet(CivetServer *server, struct mg_connection *conn);
	bool handlePost(CivetServer *server, struct mg_connection *conn);

  private:
};

class WebRestHandlerTxMessage : public CivetHandler
{
  public:
	bool handleGet(CivetServer *server, struct mg_connection *conn);
	bool handlePost(CivetServer *server, struct mg_connection *conn);

  private:
};