#include "MCP23017.h"
#include "wiringPiI2C.h"

MCP23017::MCP23017(uint8_t address) {
	_deviceAddr = address;
}

MCP23017::MCP23017() {
	_deviceAddr = MCP23017_I2C_ADDRESS;
}

MCP23017::~MCP23017() {}

void MCP23017::init()
{
	//BANK = 	0 : sequential register addresses
	//MIRROR = 	0 : use configureInterrupt 
	//SEQOP = 	1 : sequential operation disabled, address pointer does not increment
	//DISSLW = 	0 : slew rate enabled
	//HAEN = 	0 : hardware address pin is always enabled on 23017
	//ODR = 	0 : active driver output (INTPOL bit sets the polarity.)

	//INTPOL = 	0 : interrupt active low
	//UNIMPLMENTED 	0 : unimplemented: Read as ‘0’

	writeRegister(MCP23017Register::IOCON, 0b00100000);
}

bool MCP23017::begin()
{
	if ((fd = wiringPiI2CSetup(_deviceAddr)) < 0)
		return false;
	init();
	return true;
}

bool MCP23017::begin(uint8_t address)
{
	_deviceAddr = address;
	return begin();
}

void MCP23017::portMode(MCP23017Port port, uint8_t directions, uint8_t pullups, uint8_t inverted)
{
	writeRegister(MCP23017Register::IODIR_A + port, directions);
	writeRegister(MCP23017Register::GPPU_A + port, pullups);
	writeRegister(MCP23017Register::IPOL_A + port, inverted);
}

void MCP23017::pinMode(uint8_t pin, uint8_t mode, bool inverted)
{
	MCP23017Register iodirreg = MCP23017Register::IODIR_A;
	MCP23017Register pullupreg = MCP23017Register::GPPU_A;
	MCP23017Register polreg = MCP23017Register::IPOL_A;
	uint8_t iodir, pol, pull;

	if(pin > 7)
	{
		iodirreg = MCP23017Register::IODIR_B;
		pullupreg = MCP23017Register::GPPU_B;
		polreg = MCP23017Register::IPOL_B;
		pin -= 8;
	}

	iodir = readRegister(iodirreg);
	if (mode == MCP23017_INPUT || mode == MCP23017_INPUT_PULLUP) bitSet(iodir, pin);
	else bitClear(iodir, pin);

	pull = readRegister(pullupreg);
	if (mode == MCP23017_INPUT_PULLUP) bitSet(pull, pin);
	else bitClear(pull, pin);

	pol = readRegister(polreg);
	if(inverted) bitSet(pol, pin);
	else bitClear(pol, pin);

	writeRegister(iodirreg, iodir);
	writeRegister(pullupreg, pull);
	writeRegister(polreg, pol);
}

void MCP23017::digitalWrite(uint8_t pin, uint8_t state)
{
	MCP23017Register gpioreg = MCP23017Register::GPIO_A;
	uint8_t gpio;
	if(pin > 7)
	{
		gpioreg = MCP23017Register::GPIO_B;
		pin -= 8;
	}

	gpio = readRegister(gpioreg);
	if(state > 0) bitSet(gpio, pin);
	else bitClear(gpio, pin);

	writeRegister(gpioreg, gpio);
}

uint8_t MCP23017::digitalRead(uint8_t pin)
{
	MCP23017Register gpioreg = MCP23017Register::GPIO_A;
	uint8_t gpio;
	if(pin > 7)
	{
		gpioreg = MCP23017Register::GPIO_B;
		pin -=8;
	}

	gpio = readRegister(gpioreg);
	if(bitRead(gpio, pin)) return HIGH;
	return LOW;
}

void MCP23017::writePort(MCP23017Port port, uint8_t value)
{
	writeRegister(MCP23017Register::GPIO_A + port, value);
}

void MCP23017::write(uint16_t value)
{
	writeRegister(MCP23017Register::GPIO_A, lowByte(value), highByte(value));
}

uint8_t MCP23017::readPort(MCP23017Port port)
{
	return readRegister(MCP23017Register::GPIO_A + port);
}

uint16_t MCP23017::read()
{
	uint8_t a = readPort(MCP23017Port::A);
	uint8_t b = readPort(MCP23017Port::B);

	return a | b << 8;
}

void MCP23017::writeRegister(MCP23017Register reg, uint8_t value)
{
	wiringPiI2CWriteReg8(fd, (int)reg, value);
	//_bus->beginTransmission(_deviceAddr);
	//_bus->write(static_cast<uint8_t>(reg));
	//_bus->write(value);
	//_bus->endTransmission();
}

void MCP23017::writeRegister(MCP23017Register reg, uint8_t portA, uint8_t portB){
	
	int value = (portA << 8) | portA;
	int ret = wiringPiI2CWriteReg16(fd, (int)reg, value);

	//_bus->beginTransmission(_deviceAddr);
	//_bus->write(static_cast<uint8_t>(reg));
	//_bus->write(portA);
	//_bus->write(portB);
	//_bus->endTransmission();
}


uint8_t MCP23017::readRegister(MCP23017Register reg)
{

	int ret = wiringPiI2CRead(fd);
	return ret;

	//_bus->beginTransmission(_deviceAddr);
	//_bus->write(static_cast<uint8_t>(reg));
	//_bus->endTransmission();
	//_bus->requestFrom(_deviceAddr, (uint8_t)1);
	//return _bus->read();
}

void MCP23017::readRegister(MCP23017Register reg, uint8_t& portA, uint8_t& portB)
{
	int ret = wiringPiI2CReadReg16(fd, (int)reg);
	portA = (ret & 0xFF00) >> 8;
	portB = ret & 0x00FF;

	//_bus->beginTransmission(_deviceAddr);
	//_bus->write(static_cast<uint8_t>(reg));
	//_bus->endTransmission();
	//_bus->requestFrom(_deviceAddr, (uint8_t)2);
	//portA = _bus->read();
	//portB = _bus->read();
}

#ifdef _MCP23017_INTERRUPT_SUPPORT_

void MCP23017::interruptMode(MCP23017InterruptMode intMode)
{
	uint8_t iocon = readRegister(MCP23017Register::IOCON);
	if(intMode == MCP23017InterruptMode::Or) iocon |= static_cast<uint8_t>(MCP23017InterruptMode::Or);
	else iocon &= ~(static_cast<uint8_t>(MCP23017InterruptMode::Or));

	writeRegister(MCP23017Register::IOCON, iocon);
}

void MCP23017::interrupt(MCP23017Port port, uint8_t mode)
{
	MCP23017Register defvalreg = MCP23017Register::DEFVAL_A + port;
	MCP23017Register intconreg = MCP23017Register::INTCON_A + port;

	//enable interrupt for port
	writeRegister(MCP23017Register::GPINTEN_A + port, 0xFF);
	switch(mode)
	{
	case CHANGE:
		//interrupt on change
		writeRegister(intconreg, 0);
		break;
	case FALLING:
		//interrupt falling : compared against defval, 0xff
		writeRegister(intconreg, 0xFF);
		writeRegister(defvalreg, 0xFF);
		break;
	case RISING:
		//interrupt rising : compared against defval, 0x00
		writeRegister(intconreg, 0xFF);
		writeRegister(defvalreg, 0x00);
		break;
	}
}

void MCP23017::interruptedBy(uint8_t& portA, uint8_t& portB)
{
	readRegister(MCP23017Register::INTF_A, portA, portB);
}

void MCP23017::disableInterrupt(MCP23017Port port)
{
	writeRegister(MCP23017Register::GPINTEN_A + port, 0x00);
}

void MCP23017::clearInterrupts()
{
	uint8_t a, b;
	clearInterrupts(a, b);
}

void MCP23017::clearInterrupts(uint8_t& portA, uint8_t& portB)
{
	readRegister(MCP23017Register::INTCAP_A, portA, portB);
}

#endif
