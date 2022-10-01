#pragma once
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <tinyb.hpp>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <cerrno>
#include <array>
#include <iomanip>
#include <vector>
#include <iostream>
#include <thread>
#include <atomic>
#include <csignal>
#include <signal.h>
#include <stdexcept>
#include <string>

inline void to_upper(std::string &str)
{
	for (auto &i : str)
		i = toupper(i);
};

class ble_class
{
public:
	ble_class() {};
	~ble_class();
	int		setup_ble();	
	std::unique_ptr <BluetoothDevice>					sensor_tag;
	int		connect();
	void	set_mac_address(std::string mac) {to_upper(mac); m_mac_address = mac; }
	bool	is_connected() {return m_connected;}
	void	operator()();
	
private:
	BluetoothManager									*manager = nullptr;
	std::unique_ptr<BluetoothGattService>				encoder_service;
	std::unique_ptr<BluetoothGattCharacteristic>		encoder_value;
	std::string											value_uuid;
	bool												m_connected = false;
	std::string											m_mac_address;
};

extern ble_class	Ble_instance;
int create_ble_thread(std::string mac_addres);
