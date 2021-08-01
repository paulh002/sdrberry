/*
 Name:		ESPBleControler.ino
 Created:	7/21/2021 11:43:23 PM
 Author:	paulh
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <ESP32Encoder.h>
#include <AceButton.h>
using namespace ace_button;

/*-------------------------------------------------------
   Optical Rotary encoder settings (used for frequency)
--------------------------------------------------------*/
#define ROTARY_A      16      
#define ROTARY_B      17
#define ROTARY_PRESS  26

ESP32Encoder	GuiEncoder;
AceButton       rotary_button(ROTARY_PRESS);

int EncoderCount = 0;
int EncoderPress = 0;
uint8_t value = 0;  //the set value function only accepts unsigned 8 bit integers

/* Define the UUID for our Custom Service */
#define SERVICE_UUID        "30f43f3e-ea17-11eb-9a03-0242ac130003"
#define CHARACTERISTIC_UUID "427ffc34-ea17-11eb-9a03-0242ac130003"
//#define serviceID BLEUUID((uint16_t)0x1700)
#define serviceID BLEUUID(SERVICE_UUID)

/* Define our custom characteristic along with it's properties */
BLECharacteristic customCharacteristic(
	BLEUUID(CHARACTERISTIC_UUID),
	BLECharacteristic::PROPERTY_READ |
	BLECharacteristic::PROPERTY_NOTIFY
);

static void rotary_button_eventhandler(AceButton*, uint8_t eventType, uint8_t buttonState)
{
	switch (eventType) {
	case AceButton::kEventLongPressed:
		EncoderPress = 2;
		Serial.println("Long press");
		break;
	case AceButton::kEventPressed:
		EncoderPress = 1;
		Serial.println("Pressed");
		break;

	case AceButton::kEventReleased:
		Serial.println("Released");
		break;
	}
}

/* This function handles the server callbacks */
bool deviceConnected = false;
class ServerCallbacks : public BLEServerCallbacks {
	void onConnect(BLEServer* MyServer) {
		deviceConnected = true;
		Serial.println("Client connected");
	};

	void onDisconnect(BLEServer* MyServer) {
		Serial.println("Client disconnected");
		deviceConnected = false;
	}
};

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(115200); 
	pinMode(ROTARY_PRESS, INPUT);
	rotary_button.setEventHandler(rotary_button_eventhandler); 
	ESP32Encoder::useInternalWeakPullResistors = NONE;
	GuiEncoder.attachHalfQuad(ROTARY_B, ROTARY_A);
	GuiEncoder.setFilter(1023);

	BLEDevice::init("BleControler");

	/* Create the BLE Server */
	BLEServer* MyServer = BLEDevice::createServer();
	MyServer->setCallbacks(new ServerCallbacks());  // Set the function that handles Server Callbacks

	/* Add a service to our server */
	BLEService* customService = MyServer->createService(serviceID); //  A random ID has been selected

	/* Add a characteristic to the service */
	customService->addCharacteristic(&customCharacteristic);  //customCharacteristic was defined above

	/* Add Descriptors to the Characteristic*/
	customCharacteristic.addDescriptor(new BLE2902());  //Add this line only if the characteristic has the Notify property

	BLEDescriptor VariableDescriptor(BLEUUID((uint16_t)0x2901));  /*```````````````````````````````````````````````````````````````*/
	VariableDescriptor.setValue("Controler data");          /* Use this format to add a hint for the user. This is optional. */
	customCharacteristic.addDescriptor(&VariableDescriptor);    /*```````````````````````````````````````````````````````````````*/

	/* Configure Advertising with the Services to be advertised */
	MyServer->getAdvertising()->addServiceUUID(serviceID);

	// Start the service
	customService->start();

	// Start the Server/Advertising
	MyServer->getAdvertising()->start();

	Serial.println("Waiting for a Client to connect...");
}

// the loop function runs over and over again until power down or reset
void loop() {
	rotary_button.check();
	int count = GuiEncoder.getCount();
	GuiEncoder.clearCount();
	if (count != 0)
	{
		Serial.println("Count :" + String(count));
		value = min(count, 255);
		if (deviceConnected) {
			/* Set the value */
			Serial.println("+Count :" + String(count));
			char buffer[20];
			dtostrf(count, 1, 0, buffer);
			customCharacteristic.setValue((char*)&buffer);
			customCharacteristic.notify();
		//	customCharacteristic.setValue(&value, 1);  // This is a value of a single byte
		//	customCharacteristic.notify();  // Notify the client of a change
		}
		delay(50);
	}
}
