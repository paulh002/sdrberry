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
	uint8_t Getag();
	long long Getfa();
	uint8_t GetRG();
	void SetBand(uint16_t band);
	void SetAG(uint8_t volume);
	void SetSH(int bandwidth);
	void SetFA(uint32_t freq);
	void Pause_Cat(bool pause) { bpause_cat = pause; }
	void MuteFA(bool pause) { muteFA = pause; }
	void SetSM(uint8_t sm);
	uint8_t GetIG();
	void SetIG(int ig);
	void SetRG(int rg);
	void SetTX(int tx);

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
};

extern Catinterface catinterface;
