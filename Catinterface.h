#pragma once

#include "FT891_CAT.h"
#include "wiringSerial.h"
#include <atomic>
#include <mutex>
#include <string>

class Comm : public Cat_communicator
{
  public:
	bool begin();
	void Send(std::string s) override;
	int Read(char c, std::string &s) override;
	bool available() override;
	void SendInformation(int info) override;
	bool IsCommuncationPortOpen() override;

	~Comm() override;
	void Close();

  private:
	std::string device;
	int speed;
	int serialport;
};

class Catinterface
{
  public:
	void checkCAT();
	void begin();
	void operator()();
	void SetBand(uint16_t band);
	void InitVfo(uint32_t a, uint32_t b);
	void Pause_Cat(bool pause) { bpause_cat = pause; }
	void MuteFA(bool pause) { muteFA = pause; }

	FT891_CAT *operator->() { return &cat_message; }

  private:
	FT891_CAT cat_message;
	bool f_rxtx;
	Comm comm_port;
	int m_mode;
	int ifgain;
	int volume;
	int rfgain;
	std::atomic<bool> bpause_cat{false};
	std::atomic<bool> muteFA{false};
	int channel{};
	uint32_t vfo_a, vfo_b;
};

extern Catinterface catinterface;
