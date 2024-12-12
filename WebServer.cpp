#include "WebServer.h"
#include "WebRestHandler.h"

#define DOCUMENT_ROOT "./sdrweb/"
#define PORT "8081"
#define EXAMPLE_URI "/example"
#define EXIT_URI "/exit"

WebRestHandler wsjtx_messages;
WebRestHandlerVfo frequencyvfo1;
WebRestHandlerSelectMessage SelectMessage;
WebRestHandlerQso qso_messages;
WebRestHandlerCq cq_messages;
WebRestHandlerWsjtxFrq wsjtx_frequencies;
WebRestHandlerButtonMessage buttonMessage;
WebRestHandlerTxMessage txmessage;
WebRestHandlerFilterFrq filter_frequencies;
WebRestHandlerSpectrum webspectrum;
WebRestHandlerSpectrumLedgend webspectrumledgend;
WebRestHandlerSpectrumSliders webspectrumsliders;
WebRestHandlerSpectrumSlidersButtons webspectrumslidersbuttons;
WebSocketHandler websocketserver;
WsStartHandler wsstarthandler;

WebServer::WebServer()
{
	mg_init_library(0);
	options.push_back("document_root");
	options.push_back(DOCUMENT_ROOT);
	options.push_back("listening_ports");
	options.push_back(PORT);
	options.push_back("enable_keep_alive");
	options.push_back("yes");
	options.push_back("keep_alive_timeout_ms");
	options.push_back("500");
}

WebServer::~WebServer()
{
	mg_exit_library();
}

void WebServer::StartServer()
{
	Server = std::make_unique<CivetServer>(options);
	AddHandler("/api/wsjtxmessages", wsjtx_messages);
	AddHandler("/api/frequencyvfo1", frequencyvfo1);
	AddHandler("/api/selectmessage", SelectMessage);
	AddHandler("/api/qsomessages", qso_messages);
	AddHandler("/api/cqmessages", cq_messages);
	AddHandler("/api/wsjtxfrequencies", wsjtx_frequencies);
	AddHandler("/api/buttonmessage", buttonMessage);
	AddHandler("/api/txmessage", txmessage);
	AddHandler("/api/filterfrequencies", filter_frequencies);
	AddHandler("/api/spectrum", webspectrum);
	AddHandler("/api/spectrumledgend", webspectrumledgend);
	AddHandler("/api/spectrumsliders", webspectrumsliders);
	AddHandler("/api/spectrumslidersbuttons", webspectrumslidersbuttons);
	//AddHandler("/ws", wsstarthandler);
	//Server->addWebSocketHandler("/websocket", websocketserver);
}

void WebServer::AddHandler(const std::string &uri, CivetHandler& handler)
{
	Server->addHandler(uri, handler);
}