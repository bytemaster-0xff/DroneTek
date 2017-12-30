#include <ZigduinoRadioCfg.h>
#include <transceiver.h>
#include <radio.h>
#include <PicopterRadio.h>
#include <const.h>
#include <board_cfg.h>
#include <board.h>
#include <atmega_rfa1.h>
#include <NiVekMessageScribe.h>
#include <NiVekMessageParser.h>
#include <NiVekMessage.h>
#include <PicopterWiiClassicCtrler.h>
#include <avr/wdt.h>
#include "def.h"

//#define USE_ACCUM_THROTTLE
//#define DEADBAND (128/8)

unsigned long lastMicros;
char heatBeatCnt;
char isArmed = 0;
uint16_t accumThrottle = 0;

uint8_t LocalAddress;

NiVekMessageScribe ToComputer;
NiVekMessageScribe ToRadio;

NiVekMessageParser FromComputerParser(&Serial1, &ToComputer);
NiVekMessageParser FromRadioParser(&Serial, &ToRadio);
  
#define BluetoothComms Serial1
#define GPSComms Serial
#define DebugWriter Serial

void setup(){
	// this sets up all the LEDs
  
	//Serial.begin(38400);
	//Serial1.begin(115200);

	BluetoothComms.begin(57600);

	DebugWriter.println("Starting up.");
	DebugWriter.begin(115200);

	LocalAddress = 10;

	PicopterRadio.beginAsCoordinator(0, 0, PCR_CTRLER_DEVADDR);

	//wcc_init();

	BATMON = 0x13;
  
	dual_stick_info_t data;

	initLEDS();

	/*if (!wcc_read(&data))
		isp_setup();*/
  
	lastMicros = micros();

	initOutput();
	
	setLEDState(LEDOnline, LEDOn);

	DebugWriter.println("Start up completed.");
}

void CheckOK(HardwareSerial port)
{
	char a, b;
	while (1)
	{
		if (port.available())
		{
			a = port.read();

			if ('O' == a)
			{
				// Wait for next character K. available() is required in some cases, as K is not immediately available.
				while (port.available())
				{
					b = port.read();
					break;
				}
				if ('K' == b)
				{
					break;
				}
			}
		}
	}

	while ((a = port.read()) != -1)
	{
		//Wait until all other response chars are received
	}
}

void sendBlueToothCommand(HardwareSerial port, char command[])
{
	port.print(command);
	CheckOK(port);
}


void setupBluetooth(HardwareSerial port){
	port.begin(57600); //Set BluetoothBee BaudRate to default baud rate 38400
}


void calibrateJoyStickes(){

	Serial1.print(pcr_rcChan[0]);
	Serial1.print(", ");
	Serial1.print(pcr_rcChan[1]);
	Serial1.print(", ");
	Serial1.print(pcr_rcChan[2]);
	Serial1.print(", ");
	Serial1.print(pcr_rcChan[3]);
	Serial1.print(", 0x");
	Serial1.print(pcr_rcChan[4], HEX);
	Serial1.print(", 0x");
	Serial1.print(pcr_rcChan[5], HEX);
	Serial1.println();

	wcc_initCalibrate();

	for (int i = 0; i < 5; i++)
	{
		delay(100);
		delay(100);
	}
}

void noJoystickData(){
	//PCS_LED2_TOG();
	//delay(50);
}

void controlLoop(){
	dual_stick_info_t data;
	if (wcc_read(&data)){
		pcr_rcChan[0] = 1500 + data.rx * 3;
		pcr_rcChan[1] = 1500 + data.ry * 3;

		pcr_rcChan[2] = 1500 + data.lx * 3;
		pcr_rcChan[3] = 1500 + data.ly * 3;

		pcr_rcChan[4] = data.buttons;
		pcr_rcChan[5] = pcr_rcChan[4] ^ 0xAA55;

		if (data.buttons & (1 << PCC_WIICALI))	calibrateJoyStickes();
		else flightControls(data);
	}
	else
		noJoystickData();
}

void waitLoop(){
	if (PicopterRadio.isPresent())
		setLEDState(LEDOnline, LEDOn);
	else
		setLEDState(LEDOnline, LEDOff);
}

void checkBattery(){
	if ((BATMON & (1 << BATMON_OK)) == 0)
	{
		// freeze code forever and blink LEDs
		while (1)
		{
			setLEDState(LEDFailure, LEDOff);
			delay(50);
			setLEDState(LEDFailure, LEDOn);
			delay(500);
		}
	}
}

typedef enum ISPStates {
	Idle,
	ReadI,
	ReadS,
} ISPStates_t;

ISPStates_t _ispStartState = Idle;

void commo(){
	if (BluetoothComms.available()) {
		while (BluetoothComms.available()) {
			char ch = (char) BluetoothComms.read();
			setLEDState(LEDFailure, LEDOn);
			FromComputerParser.handleCh(ch);
			setLEDState(LEDFailure, LEDOff);
			if (FromComputerParser.available()){				

				NiVekMessage *msg = FromComputerParser.currentMessage();

				DebugWriter.print("Mesage From Computer Received.");
				DebugWriter.print(" ");
				DebugWriter.print((long) msg->DestAddress);
				DebugWriter.print(" ");
				DebugWriter.print((long)msg->SystemId);
				DebugWriter.print(" ");
				DebugWriter.println((long) msg->TypeId);


				if (msg->DestAddress == LocalAddress)
					HandleMessage(msg, false);
				else
					PicopterRadio.commit(msg);				
			}
		}

		heatBeatCnt = 0;
	}
	if (PicopterRadio.available()) {
		while (PicopterRadio.available()) {
			setLEDState(LEDComms, LEDOn);
			char ch = (char) PicopterRadio.read();
			BluetoothComms.write(ch);
			setLEDState(LEDComms, LEDOff);
			FromRadioParser.handleCh(ch);
			if (FromRadioParser.available())
				HandleMessage(FromRadioParser.currentMessage(), true);
		}

		heatBeatCnt = 0;
	}
}

uint8_t isSet = 0;

void flightControls(dual_stick_info_t data) {
	PicopterRadio.sendFlightCommands(0xFF);
	PicopterRadio.setRxMode();

	heatBeatCnt++;
}

static uint32_t __oneHzLoop = 0;

bool is1HzLoop(uint32_t currentTime) {
	if (currentTime > __oneHzLoop){
		__oneHzLoop = currentTime + 1000000;
		return true;
	}

	return false;
}

#define BROADCAST_ADDR 0xFF
#define SYSTEM_MODULE_ID 100
#define PING_ID 100

void loop(){
  unsigned long curMicros = micros();

  if (is1HzLoop(curMicros))
  {
	  uint8_t idx;

	  NiVekMessage *msg = ToRadio.beginMessage(0x00, 10, BROADCAST_ADDR, SYSTEM_MODULE_ID, PING_ID);
	  SendMessageToController(msg);
  }
	  

  if ((curMicros - lastMicros) > (unsigned long)(20 * 1000)){
	  
	  PicopterRadio.sendFlightCommands(0xFF);
	  PicopterRadio.setRxMode();

	  heatBeatCnt++;


	  //BluetoothComms.print("A");

	  //controlLoop();
	  lastMicros = curMicros;
  }
/*  else
	  waitLoop();*/
  
  checkBattery();

  writeMotors();

  commo();
}
