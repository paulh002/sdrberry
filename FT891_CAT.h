/*
 *	"FT_891_CAT.h" defines the sub-set of the Yeasu FT-891 CAT language.
 *
 *	This file along with the corresponding cpp file can be installed in a directory named
 *	"FT_891_CAT" in your Arduino library directory and used just like any other Arduino
 *	library. In other words, it could also be used by other programs as-is.
 *
 *	Modified 09/26/2019 - Version 1.1
 *
 *		Added a second version of the "begin" function which does not take a GPIO pin
 *		number that would be used to key the transmitter. If the pin number is specified,
 *		the library will manipulate the specified pin whenever a command to switch from
 *		transmit to receive or vice-versa is received.
 *
 *		If the GPIO pin number is not specified, it will be up to the using application
 *		to handle the transmit/receive functionality.
 *
 *	Not Modified 06/01/2021 (Version 1.3)
 *
 *		The .cpp file was modified to fix a bug in the handling of the "IF", "OI" and "MD"
 *		status requests, however nothing was changed in this file.
 */

#ifndef _FT891_CAT_H_ // Prevent double include
#define _FT891_CAT_H_

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <stdint.h>
#include <string>
#include <vector>
#include <sstream>
#include <unistd.h>

/*
 *	This is a macro that is used to determine the number of elements in an array. It figures
 *	that out by dividing the total size of the array by the size of a single element. This is
 *	how we will calculate the number of entries in the "msgTable" array.
 */

#define ELEMENTS(x) (sizeof(x) / sizeof(x[0]))

/*
 *	Message type symbols; some messages can only be commands (eg: "AB;") and some
 *	can only be status requests (eg: "RM;"). Some can be both. We use these symbols
 *	in the "Type" field of the "msg" structure to indicate what the possibilities are.
 *	When the message is actually processed, the processing code will determine what the
 *	actual type is for those that can be either.
 *
 *	The decision as to whether the message is a command or status request is typically
 *	based on whether or not there is data attached to the message.
 */

#define MSG_STS 0x01  // Status request message
#define MSG_CMD 0x02  // Command
#define MSG_BOTH 0x03 // Could be either

#define TERM_CH ';' // Message terminator for FT-891 messages

struct msg // Keeps everything together
{
	char Name[4]; // 2 or 3 character message name plus null terminator
	uint8_t ID;   // The internal message number
	uint8_t Type; // Status, Command or both
};

/*
 *	These are numerical values which define the message being processed. We could just 
 *	work off the ASCII string, but by translating the strings to a numerical value, we
 *	leave open the possibility of handling CAT languages other than the FT-891 language
 *	easily (the Kenwood languages are very similar, for example).
 *
 *	Note that some of these messages are only here because the DxCommander program that
 *	I use to control my FT-891 (and anything I build this into) sends status requests for
 *	the parameters even though they are things we don't have the capability to change in
 *	the radio. The responses to such messages (e.g., "IS", "NA", "RI" and others) will
 *	be canned responses.
 */

#define MSG_NONE 0 // Unrecognized command
#define MSG_AB 1   // Copy VFO-A to VFO-B
#define MSG_AI 2   // (Data 0 or 1) Turn auto-information on or off
#define MSG_BA 3   // Copy VFO-B to VFO-A
#define MSG_BS 4   // Band select
#define MSG_EX 5   // FT-891 "Menu" command
#define MSG_FA 6   // Set or request VFO-A frequency
#define MSG_FB 7   // Set or request VFO-B frequency
#define MSG_ID 8   // Request radio's ID (0650 for the FT-891)
#define MSG_IF 9   // Information request/answer
#define MSG_IS 10  // IF Shift amount & Status
#define MSG_MD0 11  // Set or request operating mode
#define MSG_MD1 12	// Set or request operating mode
#define MSG_NA 13  // Set or request narrow IF shift
#define MSG_OI 14  // Opposite Band Information request/answer
#define MSG_RI 15  // Radio information request
#define MSG_RM 16  // Read meter
#define MSG_SH 17  // Set or request IF bandwidth
#define MSG_SM 18  // Read S-meter
#define MSG_ST 19  // (Data 0 - 2) Split mode off, on or on +5KHz up
#define MSG_SV 20  // Swap VFOs
#define MSG_TX 21  // Set or request transmit/receive status
#define MSG_FT 22  // Frequency Tuning delta frequency
#define MSG_AG 23  // Set or request volume
#define MSG_RG 24  // Set or request rf gain
#define MSG_GT 25  // Get information command
#define MSG_IG 26  // Get and set if command
#define MSG_PS 27  // Power status
#define MSG_RT 28  // RIT on/off
#define MSG_RD 29  // RIT delta frequency

/*
 *	Miscellaneous definitions:
 */

#define BUF_LEN 128		   // Size of all buffers (Max FT-891 message is 41 bytes)
#define BUF_COUNT 8		   // Number of transmit buffers
#define CAT_READ_TIME 25UL // Only check for incoming messages every 25mS

/*
 *	These definitions are used to key or un-key the transmitter when a "TX0;" ot "TX1;"
 *	CAT command is received. The original settings are such that the GPIO pin used to
 *	key the transmitter will go HIGH to key it and LOW puts it in receive mode. You can
 *	switch the definitions if necessary for your hardware configuration.
 */

#define XMIT_ON HIGH
#define XMIT_OFF LOW

/*
 *	These definitions are for the transmit modes. The definitions are a bit different
 *	than the ones normally used by the FT-891. 
 *
 *	In the "SetTX" function, we're only going to key the transmitter if it is in receive
 *	mode, not if it's already in "TX_MAN" mode. Similarly, we won't un-key it if it's in
 *	"TX_MAN" mode.
 */

#define TX_OFF 0 // Receive mode
#define TX_CAT 1 // Transmitting because of CAT command
#define TX_MAN 2 // Transmitter keyed manually
#define TX_TUNE_MAN 3 // Transmitter keyed manually
#define TX_TUNE_CAT 4 // Transmitter keyed manually

class Cat_communicator
{
  public:
	virtual int Read(char c, std::string &s) = 0;
	virtual void Send(std::string s) = 0;
	virtual bool available() = 0;
	virtual void SendInformation(int info) = 0;
	virtual bool IsCommuncationPortOpen() = 0;
	virtual ~Cat_communicator(){} ;
};

class FT891_CAT // Class name
{
  protected:
	Cat_communicator *catcommunicator_;

  public:
	/*
 *	Function prototypes:
 *
 *	These are available to the outside world:
 */

	explicit FT891_CAT(); // Constructor

	void begin(bool debug = false, Cat_communicator *cat_communicator = nullptr, bool bmode = true); // Initialize CAT control parameters

	int CheckCAT(bool bwait = true); // See if anything to do

	void SetFA(uint32_t freq);  // Set VFO-A frequency
	void SetFB(uint32_t freq);  // Set VFO-B frequency
	void SetMDA(uint8_t mode);  // Set VFO-A mode
	void SetMDB(uint8_t mode);  // Set VFO-B mode
	void SetTX(uint8_t tx);		// Set transmit/receive status
	void SetST(uint8_t st);		// Set split mode
	void SetBand(uint16_t bnd); // Set band in meters (for band filters)
	void SetFT(int ft);
	void SetAG(uint8_t ag);
	void SetRG(uint8_t rg);
	void SetSH(int status, int bandwidth);
	void SetSM(uint8_t sm);
	void SetIG(uint8_t ig);
	void SetAI(uint16_t ai);
	void SetNA(uint8_t na);
	void SetEX(char *buf);
	void SetRT(uint8_t rt);
	void SetRD(uint32_t rd);

	uint32_t GetFA();	// Get VFO-A frequency
	uint32_t GetFB();   // Get VFO-B frequency
	uint8_t GetMDA();   // Get VFO-A mode
	uint8_t GetMDB();   // Get VFO-B mode
	uint8_t GetTX();	// Get transmit/receive status
	bool GetST();		// Get Split mode
	uint16_t GetBand(); // Get selected band in meters
	int GetFT();
	uint8_t GetAG();
	uint8_t GetRG();
	int GetSH();
	uint8_t GetIG();
	uint8_t GetNA();
	uint8_t GetRT();
	uint32_t GetRD();

	void SendInformation(int info);
	void SendCatMessage(int fd, std::string message);
	int OpenCatChannel();

/*
 *	These can't be called from the outside world:
 */

  private:
	void set_cat_communicator(Cat_communicator *cat_communicator) { this->catcommunicator_ = cat_communicator; };
	void Init(bool debug);			   // Common logic for multiple 'begin' functions
	int GetMessage(bool bwait = true); // Get a message from the input stream
	msg FindMsg();					   // Find the message in "msgTable"
	bool ParseMsg();				   // Separate any data from the message
	bool ProcessCmd();				   // Process a command type message
	void ProcessStatus();			   // Process a status request
	unsigned xtoi(char *hexString);	// Convert a hexadecimal string to integer
		
/*
 *	All the internal data is private:
 */

  private:
	char rxBuff[BUF_LEN];			 // DxCommander receive buffer
	char txBuff[BUF_COUNT][BUF_LEN]; // DxCommander transmit buffers
	char dataBuff[BUF_LEN];			 // Data part of a message - slightly bigger than needed
	bool hasData = false;			 // Indicator that a message included some data
	bool bVFOmode = true;			 // Indicator if class is used in remot control or VFO
	uint8_t txBuffIndex = 0;		 // Next txBuff to be used

	uint8_t MSG_COUNT; // Number of messages in the list

	msg newMessage; // Use the structure for message being processed
	std::vector<std::stringstream> inputStreams;
};

#endif
