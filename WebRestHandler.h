#pragma once
#include "WebServer.h"

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

  private:
};

class WebRestHandlerSelectMessage : public CivetHandler
{
  public:
	bool handlePost(CivetServer *server, struct mg_connection *conn);

  private:
};