#pragma once
#include "WebServer.h"
#include "SpectrumConstants.h"
#include <nlohmann/json.hpp>
#include <map>
#include <mutex>
#include <condition_variable>

enum websocketstate
{
	OFFLINE,
	READY,
	CONNECTING,
	CONNECTED,
	CLOSED
};

class WebSocketHandler : public CivetWebSocketHandler
{
  private:
	virtual bool handleConnection(CivetServer *server, const struct mg_connection *conn) override;
	virtual void handleReadyState(CivetServer *server, struct mg_connection *conn) override;
	virtual bool handleData(CivetServer *server, struct mg_connection *conn, int bits, char *data, size_t data_len) override;
	virtual void handleClose(CivetServer *server, const struct mg_connection *conn) override;
  
	struct mg_connection *connection;
	enum websocketstate state;
  
public:
	WebSocketHandler();
	void SendMessage(nlohmann::json message);
};
