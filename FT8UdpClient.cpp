#include "FT8UdpClient.h"
#include "Modes.h"
#include "Settings.h"
#include "date.h"
#include "gui_ft8.h"
#include "julian.h"
#include <bits/stdc++.h>
#include <iostream>
#include <memory>

// Helper function to append data to a vector of chars
template <typename T>
void append_to_buffer(std::vector<char> &buffer, const T &value)
{
	const char *value_ptr = reinterpret_cast<const char *>(&value);
	buffer.insert(buffer.end(), value_ptr, value_ptr + sizeof(T));
}

// Helper function to append a string to the buffer (length-prefixed)
void append_string_to_buffer(std::vector<char> &buffer, const std::string &str)
{
	uint32_t len = htonl(str.length());
	append_to_buffer<uint32_t>(buffer, len);
	buffer.insert(buffer.end(), str.begin(), str.end());
}

FT8UdpClient::FT8UdpClient(int wsjtx_mode)
{
	mode = wsjtx_mode;
	sockfd = -1;
	std::string server_ip = Settings_file.get_string("wsjtx", "url");
	// Creating socket file descriptor
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		sockfd = 0;
		perror("socket creation failed");
		return;
	}

	std::memset(&servaddr, 0, sizeof(servaddr));

	// Filling server information
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	servaddr.sin_addr.s_addr = inet_addr(server_ip.c_str());
}

FT8UdpClient::~FT8UdpClient()
{
	::close(sockfd);
}

std::vector<char> FT8UdpClient::serialize_heartbeat_message(const HeartbeatMessage &msg)
{
	std::vector<char> buffer;
	buffer.reserve(BUFFER_SIZE);

	uint32_t magic = htonl(msg.magic);
	append_to_buffer<uint32_t>(buffer, magic);

	uint32_t schema = htonl(msg.Schema);
	append_to_buffer<uint32_t>(buffer, schema);

	uint32_t msg_type = htonl(msg.messageType);
	append_to_buffer<uint32_t>(buffer, msg_type);

	append_string_to_buffer(buffer, msg.version);

	uint32_t max_schema = htonl(msg.MaxSchema);
	append_to_buffer<uint32_t>(buffer, max_schema);

	append_string_to_buffer(buffer, msg.revision);

	return buffer;
}

// Function to serialize the StatusMessage
std::vector<char> serialize_status_message(const StatusMessage &msg)
{
	std::vector<char> buffer;
	buffer.reserve(BUFFER_SIZE);

	uint32_t magic = htonl(msg.magic);
	append_to_buffer<uint32_t>(buffer, magic);

	uint32_t schema = htonl(msg.Schema);
	append_to_buffer<uint32_t>(buffer, schema);

	uint32_t msg_type = htonl(msg.messageType);
	append_to_buffer<uint32_t>(buffer, msg_type);

	append_string_to_buffer(buffer, msg.id);

	uint64_t dial_freq = htobe64(msg.dialFrequency);
	append_to_buffer<uint64_t>(buffer, dial_freq);

	append_string_to_buffer(buffer, msg.mode);
	append_string_to_buffer(buffer, msg.dxCall);
	append_string_to_buffer(buffer, msg.report);
	append_string_to_buffer(buffer, msg.txMode);

	append_to_buffer<bool>(buffer, msg.txEnabled);
	append_to_buffer<bool>(buffer, msg.transmitting);
	append_to_buffer<bool>(buffer, msg.decoding);

	uint32_t rx_df = htonl(msg.rxDF);
	append_to_buffer<uint32_t>(buffer, rx_df);

	uint32_t tx_df = htonl(msg.txDF);
	append_to_buffer<uint32_t>(buffer, tx_df);

	append_string_to_buffer(buffer, msg.deCall);
	append_string_to_buffer(buffer, msg.deGrid);
	append_string_to_buffer(buffer, msg.dxGrid);

	append_to_buffer<bool>(buffer, msg.txWatchdog);
	append_string_to_buffer(buffer, msg.subMode);
	append_to_buffer<bool>(buffer, msg.fastMode);

	append_to_buffer(buffer, msg.specialOperationMode);

	uint32_t freq_tol = htonl(msg.frequencyTolerance);
	append_to_buffer<uint32_t>(buffer, freq_tol);

	uint32_t tr_period = htonl(msg.trPeriod);
	append_to_buffer<uint32_t>(buffer, tr_period);

	append_string_to_buffer(buffer, msg.configurationName);
	append_string_to_buffer(buffer, msg.txMessage);

	return buffer;
}

// Function to serialize the DecodeMessage
std::vector<char> FT8UdpClient::serialize_decode_message(const DecodeMessage &msg)
{
	std::vector<char> buffer;
	buffer.reserve(BUFFER_SIZE);

	uint32_t magic = htonl(msg.magic);
	append_to_buffer<uint32_t>(buffer, magic);

	uint32_t schema = htonl(msg.Schema);
	append_to_buffer<uint32_t>(buffer, schema);

	uint32_t msg_type = htonl(msg.messageType);
	append_to_buffer<uint32_t>(buffer, msg_type);

	append_string_to_buffer(buffer, msg.id);

	append_to_buffer(buffer, msg.isNew);

	uint32_t time_val = htonl(msg.time);
	append_to_buffer<uint32_t>(buffer, time_val);

	int32_t snr_val = htonl(msg.snr);
	append_to_buffer<uint32_t>(buffer, snr_val);

	// Serialize double as 8 bytes
	uint64_t deltaTime_val;
	std::memcpy(&deltaTime_val, &msg.deltaTime, sizeof(double));
	append_to_buffer<uint64_t>(buffer, deltaTime_val);

	uint32_t deltaFrequency_val = htonl(msg.deltaFrequency);
	append_to_buffer<uint32_t>(buffer, deltaFrequency_val);

	append_string_to_buffer(buffer, msg.mode);
	append_string_to_buffer(buffer, msg.message);

	append_to_buffer<bool>(buffer, msg.lowConfidence);
	append_to_buffer<bool>(buffer, msg.offAir);

	return buffer;
}

std::vector<char> FT8UdpClient::serialize_qso_message(const QsoLoggedMessage &msg)
{
	std::vector<char> buffer;
	buffer.reserve(BUFFER_SIZE); // Pre-allocate some memory

	uint32_t magic = htonl(msg.magic);
	append_to_buffer<uint32_t>(buffer, magic);

	uint32_t schema = htonl(msg.Schema);
	append_to_buffer<uint32_t>(buffer, schema);

	// Message Type
	uint32_t msg_type = htonl(msg.messageType);
	append_to_buffer<uint32_t>(buffer, msg_type);

	// ID
	append_string_to_buffer(buffer, msg.id);

	// Date
	uint64_t dt_off = htobe64(msg.dateOff);
	append_to_buffer<uint64_t>(buffer, dt_off);

	// Time
	uint32_t time_off = htonl(msg.timeOff);
	append_to_buffer<uint32_t>(buffer, time_off);
	append_to_buffer<uint8_t>(buffer, msg.timespecOff);

	// DX Call
	append_string_to_buffer(buffer, msg.dxCall);

	// DX Grid
	append_string_to_buffer(buffer, msg.dxGrid);

	// Tx Frequency
	uint64_t freq = htobe64(msg.txFrequency);
	append_to_buffer(buffer, freq);

	// Mode
	append_string_to_buffer(buffer, msg.mode);

	// Report Sent
	append_string_to_buffer(buffer, msg.reportSent);

	// Report Received
	append_string_to_buffer(buffer, msg.reportReceived);

	// Tx Power
	append_string_to_buffer(buffer, msg.txPower);

	// Comments
	append_string_to_buffer(buffer, msg.comments);

	// Name
	append_string_to_buffer(buffer, msg.name);

	// DateTime On
	uint64_t dt_on = htobe64(msg.dateOn);
	append_to_buffer<uint64_t>(buffer, dt_on);
	// Time
	uint32_t time_on = htonl(msg.timeOn);
	append_to_buffer<uint32_t>(buffer, time_on);
	append_to_buffer<uint8_t>(buffer, msg.timespecOn);

	// Operator Call
	append_string_to_buffer(buffer, msg.operatorCall);

	// My Call
	append_string_to_buffer(buffer, msg.myCall);

	// My Grid
	append_string_to_buffer(buffer, msg.myGrid);

	// Exchange Sent
	append_string_to_buffer(buffer, msg.exchangeSent);

	// Exchange Received
	append_string_to_buffer(buffer, msg.exchangeReceived);

	// ADIF Propagation Mode
	append_string_to_buffer(buffer, msg.adifPropagationMode);

	return buffer;
}

void FT8UdpClient::SendHeartBeat()
{
	if (sockfd > 0)
	{
		HeartbeatMessage heartbeat_msg;

		heartbeat_msg.magic = MAGIC_NUMBER;
		heartbeat_msg.Schema = 2;
		heartbeat_msg.messageType = 0;
		heartbeat_msg.MaxSchema = 2;
		heartbeat_msg.version = "WSJT-X";
		heartbeat_msg.revision = "2.7.0";

		std::vector<char> packet = serialize_heartbeat_message(heartbeat_msg);

		if (sendto(sockfd, packet.data(), packet.size(),
				   MSG_CONFIRM, (const struct sockaddr *)&servaddr,
				   sizeof(servaddr)) < 0)
		{
			perror("sendto failed");
			close(sockfd);
			sockfd = 0;
			return;
		}
		else
		{
			// std::cout << "Heartbeat message sent." << std::endl;
		}
	}
}

void FT8UdpClient::SendDecode(bool isNew, uint32_t now_ms, int32_t snr, double deltaTime, uint32_t deltaFrequency, int mode, std::string message, bool lowConfidence, bool offAir)
{
	if (sockfd > 0)
	{
		DecodeMessage decode_msg;
		decode_msg.magic = MAGIC_NUMBER;
		decode_msg.Schema = 2;
		decode_msg.messageType = 2;

		decode_msg.id = "WSJT-X";
		decode_msg.isNew = true;

		// Get current time for timestamp
		// using namespace std::chrono;
		// uint32_t now_ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
		decode_msg.time = now_ms;

		decode_msg.snr = snr;
		decode_msg.deltaTime = deltaTime;
		decode_msg.deltaFrequency = deltaFrequency;
		if (mode == mode_ft8)
			decode_msg.mode = "FT8";
		if (mode == mode_ft4)
			decode_msg.mode = "FT4";
		if (mode == mode_wspr)
			decode_msg.mode = "WSPR";
		decode_msg.message = message;
		decode_msg.lowConfidence = lowConfidence;
		decode_msg.offAir = offAir;

		std::vector<char> packet = serialize_decode_message(decode_msg);

		if (sendto(sockfd, packet.data(), packet.size(),
				   MSG_CONFIRM, (const struct sockaddr *)&servaddr,
				   sizeof(servaddr)) < 0)
		{
			perror("sendto failed");
			close(sockfd);
			sockfd = 0;
			return;
		}
		// std::cout << "Decode message sent." << std::endl;
	}
}

void FT8UdpClient::SendQso(std::chrono::time_point<std::chrono::system_clock> qso_time, std::chrono::time_point<std::chrono::system_clock> qso_time_off, std::string dxCall, std::string dxGrid, uint64_t txFrequency, int mode, std::string reportSent,
						   std::string reportReceived, std::string txPower, std::string comments, std::string name, std::string operatorCall, std::string myCall,
						   std::string myGrid, std::string exchangeSent, std::string exchangeReceived, std::string adifPropagationMode)
{
	if (sockfd > 0)
	{
		QsoLoggedMessage qso_msg;
		qso_msg.magic = MAGIC_NUMBER;
		qso_msg.Schema = 2;
		qso_msg.messageType = 5;
		qso_msg.id = "WSJT-X";

		// Get current time for timestamps
		// Get current time for timestamps
		// const auto now = std::chrono::system_clock::now();
		// const auto today = date::floor<date::days>(now);

		const auto qso_day = date::floor<date::days>(qso_time);
		const julian::year_month_day j_today{qso_day};
		const date::year_month_day g_today{qso_day};

		std::cout << "The current Gregorian calendar date is: "
				  << g_today.year() << '-'
				  << static_cast<unsigned>(g_today.month()) << '-'
				  << static_cast<unsigned>(g_today.day()) << std::endl;

		std::cout << "The current Julian calendar date is: "
				  << j_today.year() << '-'
				  << static_cast<unsigned>(j_today.month()) << '-'
				  << static_cast<unsigned>(j_today.day()) << std::endl;

		// The Julian Day Number for a given sys_days is the count of days
		// since the date library's epoch (1970-01-01) plus the JDN of that epoch.
		// The JDN of 1970-01-01 is 2440587.5.
		const auto jdn = qso_day.time_since_epoch().count() + 2440588; // 2440587.5
		std::cout << "The Julian Day Number for today is: " << std::fixed << jdn << std::endl;
		qso_msg.dateOn = jdn;

		const auto qso_day_off = date::floor<date::days>(qso_time_off);
		const julian::year_month_day j_today_off{qso_day_off};
		const date::year_month_day g_today_off{qso_day_off};
		const auto jdn_off = qso_day_off.time_since_epoch().count() + 2440588;
		qso_msg.dateOff = jdn_off;

		// auto now_time = std::chrono::system_clock::now();
		uint32_t time_qso = std::chrono::duration_cast<std::chrono::milliseconds>(qso_time - date::floor<date::days>(qso_time)).count();
		qso_msg.timeOn = time_qso;
		qso_msg.timespecOn = 0;

		uint32_t time_qso_off = std::chrono::duration_cast<std::chrono::milliseconds>(qso_time_off - date::floor<date::days>(qso_time_off)).count();
		qso_msg.timeOff = time_qso;
		qso_msg.timespecOff = 0;

		qso_msg.dxCall = dxCall;
		qso_msg.dxGrid = dxGrid;
		qso_msg.txFrequency = txFrequency;
		if (mode == mode_ft8)
			qso_msg.mode = "FT8";
		if (mode == mode_ft4)
			qso_msg.mode = "FT4";
		if (mode == mode_wspr)
			qso_msg.mode = "WSPR";
		qso_msg.reportSent = reportSent;
		qso_msg.reportReceived = reportReceived;
		qso_msg.txPower = txPower;
		qso_msg.comments = comments;
		qso_msg.name = name;
		qso_msg.operatorCall = operatorCall;
		qso_msg.myCall = myCall;
		qso_msg.myGrid = myGrid;
		qso_msg.exchangeSent = exchangeSent;
		qso_msg.exchangeReceived = exchangeReceived;
		qso_msg.adifPropagationMode = adifPropagationMode;

		std::vector<char> packet = serialize_qso_message(qso_msg);

		if (sendto(sockfd, packet.data(), packet.size(),
				   MSG_CONFIRM, (const struct sockaddr *)&servaddr,
				   sizeof(servaddr)) < 0)
		{
			perror("sendto failed");
			close(sockfd);
			sockfd = 0;
			return;
		}
		std::cout << "QSO Logged message sent." << std::endl;
	}
}

void FT8UdpClient::SendStatus(struct StatusMessage status, int mode)
{

	if (sockfd > 0)
	{
		status.magic = MAGIC_NUMBER;
		status.Schema = 2;
		status.messageType = 1;
		status.id = "WSJT-X";

		if (mode == mode_ft8)
			status.mode = "FT8";
		if (mode == mode_ft4)
			status.mode = "FT4";
		if (mode == mode_wspr)
			status.mode = "WSPR";

		std::vector<char> packet = serialize_status_message(status);

		if (sendto(sockfd, packet.data(), packet.size(),
				   MSG_CONFIRM, (const struct sockaddr *)&servaddr,
				   sizeof(servaddr)) < 0)
		{
			perror("sendto failed");
			close(sockfd);
			sockfd = 0;
			return;
		}
		std::cout << "Status message sent." << std::endl;
	}
}