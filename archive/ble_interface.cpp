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

#include <stdexcept>

using namespace tinyb;

std::condition_variable cv;
std::atomic<bool> running(true);

void data_callback(BluetoothGattCharacteristic &c, std::vector<unsigned char> &data, void *userdata)
{
	/* Read temperature data and display it */
	unsigned char *data_c;
	unsigned int size = data.size();
	if (size > 0) {
		data_c = data.data();

		std::cout << "Raw data=[";
		for (unsigned i = 0; i < size; i++)
			std::cout << std::hex << static_cast<int>(data_c[i]) << ", ";
		std::cout << "] "; std::cout << std::endl;

/*		uint16_t ambient_temp, object_temp;
		object_temp = data_c[0] | (data_c[1] << 8);
		ambient_temp = data_c[2] | (data_c[3] << 8);

		std::cout << "Ambient temp: " << celsius_temp(ambient_temp) << "C ";
		std::cout << "Object temp: " << celsius_temp(object_temp) << "C ";
		std::cout << std::endl; */
	}
}

void signal_handler(int signum)
{
	if (signum == SIGINT) {
		cv.notify_all();
	}
}

int setup_ble(String mac_address)
{	char str[80];
	
	mac_address.toUpperCase();
	strcpy(str, mac_address.c_str());
	
	
	BluetoothManager *manager = nullptr;
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

    std::unique_ptr<BluetoothGattService> encoder_service;

    std::string device_mac(str);
    auto sensor_tag = manager->find<BluetoothDevice>(nullptr, &device_mac, nullptr, std::chrono::seconds(10));
    if (sensor_tag == nullptr) {
        std::cout << "Device not found" << std::endl;
        return 1;
    }
    sensor_tag->enable_connected_notifications([] (BluetoothDevice &d, bool connected, void *usedata)
        { if (connected) std::cout << "Connected " << d.get_name() << std::endl;  }, NULL);

    if (sensor_tag != nullptr) {
        /* Connect to the device and get the list of services exposed by it */
        sensor_tag->connect();
	    std::string service_uuid("30f43f3e-ea17-11eb-9a03-0242ac130003");
        std::cout << "Waiting for service " << service_uuid << "to be discovered" << std::endl; 
        encoder_service = sensor_tag->find(&service_uuid);
    } else {
       ret = manager->stop_discovery();
       std::cerr << "SensorTag not found after 30 seconds, exiting" << std::endl;
       return 1;
    }

	/* Connect to the device and get the list of services exposed by it */
	sensor_tag->connect();
	std::string service_uuid("30f43f3e-ea17-11eb-9a03-0242ac130003");
	std::cout << "Waiting for service " << service_uuid << " to be discovered" << std::endl;
	encoder_service = sensor_tag->find(&service_uuid);

	/* Stop the discovery (the device was found or timeout was over) */
	ret = manager->stop_discovery();
	std::cout << "Stopped = " << (ret ? "true" : "false") << std::endl;
	
	auto value_uuid = std::string("427ffc34-ea17-11eb-9a03-0242ac130003");
    auto encoder_value = encoder_service->find(&value_uuid);

   // auto config_uuid = std::string("30f43f3e-ea17-11eb-9a03-0242ac130003");
    //auto encoder_config = encoder_service->find(&config_uuid);

    //auto period_uuid = std::string("30f43f3e-ea17-11eb-9a03-0242ac130003");
    //auto encoder_period = encoder_service->find(&period_uuid);

    /* Activate the temperature measurements */
   //std::vector<unsigned char> config_on {0x01};
    //encoder_config->write_value(config_on);
    //encoder_period->write_value({100});
    encoder_value->enable_value_notifications(data_callback, nullptr);

    std::mutex m;
    std::unique_lock<std::mutex> lock(m);

    std::signal(SIGINT, signal_handler);

    cv.wait(lock);

    /* Disconnect from the device */
    if (sensor_tag != nullptr)
        sensor_tag->disconnect();
	return 0;
	}
	