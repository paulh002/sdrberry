#include "ble_interface.h"

using namespace tinyb;

//std::condition_variable cv;
//std::atomic<bool> running(true);

ble_class	Ble_instance;
extern mutex	gui_mutex;

void data_callback(BluetoothGattCharacteristic &c, std::vector<unsigned char> &data, void *userdata)
{
	/* Read temperature data and display it */
	unsigned char	*data_c;
	unsigned int	size = data.size();
	int				ii = 0;
	char			buf[81];
		
	if (size > 0 && size < 80) {
		data_c = data.data();
		memset(buf, 0, 81*sizeof(char));		
		strncpy(buf,(char *)data_c,size);
		//printf("%s\n", buf);
		if(isdigit(buf[0]) || isdigit(buf[1]))
		{
			char *ptr = strchr(buf, ';');
			if (ptr != NULL)
				*ptr = '\0';
			ii = atoi(buf);
			if (ii)
			{
				unique_lock<mutex> gui_lock(gui_mutex);
				vfo.step_vfo(ii);	
			}
			return;
		}
		char *ptr1 = strstr(buf, "VOL");
		if (ptr1)
		{
			char *ptr = strchr(ptr1, ';');
			if (ptr != NULL)
				*ptr = '\0';
			ii = atoi(ptr1+3);
			if (ii)
			{
				unique_lock<mutex> gui_lock(gui_mutex);
				step_vol_slider(ii);
			}
		}
		ptr1 = strstr(buf, "GAIN");
		if (ptr1)
		{
			char *ptr = strchr(ptr1, ';');
			if (ptr != NULL)
				*ptr = '\0';
			ii = atoi(ptr1+4);
			if (ii)
			{
				unique_lock<mutex> gui_lock(gui_mutex);
				step_gain_slider(ii);
			}
		}
		ptr1 = strstr(buf, "TX");
		if (ptr1)
		{
			char *ptr = strchr(ptr1, ';');
			if (ptr != NULL)
				*ptr = '\0';
			// start tx mode
			{
				unique_lock<mutex> gui_lock(gui_mutex);
				select_mode_tx(mode);
			}
		}
		ptr1 = strstr(buf, "RX");
		if (ptr1)
		{
			char *ptr = strchr(ptr1, ';');
			if (ptr != NULL)
				*ptr = '\0';
			// start rx mode
			{
				unique_lock<mutex> gui_lock(gui_mutex);
				select_mode(mode);
			}
		}
	}
}


int ble_class::setup_ble()
{	char str[80];
	
	//to_upper(mac_address);
	//m_mac_address = mac_address;
	strcpy(str, m_mac_address.c_str());
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
	m_connected = true;
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

pthread_t	ble_thread_handle {0};
	
void* ble_thread(void* psdr_dev)
{
	Ble_instance.setup_ble();
	while (1)
	{
		if (!Ble_instance.is_connected())
		{
			Ble_instance.connect();
		}
		usleep(5000);
	}
}

int create_ble_thread(string mac_address)
{
	Ble_instance.set_mac_address(mac_address);
	if (ble_thread_handle == 0)
		return pthread_create(&ble_thread_handle, NULL, ble_thread, NULL);
	return 0;
}