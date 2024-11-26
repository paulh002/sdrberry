#include "WebRestHandler.h"
#include "gui_ft8.h"
#include "gui_top_bar.h"
#include "vfo.h"
#include "SharedQueue.h"
#include "gui_ft8bar.h"

using json = nlohmann::json;

bool WebRestHandler::handleGet(CivetServer *server, struct mg_connection *conn)
{
	/* Handler may access the request info using mg_get_request_info */
	const struct mg_request_info *req_info = mg_get_request_info(conn);

	//json message_array = json::array();

	json message_array = gft8.get_messages(0, 0);
	mg_send_http_ok(conn, "application/json; charset=utf-8", message_array.dump().length());
	mg_printf(conn, "%s", message_array.dump().c_str());
	return true;
}

bool WebRestHandler::handlePost(CivetServer *server, struct mg_connection *conn)
{
	/* Handler may access the request info using mg_get_request_info */
	const struct mg_request_info *req_info = mg_get_request_info(conn);
	long long rlen, wlen;
	long long nlen = 0;
	long long tlen = req_info->content_length;
	char buf[1024];

	json jsonresult;
	json message;
	json message_array = json::array();

	mg_printf(conn,
			  "HTTP/1.1 200 OK\r\nContent-Type: "
			  "text/html\r\nConnection: close\r\n\r\n");

	std::vector<std::string> result = gft8.get_messages(0, 0);

	int i = 0;
	for (auto col : result)
	{	
		switch (i)
		{
		case 0:
			message.emplace("time", col);
			break;
		case 1:
			message.emplace("decibel", col);
			break;
		case 2:
			message.emplace("frequency", col);
			break;
		case 3:
			message.emplace("message", col);
			break;
		}
		i++;
		if (i == 4)
		{
			i = 0;
			//std::string s = message.dump() + "\n";
			message_array.push_back(message);
			message.clear();
		}
	}
	jsonresult["message"] = message_array;
	mg_printf(conn, "%s", jsonresult.dump().c_str());

	while (nlen < tlen)
	{
		rlen = tlen - nlen;
		if (rlen > sizeof(buf))
		{
			rlen = sizeof(buf);
		}
		rlen = mg_read(conn, buf, (size_t)rlen);
		if (rlen <= 0)
		{
			break;
		}
		wlen = mg_write(conn, buf, (size_t)rlen);
		if (wlen != rlen)
		{
			break;
		}
		nlen += wlen;
	}

	return true;
}

bool WebRestHandlerVfo::handleGet(CivetServer *server, struct mg_connection *conn)
{
	/* Handler may access the request info using mg_get_request_info */
	const struct mg_request_info *req_info = mg_get_request_info(conn);
	json message;

	std::string freq = vfo.get_vfo_str();
	std::string mode = vfo.getMode(vfo.get_active_vfo());
	std::string band = vfo.get_band_in_text();
	std::string call = Settings_file.get_string("wsjtx", "call");
	
	message.emplace("frequency", freq);
	message.emplace("mode", mode);
	message.emplace("band", band);
	message.emplace("call", call);
	message.emplace("label", GuiTopBar.getLabel());

	mg_send_http_ok(conn, "application/json; charset=utf-8", message.dump().length());
	mg_printf(conn, "%s", message.dump().c_str());
	return true;
}

bool WebRestHandlerVfo::handlePost(CivetServer *server, struct mg_connection *conn)
{
	/* Handler may access the request info using mg_get_request_info */
	const struct mg_request_info *req_info = mg_get_request_info(conn);
	long long rlen, wlen;
	long long nlen = 0;
	long long tlen = req_info->content_length;
	char buf[1024];

	std::memset(buf, 0, sizeof(buf));
	int dlen = mg_read(conn, buf, sizeof(buf) - 1);
	if ((dlen < 1) || (dlen >= sizeof(buf)))
	{
		mg_send_http_error(conn, 400, "%s", "No request body data");
		return false;
	}
	json argument;
	try
	{
		argument = json::parse(buf);
	}
	catch (const exception &e)
	{
		std::string err = e.what();
		mg_send_http_error(conn, 400, "%s", err.c_str());
		return true;
	}

	try
	{
		argument.at("identifier");
	}
	catch (const exception &e)
	{
		std::string err = e.what();
		mg_send_http_error(conn, 400, "%s", err.c_str());
		return true;
	}

	json message;

	std::string freq = vfo.get_vfo_str();
	std::string mode = vfo.getMode(vfo.get_active_vfo());
	std::string band = vfo.get_band_in_text();
	std::string call = Settings_file.get_string("wsjtx", "call");

	message.emplace("frequency", freq);
	message.emplace("mode", mode);
	message.emplace("band", band);
	message.emplace("call", call);
	message.emplace("label", GuiTopBar.getLabel());

	if (auto search = identifier.find(argument.at("identifier")); search == identifier.end())
	{
		identifier[argument.at("identifier")] = message;
		mg_send_http_ok(conn, "application/json; charset=utf-8", message.dump().length());
		mg_printf(conn, "%s", message.dump().c_str());
		printf("New identifier %s\n", argument.at("identifier").dump().c_str());
		return true;
	}
	std::unique_lock<std::mutex> lock(longpoll);
	bool wait = true;
	while (wait)
	{
		printf("Existing identifier %s\n", argument.at("identifier").dump().c_str());
		new_data.wait_for(lock, std::chrono::seconds(10)); // conditional wait unlocks the mutex!
		
		std::string freq = vfo.get_vfo_str();
		std::string mode = vfo.getMode(vfo.get_active_vfo());
		std::string band = vfo.get_band_in_text();
		std::string call = Settings_file.get_string("wsjtx", "call");

		message.clear();
		message.emplace("frequency", freq);
		message.emplace("mode", mode);
		message.emplace("band", band);
		message.emplace("call", call);
		message.emplace("label", GuiTopBar.getLabel());

		//if (auto search = identifier.find(argument.at("identifier")); search == identifier.end())
			wait = false;
	}
	mg_send_http_ok(conn, "application/json; charset=utf-8", message.dump().length());
	mg_printf(conn, "%s", message.dump().c_str());
	return true;
}

void WebRestHandlerVfo::NewData()
{
	new_data.notify_all();
}
	
bool WebRestHandlerSelectMessage::handlePost(CivetServer *server, struct mg_connection *conn)
{
	/* Handler may access the request info using mg_get_request_info */
	const struct mg_request_info *req_info = mg_get_request_info(conn);
	long long rlen, wlen;
	long long nlen = 0;
	long long tlen = req_info->content_length;
	char buf[1024];

	std::memset(buf, 0, sizeof(buf));
	int dlen = mg_read(conn, buf, sizeof(buf) - 1);
	if ((dlen < 1) || (dlen >= sizeof(buf)))
	{
		mg_send_http_error(conn, 400, "%s", "No request body data");
		return false;
	}
	json message;
	try
	{
		message = json::parse(buf);
	}
	catch (const exception &e )
	{
		std::string err = e.what();
		mg_send_http_error(conn, 400, "%s", err.c_str());
		return true;
	}

	try
	{
		message.at("time");
		message.at("decibel");
		message.at("frequency");
		message.at("message");
	}
	catch (const exception &e)
	{
			std::string err = e.what();
			mg_send_http_error(conn, 400, "%s", err.c_str());
			return true;
	}

	//printf("select message\n");
	guiQueue.push_back(GuiMessage(GuiMessage::action::selectMessage, message.dump()));
	
	mg_printf(conn,
			  "HTTP/1.1 201 OK\r\nContent-Type: "
			  "application/json\r\nConnection: close\r\n\r\n");
	
	return true;
}

bool WebRestHandlerQso::handleGet(CivetServer *server, struct mg_connection *conn)
{
	/* Handler may access the request info using mg_get_request_info */
	const struct mg_request_info *req_info = mg_get_request_info(conn);

	json message_array = json::array();

	message_array = gft8.get_qso(0, 0);
	mg_send_http_ok(conn, "application/json; charset=utf-8", message_array.dump().length());
	mg_printf(conn, "%s", message_array.dump().c_str());
	return true;
}

bool WebRestHandlerCq::handleGet(CivetServer *server, struct mg_connection *conn)
{
	/* Handler may access the request info using mg_get_request_info */
	const struct mg_request_info *req_info = mg_get_request_info(conn);

	json message_array = json::array();

	message_array = gft8.get_cq(0, 0);
	mg_send_http_ok(conn, "application/json; charset=utf-8", message_array.dump().length());
	mg_printf(conn, "%s", message_array.dump().c_str());
	return true;
}

bool WebRestHandlerWsjtxFrq::handleGet(CivetServer *server, struct mg_connection *conn)
{
	/* Handler may access the request info using mg_get_request_info */
	const struct mg_request_info *req_info = mg_get_request_info(conn);

	json message_array = json::array();

	message_array = guift8bar.get_wsjtxfreq(0, 0);
	mg_send_http_ok(conn, "application/json; charset=utf-8", message_array.dump().length());
	mg_printf(conn, "%s", message_array.dump().c_str());
	return true;
}

bool WebRestHandlerButtonMessage::handleGet(CivetServer *server, struct mg_connection *conn)
{
	/* Handler may access the request info using mg_get_request_info */
	const struct mg_request_info *req_info = mg_get_request_info(conn);

	json message_array = json::array();

	message_array = guift8bar.get_buttons();
	mg_send_http_ok(conn, "application/json; charset=utf-8", message_array.dump().length());
	mg_printf(conn, "%s", message_array.dump().c_str());
	return true;
}

bool WebRestHandlerButtonMessage::handlePost(CivetServer *server, struct mg_connection *conn)
{
	/* Handler may access the request info using mg_get_request_info */
	const struct mg_request_info *req_info = mg_get_request_info(conn);
	long long rlen, wlen;
	long long nlen = 0;
	long long tlen = req_info->content_length;
	char buf[1024];

	std::memset(buf, 0, sizeof(buf));
	int dlen = mg_read(conn, buf, sizeof(buf) - 1);
	if ((dlen < 1) || (dlen >= sizeof(buf)))
	{
			mg_send_http_error(conn, 400, "%s", "No request body data");
			return false;
	}
	json message;
	try
	{
			message = json::parse(buf);
	}
	catch (const exception &e)
	{
			std::string err = e.what();
			mg_send_http_error(conn, 400, "%s", err.c_str());
			return true;
	}

	try
	{
			message.at("button");
			message.at("type");
	}
	catch (const exception &e)
	{
			std::string err = e.what();
			mg_send_http_error(conn, 400, "%s", err.c_str());
			return true;
	}
	guiQueue.push_back(GuiMessage(GuiMessage::action::buttonMessage, message.dump()));

	mg_printf(conn,
			  "HTTP/1.1 201 OK\r\nContent-Type: "
			  "application/json\r\nConnection: close\r\n\r\n");

	return true;
}

bool WebRestHandlerTxMessage::handleGet(CivetServer *server, struct mg_connection *conn)
{
	/* Handler may access the request info using mg_get_request_info */
	const struct mg_request_info *req_info = mg_get_request_info(conn);

	json message_array = json::array();

	message_array = guift8bar.get_txmessage();
	mg_send_http_ok(conn, "application/json; charset=utf-8", message_array.dump().length());
	mg_printf(conn, "%s", message_array.dump().c_str());
	return true;
}

bool WebRestHandlerTxMessage::handlePost(CivetServer *server, struct mg_connection *conn)
{
	/* Handler may access the request info using mg_get_request_info */
	const struct mg_request_info *req_info = mg_get_request_info(conn);
	long long rlen, wlen;
	long long nlen = 0;
	long long tlen = req_info->content_length;
	char buf[1024];

	std::memset(buf, 0, sizeof(buf));
	int dlen = mg_read(conn, buf, sizeof(buf) - 1);
	if ((dlen < 1) || (dlen >= sizeof(buf)))
	{
			mg_send_http_error(conn, 400, "%s", "No request body data");
			return false;
	}
	json message;
	try
	{
			message = json::parse(buf);
	}
	catch (const exception &e)
	{
			std::string err = e.what();
			mg_send_http_error(conn, 400, "%s", err.c_str());
			return true;
	}

	try
	{
			message.at("no");
	}
	catch (const exception &e)
	{
			std::string err = e.what();
			mg_send_http_error(conn, 400, "%s", err.c_str());
			return true;
	}
	printf("TxMessage: %s \n", message.dump().c_str());
	guiQueue.push_back(GuiMessage(GuiMessage::action::TxMessage, message.dump()));

	mg_printf(conn,
			  "HTTP/1.1 201 OK\r\nContent-Type: "
			  "application/json\r\nConnection: close\r\n\r\n");

	return true;
}