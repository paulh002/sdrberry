#include "WebServer.h"
#include "WebRestHandler.h"
#include "lvgl_.h"

#define DOCUMENT_ROOT "./sdrweb/"
#define PORT "8081"
#define EXAMPLE_URI "/example"
#define EXIT_URI "/exit"

WebSocketHandler websocketserver;

WebServer::WebServer()
{
	enabled = false;
}

WebServer::~WebServer()
{
	mg_exit_library();
}

void WebServer::StopServer()
{
	lv_obj_t *mbox1 = lv_msgbox_create(NULL, "Sdrberry", "WebServer will be switched off after reboot", NULL, true);
	lv_obj_center(mbox1);
}

void WebServer::StartServer()
{
	if (!isEnabled())
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

		Server = std::make_unique<CivetServer>(options);
		Server->addWebSocketHandler("/ws", websocketserver);
		enabled = true;
	}
}

void WebServer::AddHandler(const std::string &uri, CivetHandler& handler)
{
	Server->addHandler(uri, handler);
}

void WebServer::SendMessage(nlohmann::json message)
{
	if (enabled)
		websocketserver.SendMessage(message);
}

bool WebServer::isEnabled() 
{
	if (websocketserver.IsConnected() && enabled)
		return true;
	return false;
}
