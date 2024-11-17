#include "WebServer.h"
#include "WebRestHandler.h"

#define DOCUMENT_ROOT "./sdrweb/"
#define PORT "8081"
#define EXAMPLE_URI "/example"
#define EXIT_URI "/exit"

WebRestHandler wsjtx_messages;
WebRestHandlerVfo frequencyvfo1;
WebRestHandlerSelectMessage SelectMessage;

WebServer::WebServer()
{
	mg_init_library(0);
	options.push_back("document_root");
	options.push_back(DOCUMENT_ROOT);
	options.push_back("listening_ports");
	options.push_back(PORT);
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
	
}

void WebServer::AddHandler(const std::string &uri, CivetHandler& handler)
{
	Server->addHandler(uri, handler);
}