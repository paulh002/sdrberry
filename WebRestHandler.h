#pragma once
#include "WebServer.h"
#include "SpectrumConstants.h"
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

class WebRestHandlerFilterFrq : public CivetHandler
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

class WebRestHandlerSpectrum : public CivetHandler
{
  public:
	WebRestHandlerSpectrum() : spectrum(nfft_samples,0) {}
	bool handleGet(CivetServer *server, struct mg_connection *conn);
	bool handlePost(CivetServer *server, struct mg_connection *conn);
	void NewData(const std::vector<int16_t>& newspectrum);

  private:
	std::map<std::string, nlohmann::json> identifier;
	std::mutex longpoll;
	std::condition_variable new_data;
	std::vector<int16_t> spectrum;
};

class WsStartHandler : public CivetHandler
{
  public:
	bool handleGet(CivetServer *server, struct mg_connection *conn);
	
};

class WebSocketHandler : public CivetWebSocketHandler
{
	virtual bool handleConnection(CivetServer *server, const struct mg_connection *conn) override;
	virtual void handleReadyState(CivetServer *server, struct mg_connection *conn) override;
	virtual bool handleData(CivetServer *server, struct mg_connection *conn, int bits, char *data, size_t data_len) override;
	virtual void handleClose(CivetServer *server, const struct mg_connection *conn) override;
};

extern WebRestHandlerSpectrum webspectrum;
extern WebRestHandlerVfo frequencyvfo1;