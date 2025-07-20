#pragma once
#pragma once
#include <string>
#include <vector>
#include <endian.h>
#include <arpa/inet.h>
#include <chrono>
#include "tz.h"

#define PORT 2237
#define BUFFER_SIZE 1024
#define MAGIC_NUMBER 0xadbccbda

// Structure for the Heartbeat message
struct HeartbeatMessage
{
	uint32_t magic;
	uint32_t Schema;
	uint32_t messageType;
	uint32_t MaxSchema;
	std::string version;
	std::string revision;
};

// Structure for the QSO Logged message
struct QsoLoggedMessage
{
	uint32_t magic;
	uint32_t Schema;
	uint32_t messageType;
	std::string id;
	uint64_t dateOff; // Milliseconds since epoch
	uint32_t timeOff;
	uint8_t timespecOff;
	uint32_t offset;
	// timezone ??;
	std::string dxCall;
	std::string dxGrid;
	uint64_t txFrequency;
	std::string mode;
	std::string reportSent;
	std::string reportReceived;
	std::string txPower;
	std::string comments;
	std::string name;
	uint64_t dateOn;
	uint32_t timeOn;
	uint8_t timespecOn;
	std::string operatorCall;
	std::string myCall;
	std::string myGrid;
	std::string exchangeSent;
	std::string exchangeReceived;
	std::string adifPropagationMode;
};

// Structure for the Decode message
struct DecodeMessage
{
	uint32_t magic;
	uint32_t Schema;
	uint32_t messageType;
	std::string id;
	bool isNew;
	uint32_t time; // Milliseconds since epoch
	int32_t snr;
	double deltaTime;		 // Seconds
	uint32_t deltaFrequency; // Hz
	std::string mode;
	std::string message;
	bool lowConfidence;
	bool offAir;
};

// Structure for the Status message
struct StatusMessage
{
	uint32_t magic;
	uint32_t Schema;
	uint32_t messageType;
	std::string id;
	uint64_t dialFrequency;
	std::string mode;
	std::string dxCall;
	std::string report;
	std::string txMode;
	bool txEnabled;
	bool transmitting;
	bool decoding;
	uint32_t rxDF;
	uint32_t txDF;
	std::string deCall;
	std::string deGrid;
	std::string dxGrid;
	bool txWatchdog;
	std::string subMode;
	bool fastMode;
	uint8_t specialOperationMode;
	uint32_t frequencyTolerance;
	uint32_t trPeriod;
	std::string configurationName;
	std::string txMessage;
};

class FT8UdpClient
{
  public:
	~FT8UdpClient();
	FT8UdpClient(int wsjtx_mode);
	void SendQso(date::zoned_time<std::chrono::seconds> utc_time, date::zoned_time<std::chrono::seconds> utc_time_off, std::string dxCall, std::string dxGrid, uint64_t txFrequency, int mode, std::string reportSent,
				 std::string reportReceived, std::string txPower, std::string comments, std::string name, std::string operatorCall, std::string myCall,
				 std::string myGrid, std::string exchangeSent, std::string exchangeReceived, std::string adifPropagationMode);
	void SendHeartBeat();
	void SendDecode(bool isNew, uint32_t now_ms, int32_t snr, double deltaTime, uint32_t deltaFrequency, int mode, std::string message, bool lowConfidence, bool offAir);
	void SendStatus();
	
  private:
	int sockfd;
	int mode;
	char buffer[BUFFER_SIZE];
	struct sockaddr_in servaddr;

	std::vector<char> serialize_heartbeat_message(const HeartbeatMessage &msg);
	std::vector<char> serialize_decode_message(const DecodeMessage &msg);
	std::vector<char> serialize_qso_message(const QsoLoggedMessage &msg);
};

