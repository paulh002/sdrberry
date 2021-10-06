#pragma once

#include <string>
#include <atomic>
#include <mutex>
#include "FT891_CAT.h"
#include "wiringSerial.h"


	
class Comm : public Cat_communicator
{
public:
	bool	begin();
	void	Send(std::string s) override;
	void	Read(char c, std::string& s) override;
	bool	available() override;
	void	SendInformation(int info) override;
	~Comm();

private:
	std::string					device;
	int							speed;
	bool						bserial {false};
	int							serialport;
};

class Catinterface
{
public:
	void		checkCAT();
	void		begin();
	void		operator()();
	uint8_t		Getag();
	long long	Getfa();
	uint8_t		Getrg();
	void		SetBand(uint16_t band);


private:
	FT891_CAT		cat_message;
	bool			f_rxtx;
	Comm			comm_port;
	bool			bcomm_port;
	int				m_mode;		
};


extern Catinterface	catinterface;