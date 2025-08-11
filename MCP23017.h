#pragma once
#include <cstdint>

#define MCP23017_I2C_ADDRESS 0x20    ///< The default I2C address of MCP23017.
#define _MCP23017_INTERRUPT_SUPPORT_ ///< Enables support for MCP23017 interrupts.

#define CHANGE 0
#define FALLING 1
#define RISING 2

#define MCP23017_INPUT			0
#define MCP23017_INPUT_PULLUP	1
#define MCP23017_OUTPUT			2

#define HIGH 1
#define LOW 0

enum class MCP23017Port : uint8_t
{
	A = 0,
	B = 1
};

struct MCP23017Pin
{
	enum Names {
		GPA0 = 0,
		GPA1,
		GPA2,
		GPA3,
		GPA4,
		GPA5,
		GPA6,
		GPA7,
		GPB0 = 8,
		GPB1,
		GPB2,
		GPB3,
		GPB4,
		GPB5,
		GPB6,
		GPB7
	};
};

/**
 * Controls if the two interrupt pins mirror each other.
 * See "3.6 Interrupt Logic".
 */
enum class MCP23017InterruptMode : uint8_t
{
	Separated = 0,	///< Interrupt pins are kept independent
	Or = 0b01000000	///< Interrupt pins are mirrored
};

/**
 * Registers addresses.
 * The library use addresses for IOCON.BANK = 0.
 * See "3.2.1 Byte mode and Sequential mode".
 */
enum class MCP23017Register : uint8_t
{
	IODIR_A		= 0x00, 		///< Controls the direction of the data I/O for port A.
	IODIR_B		= 0x01,			///< Controls the direction of the data I/O for port B.
	IPOL_A		= 0x02,			///< Configures the polarity on the corresponding GPIO_ port bits for port A.
	IPOL_B		= 0x03,			///< Configures the polarity on the corresponding GPIO_ port bits for port B.
	GPINTEN_A	= 0x04,			///< Controls the interrupt-on-change for each pin of port A.
	GPINTEN_B	= 0x05,			///< Controls the interrupt-on-change for each pin of port B.
	DEFVAL_A	= 0x06,			///< Controls the default comparaison value for interrupt-on-change for port A.
	DEFVAL_B	= 0x07,			///< Controls the default comparaison value for interrupt-on-change for port B.
	INTCON_A	= 0x08,			///< Controls how the associated pin value is compared for the interrupt-on-change for port A.
	INTCON_B	= 0x09,			///< Controls how the associated pin value is compared for the interrupt-on-change for port B.
	IOCON		= 0x0A,			///< Controls the device.
	GPPU_A		= 0x0C,			///< Controls the pull-up resistors for the port A pins.
	GPPU_B		= 0x0D,			///< Controls the pull-up resistors for the port B pins.
	INTF_A		= 0x0E,			///< Reflects the interrupt condition on the port A pins.
	INTF_B		= 0x0F,			///< Reflects the interrupt condition on the port B pins.
	INTCAP_A	= 0x10,			///< Captures the port A value at the time the interrupt occured.
	INTCAP_B	= 0x11,			///< Captures the port B value at the time the interrupt occured.
	GPIO_A		= 0x12,			///< Reflects the value on the port A.
	GPIO_B		= 0x13,			///< Reflects the value on the port B.
	OLAT_A		= 0x14,			///< Provides access to the port A output latches.
	OLAT_B		= 0x15,			///< Provides access to the port B output latches.
};

inline MCP23017Register operator+(MCP23017Register a, MCP23017Port b) {
	return static_cast<MCP23017Register>(static_cast<uint8_t>(a) + static_cast<uint8_t>(b));
};

class MCP23017
{
private:
	uint8_t _deviceAddr;
	int fd;

  public:
	/**
	 * Instantiates a new instance to interact with a MCP23017 at the specified address.
	 */
	MCP23017(uint8_t address);
	/**
	 * Instantiates a new instance to interact with a MCP23017 at the
	 * MCP23017_I2C_ADDRESS default.
	 */
	MCP23017();
	~MCP23017();
#ifdef _DEBUG
	void debug();
#endif
	/**
	 * Uses the I2C address set during construction. Implicitly calls init().
	 */
	bool begin();
	/**
	 * Overrides the I2C address set by the constructor. Implicitly calls begin().

	 */
	bool begin(uint8_t address);
	/**
	 * Initializes the chip with the default configuration.
	 * Enables Byte mode (IOCON.BANK = 0 and IOCON.SEQOP = 1).
	 * Enables pull-up resistors for all pins. This will only be effective for input pins.
	 * 
	 * See "3.2.1 Byte mode and Sequential mode".
	 */
	void init();
	/**
	 * Controls the pins direction on a whole port at once.
	 * 
	 * 1 = Pin is configured as an input.
	 * 0 = Pin is configured as an output.
	 * 
	 * See "3.5.1 I/O Direction register".
	 */
	void portMode(MCP23017Port port, uint8_t directions, uint8_t pullups = 0xFF, uint8_t inverted = 0x00);
	/**
	 * Controls a single pin direction. 
	 * Pin 0-7 for port A, 8-15 fo port B.
	 * 
	 * 1 = Pin is configured as an input.
	 * 0 = Pin is configured as an output.
	 *
	 * See "3.5.1 I/O Direction register".
	 * 
	 * Beware!  
	 * On Arduino platform, INPUT = 0, OUTPUT = 1, which is the inverse
	 * of the MCP23017 definition where a pin is an input if its IODIR bit is set to 1.
	 * This library pinMode function behaves like Arduino's standard pinMode for consistency.
	 * [ OUTPUT | INPUT | INPUT_PULLUP ]
	 */
	void pinMode(uint8_t pin, uint8_t mode, bool inverted = false);

	/**
	 * Writes a single pin state.
	 * Pin 0-7 for port A, 8-15 for port B.
	 * 
	 * 1 = Logic-high
	 * 0 = Logic-low
	 * 
	 * See "3.5.10 Port register".
	 */
	void digitalWrite(uint8_t pin, uint8_t state);
	/**
	 * Reads a single pin state.
	 * Pin 0-7 for port A, 8-15 for port B.
	 * 
	 * 1 = Logic-high
	 * 0 = Logic-low
	 * 
	 * See "3.5.10 Port register".
	 */ 
	uint8_t digitalRead(uint8_t pin);

	/**
	 * Writes pins state to a whole port.
	 * 
	 * 1 = Logic-high
	 * 0 = Logic-low
	 * 
	 * See "3.5.10 Port register".
	 */
	void writePort(MCP23017Port port, uint8_t value);
	/**
	 * Writes pins state to both ports.
	 * 
	 * 1 = Logic-high
	 * 0 = Logic-low
	 * 
	 * See "3.5.10 Port register".
	 */
	void write(uint16_t value);

	/**
	 * Reads pins state for a whole port.
	 * 
	 * 1 = Logic-high
	 * 0 = Logic-low
	 * 
	 * See "3.5.10 Port register".
	 */
	uint8_t readPort(MCP23017Port port);
	/**
	 * Reads pins state for both ports. 
	 * 
	 * 1 = Logic-high
	 * 0 = Logic-low
	 * 
	 * See "3.5.10 Port register".
	 */
	uint16_t read();

	/**
	 * Writes a single register value.
	 */
	void writeRegister(MCP23017Register reg, uint8_t value);
	/**
	 * Writes values to a register pair.
	 * 
	 * For portA and portB variable to effectively match the desired port,
	 * you have to supply a portA register address to reg. Otherwise, values
	 * will be reversed due to the way the MCP23017 works in Byte mode.
	 */
	void writeRegister(MCP23017Register reg, uint8_t portA, uint8_t portB);
	/**
	 * Reads a single register value.
	 */
	uint8_t readRegister(MCP23017Register reg);
	/**
	 * Reads the values from a register pair.
	 * 
	 * For portA and portB variable to effectively match the desired port,
	 * you have to supply a portA register address to reg. Otherwise, values
	 * will be reversed due to the way the MCP23017 works in Byte mode.
	 */
	void readRegister(MCP23017Register reg, uint8_t& portA, uint8_t& portB);

#ifdef _MCP23017_INTERRUPT_SUPPORT_

	/**
	 * Controls how the interrupt pins act with each other.
	 * If intMode is Separated, interrupt conditions on a port will cause its respective INT pin to active.
	 * If intMode is Or, interrupt pins are OR'ed so an interrupt on one of the port will cause both pints to active.
	 * 
	 * Controls the IOCON.MIRROR bit. 
	 * See "3.5.6 Configuration register".
	 */
	void interruptMode(MCP23017InterruptMode intMode);
	/**
	 * Configures interrupt registers using an Arduino-like API.
	 * mode can be one of CHANGE, FALLING or RISING.
	 */
	void interrupt(MCP23017Port port, uint8_t mode);
	/**
	 * Disable interrupts for the specified port.
	 */
	void disableInterrupt(MCP23017Port port);
	/**
	 * Reads which pin caused the interrupt.
	 */
	void interruptedBy(uint8_t& portA, uint8_t& portB);
	/**
	 * Clears interrupts on both ports.
	 */
	void clearInterrupts();
	/**
	 * Clear interrupts on both ports. Returns port values at the time the interrupt occured.
	 */
	void clearInterrupts(uint8_t& portA, uint8_t& portB);

	void bitSet(uint8_t &data, uint8_t bit) { data = data | 0x01 << bit; }
	void bitClear(uint8_t &data, uint8_t bit) { data = data & 0x01 << bit; }
	bool bitRead(uint8_t &data, uint8_t bit) { return data & 0x01 << bit; }
	uint8_t lowByte(uint16_t data) { return data & 0x00FF; };
	uint8_t highByte(uint16_t data) { return (data & 0xFF00) >> 8; };
#endif
};
