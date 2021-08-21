#pragma once
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <tinyb.hpp>
#include "lvgl/lvgl.h"
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
#include "vfo.h"
#include "RtAudio.h"
#include "Audiodefs.h"
#include "DataBuffer.h"
#include "AudioOutput.h"
#include "AudioInput.h"
#include "gui_right_pane.h"
#include "sdrberry.h"
#include <stdexcept>

class ble_class
{
public:
	~ble_class();
	int		setup_ble();	
	std::unique_ptr <BluetoothDevice>					sensor_tag;
	int		connect();
	void	set_mac_address(string mac) {to_upper(mac); m_mac_address = mac; }
	bool	is_connected() {return m_connected;}
	
private:
	BluetoothManager									*manager = nullptr;
	std::unique_ptr<BluetoothGattService>				encoder_service;
	std::unique_ptr<BluetoothGattCharacteristic>		encoder_value;
	std::string											value_uuid;
	bool												m_connected = false;
	std::string											m_mac_address;
};

extern ble_class	Ble_instance;
int create_ble_thread(string mac_addres);
