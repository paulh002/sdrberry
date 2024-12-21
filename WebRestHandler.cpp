#include "WebRestHandler.h"
#include "SharedQueue.h"
#include "gui_bar.h"
#include "gui_ft8.h"
#include "gui_ft8bar.h"
#include "gui_top_bar.h"
#include "vfo.h"

using json = nlohmann::json;

WebSocketHandler::WebSocketHandler()
{
	state = OFFLINE;
	connection = NULL;
}

bool WebSocketHandler::handleConnection(CivetServer *server, const struct mg_connection *conn)
{
	printf("WS connected\n");
	state = CONNECTING;
	return true;
}

void WebSocketHandler::handleReadyState(CivetServer *server, struct mg_connection *conn)
{
	json message;
	
	printf("WS ready\n");
	state = CONNECTED;
	connection = conn;

	message.emplace("type", "connected");
	mg_websocket_write(conn, MG_WEBSOCKET_OPCODE_TEXT, message.dump().c_str(), message.dump().length());
}

bool WebSocketHandler::handleData(CivetServer *server, struct mg_connection *conn, int bits, char *data, size_t data_len)
{
	json message;
	std::string s;
	
	s.resize(data_len + 1);
	memset(s.data(), 0, data_len + 1);
	strncpy(s.data(), data, data_len);
	
	//printf("WS got %lu bytes: ", (long unsigned)data_len);
	//fwrite(data, 1, data_len, stdout);
	//printf("\n");

	try
	{
		message = json::parse(s);
	}
	catch (const exception &e)
	{
		std::string err = e.what();
		printf("%s\n", err.c_str());
		return true;
	}
	if (message.find("start") != message.end())
	{
		vfo.updateweb();
		gbar.updateweb();
		gbar.web_filterfreq();
		guift8bar.get_buttons();
		guift8bar.web_wsjtxfreq();
		guift8bar.web_call(guift8bar.GetFilter());
		guift8bar.web_txmessage();
		gft8.web_cq();
		gft8.web_qso();
		return true;
	}
	//printf("%s\n", message.dump().c_str());
	if (message.find("type") != message.end())
	{
		//printf("%s\n", message.dump().c_str());

		if (message.at("type") == "wsjtxbar" || message.at("type") == "selecttxmessage" || message.at("type") == "selectmessage")
		{
			guiQueue.push_back(GuiMessage(GuiMessage::action::wsjtxMessage, message.dump()));
			return true;
		}

		if (message.at("type") == "vfo")
		{
			//printf("%s\n", message.dump().c_str());
			int step = message.at("step");
			guiQueue.push_back(GuiMessage(GuiMessage::action::step, step));
			return true;
		}
	}

	guiQueue.push_back(GuiMessage(GuiMessage::action::TranceiverMessage, message.dump()));
	return true;
}

void WebSocketHandler::handleClose(CivetServer *server, const struct mg_connection *conn)
{
	state = CLOSED;
	connection = NULL;
	printf("WS closed\n");
}

void WebSocketHandler::SendMessage(json message)
{
	if (state == CONNECTED)
	{
		//printf("WS send %lu bytes: ", (long unsigned)message.dump().length());
		mg_websocket_write(connection, MG_WEBSOCKET_OPCODE_TEXT, message.dump().c_str(), message.dump().length());
	}
}