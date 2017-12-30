
#include "def.h"
#include "AppData.h"

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

NiVekMessageScribe ToRadio;
NiVekMessageScribe ToSerialPort;

NiVekMessageParser RadioMessageParser(&Serial, &ToRadio);
NiVekMessageParser SerialMessageParser(&Serial, null);

extern void handleFlightMessage(NiVekMessage *msg);
extern void handleSensorMessage(NiVekMessage *msg);

#define SYSCONFIG_SIZE sizeof(eep_entry)/sizeof(eep_entry_t)

void handleMessage(NiVekMessage *message){
	//if (message->SystemId != 100 && message->TypeId != 100){
		/*Serial.print("MESSAGE ");
		Serial.print((int) message->SourceAddress);
		Serial.print(" ");
		Serial.print((int) LocalAddress);
		Serial.print(" ");
		Serial.print((int) message->SystemId);
		Serial.print(" ");
		Serial.println((int) message->TypeId);*/
	//}device out

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

void sendBasicTelemetry(){
	/* Building message takes 35 uSeconds*/
	NiVekMessage *msg = ToRadio.beginMessage(0x00, LocalAddress, BROADCAST_ADDR, MODULE_SENSOR, OUT_SENSOR_UPDATE);

/*	msg->serialize16(att.angle[PITCH]); //20
	msg->serialize16(att.angle[ROLL]); //20

	msg->serialize16(0);
	msg->serialize16(alt.EstAlt);*/


//	PicopterRadio.commit(msg);
}

void sendAck(NiVekMessage *receivedMessage) {
	NiVekMessage *msg = ToRadio.beginMessage(0x00, LocalAddress, receivedMessage->DestAddress, MODULE_SYSTEM, ACK);
	msg->serialize8((receivedMessage->SerialNumber >> 8) & 0xFF);
	msg->serialize8(receivedMessage->SerialNumber & 0xFF);

	CommitMessage(msg);
}

void sendNak(NiVekMessage *receivedMessage) {
	NiVekMessage *msg = ToRadio.beginMessage(0x00, LocalAddress, receivedMessage->DestAddress, MODULE_SYSTEM, NAK);
	msg->serialize8((receivedMessage->SerialNumber >> 8) & 0xFF);
	msg->serialize8(receivedMessage->SerialNumber & 0xFF);
	CommitMessage(msg);
}

void commsHandler() {
	while (BlueTooth.available()){
		int ch = BlueTooth.read();
		SerialMessageParser.handleCh(ch); 
		if (SerialMessageParser.available())
			handleMessage(SerialMessageParser.currentMessage());
	}
}

char outMsgBuffer[128];

void CommitMessage(NiVekMessage *msg){
	uint8_t idx;
	BlueTooth.write(SOH);
	BlueTooth.write(msg->ExpectACK);
	BlueTooth.write(msg->SourceAddress);
	BlueTooth.write(msg->DestAddress);
	BlueTooth.write(msg->SystemId);
	BlueTooth.write(msg->TypeId);
	BlueTooth.write(msg->SerialNumber >> 8);
	BlueTooth.write(msg->SerialNumber & 0xFF);
	BlueTooth.write(msg->MsgSize >> 8);
	BlueTooth.write(msg->MsgSize & 0xFF);

	BlueTooth.write(STX);

	for (idx = 0; idx < msg->MsgSize; ++idx)
		BlueTooth.write(msg->MsgBuffer[idx]);

	BlueTooth.write(ETX);
	BlueTooth.write(msg->CheckSum);
	BlueTooth.write(EOT);
}

void initComms() {
	Logger.begin(SERIAL_COM_SPEED);
	BlueTooth.begin(SERIAL_COM_SPEED);
}