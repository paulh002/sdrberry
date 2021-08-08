#include "ble_interface.h"

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
#include "wstring.h"
#include <iostream>
#include <thread>
#include <atomic>
#include <csignal>
#include <signal.h>
#include "vfo.h"

#include <stdexcept>

using namespace tinyb;

//std::condition_variable cv;
//std::atomic<bool> running(true);

ble_class	Ble_instance;

void data_callback(BluetoothGattCharacteristic &c, std::vector<unsigned char> &data, void *userdata)
{
	/* Read temperature data and display it */
	unsigned char	*data_c;
	unsigned int	size = data.size();
	int				ii = 0;
	char			buf[81];
		
	if (size > 0 && size < 80) {
		data_c = data.data();
		memset(buf, 0, 80*sizeof(char));
		strncpy(buf,(char *)data_c,size);
		ii = atol((char *)data_c);
		vfo.step_vfo(ii);
	}
}


int ble_class::setup_ble(String mac_address)
{	char str[80];
	
	mac_address.toUpperCase();
	m_mac_address = mac_address;
	strcpy(str, mac_address.c_str());
	
	try {
		manager = BluetoothManager::get_bluetooth_manager();
	}
	catch (const std::runtime_error& e) {
		std::cerr << "Error while initializing libtinyb: " << e.what() << std::endl;
		exit(1);
	}
	
	/* Start the discovery of devices */
    bool ret = manager->start_discovery();
    std::cout << "Started = " << (ret ? "true" : "false") << std::endl;

    std::string device_mac(str);
	sensor_tag = manager->find<BluetoothDevice>(nullptr, &device_mac, nullptr, std::chrono::seconds(20));
    if (sensor_tag == nullptr) {
        std::cout << "Device not found" << std::endl;
        return 1;
    }
    
	sensor_tag->enable_connected_notifications([] (BluetoothDevice &d, bool connected, void *usedata)
        { if (connected) 
		        std::cout << "Connected " << d.get_name() << std::endl;  
	        else
		        std::cout << "Disconnected " << std::endl; 
	        Ble_instance.m_connected = connected;
        }, NULL);

    if (sensor_tag != nullptr) {
        /* Connect to the device and get the list of services exposed by it */
        sensor_tag->connect();
	    std::string service_uuid("30f43f3e-ea17-11eb-9a03-0242ac130003");
        std::cout << "Waiting for service " << service_uuid << " to be discovered" << std::endl; 
        encoder_service = sensor_tag->find(&service_uuid);
    } else {
       ret = manager->stop_discovery();
       std::cerr << "SensorTag not found after 30 seconds, exiting" << std::endl;
       return 1;
    }

	std::string service_uuid("30f43f3e-ea17-11eb-9a03-0242ac130003");
	std::cout << "Waiting for service " << service_uuid << " to be discovered" << std::endl;
	encoder_service = sensor_tag->find(&service_uuid);

	/* Stop the discovery (the device was found or timeout was over) */
	ret = manager->stop_discovery();
	std::cout << "Stopped = " << (ret ? "true" : "false") << std::endl;
	
	value_uuid = std::string("427ffc34-ea17-11eb-9a03-0242ac130003");
    encoder_value = encoder_service->find(&value_uuid);
    encoder_value->enable_value_notifications(data_callback, nullptr);
	return 0;
	}

ble_class::~ble_class()
{
	if (sensor_tag != nullptr)
		sensor_tag->disconnect();
}
	
int ble_class::connect()
{	
	char str[80];
	
	if (m_connected || m_mac_address.length() == 0)
		return 0;

	bool ret = manager->start_discovery();
	std::cout << "Started = " << (ret ? "true" : "false") << std::endl;
	
	strcpy(str, m_mac_address.c_str());
	std::string device_mac(str);
	sensor_tag = manager->find<BluetoothDevice>(nullptr, &device_mac, nullptr, std::chrono::seconds(20));
	if (sensor_tag == nullptr) {
		std::cout << "Device not found" << std::endl;
		return 1;
	}

	std::cout << " enable_connected_notifications" << std::endl;
	sensor_tag->enable_connected_notifications([](BluetoothDevice &d, bool connected, void *usedata)
		{
			if (connected) 
				std::cout << "Connected " << d.get_name() << std::endl;  
			else
				std::cout << "Disconnected " << std::endl; 
			Ble_instance.m_connected = connected;
		},
		NULL);

	std::cout << " connect" << std::endl;
	if (sensor_tag != nullptr) {
		/* Connect to the device and get the list of services exposed by it */
		sensor_tag->connect();
		std::string service_uuid("30f43f3e-ea17-11eb-9a03-0242ac130003");
		std::cout << "Waiting for service " << service_uuid << " to be discovered" << std::endl; 
		encoder_service = sensor_tag->find(&service_uuid);
	}
	else {
		ret = manager->stop_discovery();
		std::cerr << "SensorTag not found after 30 seconds, exiting" << std::endl;
		return 1;
	}

	std::cout << " encoder_service" << std::endl;
	std::string service_uuid("30f43f3e-ea17-11eb-9a03-0242ac130003");
	std::cout << "Waiting for service " << service_uuid << " to be discovered" << std::endl;
	encoder_service = sensor_tag->find(&service_uuid);

	/* Stop the discovery (the device was found or timeout was over) */
	std::cout << " stop_discovery" << std::endl;
	ret = manager->stop_discovery();
	std::cout << "Stopped = " << (ret ? "true" : "false") << std::endl;
	
	std::cout << " encoder_service->find" << std::endl;
	value_uuid = std::string("427ffc34-ea17-11eb-9a03-0242ac130003");
	encoder_value = encoder_service->find(&value_uuid);
	encoder_value->enable_value_notifications(data_callback, nullptr);
	m_connected = true;
	return 0;
}

