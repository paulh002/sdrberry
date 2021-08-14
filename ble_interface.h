#pragma once
#include <tinyb.hpp>


class ble_class
{
public:
	~ble_class();
	int setup_ble(std::string mac_address);	
	std::unique_ptr <BluetoothDevice>					sensor_tag;
	int connect();
	
private:
	BluetoothManager									*manager = nullptr;
	std::unique_ptr<BluetoothGattService>				encoder_service;
	std::unique_ptr<BluetoothGattCharacteristic>		encoder_value;
	std::string											value_uuid;
	bool												m_connected = false;
	std::string											m_mac_address;
};

extern ble_class	Ble_instance;
