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

	~Comm();
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
	uint8_t Getag();
	long long Getfa();
	uint8_t Getrg();
	void SetBand(uint16_t band);
	void SetAG(uint8_t volume);
	void SetSH(int bandwidth);
	void SetFA(uint32_t freq);
	void Pause_Cat(bool pause) { bpause_cat = pause; }

  private:
	FT891_CAT cat_message;
	bool f_rxtx;
	Comm comm_port;
	int m_mode;
	std::atomic<bool> bpause_cat{false};
	int channel{};
};

extern Catinterface catinterface;
