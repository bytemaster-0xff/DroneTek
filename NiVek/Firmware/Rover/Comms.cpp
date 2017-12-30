#include "NiVekMessageParser.h"
#include "NiVekMessageScribe.h"
#include "arduino.h"
#include "GPIO.h"
#include "System.h"
#include "Drive.h"
#include "Turret.h"
#include "Sensors.h"
#include "Comms.h"
#include "Compass.h"

char *OutBufffer;

extern uint8_t point;
extern uint8_t s[128];

#define MODULE_SENSOR	70
#define MODULE_GPIO		80
#define MODULE_SYSTEM   100
#define MODULE_MOTORS   110
#define MODULE_TURRET   120

#define SERIAL_COM_SPEED 115200
#define SOH 0x01
#define STX 0x02
#define ETX 0x03
#define EOT 0x04

#define ACK 0x06
#define NAK 0x15

#define BROADCAST_ADDR 0xFF

#define OUT_SENSOR_UPDATE      0x80

#define BlueTooth Serial

#define LocalAddress 20
#define null 0x00

NiVekLogger Logger;

NiVekMessageScribe ToRadio; 

//NiVekMessageParser RadioMessageParser(&Logger, &ToRadio);
NiVekMessageParser SerialMessageParser(&Logger, &ToRadio);

extern void handleFlightMessage(NiVekMessage *msg);
extern void handleSensorMessage(NiVekMessage *msg);

#define SYSCONFIG_SIZE sizeof(eep_entry)/sizeof(eep_entry_t)

char OutputBuffer[OUTPUT_BUFFER_SIZE];

uint8_t msg_buffer[256];

void WriteOutput() {
	OutputDebugStringA(OutputBuffer); // for VS Output
	printf(OutputBuffer); // for commandline output
}

void CommitMessage(NiVekMessage *msg){
	uint8_t idx = 0;
	msg_buffer[idx++] = (uint8_t) SOH;
	msg_buffer[idx++] = msg->ExpectACK;
	msg_buffer[idx++] = msg->SourceAddress;
	msg_buffer[idx++] = msg->DestAddress;
	msg_buffer[idx++] = msg->SystemId;
	msg_buffer[idx++] = msg->TypeId;
	msg_buffer[idx++] = (uint8_t) (msg->SerialNumber >> 8);
	msg_buffer[idx++] = (uint8_t) (msg->SerialNumber & 0xFF);
	msg_buffer[idx++] = (uint8_t) (msg->MsgSize >> 8);
	msg_buffer[idx++] = (uint8_t) (msg->MsgSize & 0xFF);
	msg_buffer[idx++] = (uint8_t) STX;

	for (uint8_t payloadIndex = 0; payloadIndex < msg->MsgSize; ++payloadIndex)
		msg_buffer[idx++] = (uint8_t) msg->MsgBuffer[payloadIndex];

	msg_buffer[idx++] = (uint8_t) ETX;
	msg_buffer[idx++] = msg->CheckSum;
	msg_buffer[idx++] = (uint8_t) EOT;

	size_t bytesWritten = Serial.write(msg_buffer, idx);
	printf(OutputBuffer);
}


void handleMessage(NiVekMessage *message){
	CustomLogging("GOT MESSAGE!\r\n");

	if (message->DestAddress == LocalAddress || message->DestAddress == 0xFF)
	{
		switch (message->SystemId){
		case MODULE_SYSTEM: HandleSystemMessage(message); break;
		case MODULE_GPIO: HandleGPIOMessage(message); break;
		case MODULE_SENSOR: HandleSensorMessage(message); break;
		case MODULE_MOTORS: HandleMotorMessage(message); break;
		case MODULE_TURRET: HandleTurretMessge(message); break;
		}
	}
}

byte message [] = { 0x73, 0x00, 0x0A, 0x01, 0x01, 0x50, 0x01, 0x00, 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0xB8 };

void SendTelemetry(){
	/* Building message takes 35 uSeconds*/
	NiVekMessage *msg = ToRadio.beginMessage(0x00, LocalAddress, BROADCAST_ADDR, MODULE_SENSOR, OUT_SENSOR_UPDATE);

	msg->serialize16(_currentPan); //20
	msg->serialize16(_currentTilt); //20

	msg->serialize16(_frontSonar); //20
	msg->serialize16(_rearSonar); //20
	msg->serialize16(_currentLeft); 
	msg->serialize16(_currentRight); 

	msg->serialize16((uint16_t)CurrentHeading);

	ToRadio.endMessage();
}

void sendAck(NiVekMessage *receivedMessage) {
	NiVekMessage *msg = ToRadio.beginMessage(0x00, LocalAddress, receivedMessage->DestAddress, MODULE_SYSTEM, ACK);
	msg->serialize8((receivedMessage->SerialNumber >> 8) & 0xFF);
	msg->serialize8(receivedMessage->SerialNumber & 0xFF);
	ToRadio.endMessage();
}

void sendNak(NiVekMessage *receivedMessage) {
	NiVekMessage *msg = ToRadio.beginMessage(0x00, LocalAddress, receivedMessage->DestAddress, MODULE_SYSTEM, NAK);
	msg->serialize8((receivedMessage->SerialNumber >> 8) & 0xFF);
	msg->serialize8(receivedMessage->SerialNumber & 0xFF);
	ToRadio.endMessage();
}

void commsHandler() {
	while (BlueTooth.available()){
		int ch = BlueTooth.read();

		SerialMessageParser.handleCh(ch);
		if (SerialMessageParser.available())
			handleMessage(SerialMessageParser.currentMessage());
	}

	NiVekMessage *msg = ToRadio.getNextToSend();
	if (msg != null)
	{
		CustomLogging("SENT MESSAGE!\r\n");
		CommitMessage(msg);		
	}
}

char outMsgBuffer[128];

void initComms() {
	BlueTooth.begin(SERIAL_COM_SPEED);
}