#include <NivekMessage.h>

#define MODULE_SENSOR	70
#define MODULE_GPIO		80
#define MODULE_WIFI		90
#define MODULE_SYSTEM	100
#define MODULE_PROXY	200

#define OUT_SENSOR_UPDATE      0x80
#define OUT_SYSTEM_CONFIG      0x21
#define OUT_SYSTEM_STATUS	   0x13
#define OUT_SYSTEM_TARGETS	   0x14
#define OUT_MOTOR_STATUS	   0x16
#define OUT_RC_CHANNEL			136 

#define OUT_WII_PID 0x21

#define ACK 0x06
#define NAK 0x15

#define BROADCAST_ADDR 0xFF

extern uint8_t point;
extern uint8_t s[128];

NiVekMessageScribe ToRadio;
NiVekMessageScribe ToSerialPort;

NiVekMessageParser RadioMessageParser(&Serial, &ToRadio);
NiVekMessageParser SerialMessageParser(&Serial, null);

extern void handleFlightMessage(NiVekMessage *msg);
extern void handleSensorMessage(NiVekMessage *msg);

#define SYSCONFIG_SIZE sizeof(eep_entry)/sizeof(eep_entry_t)

void handleMessage(NiVekMessage *message){
	if (message->SystemId != 100 && message->TypeId != 100){
		Serial.print("MESSAGE ");
		Serial.print((int) message->SourceAddress);
		Serial.print(" ");
		Serial.print((int) LocalAddress);
		Serial.print(" ");
		Serial.print((int) message->SystemId);
		Serial.print(" ");
		Serial.println((int) message->TypeId);
	}

	if (message->DestAddress == LocalAddress || message->DestAddress == 0xFF)
	{
		switch (message->SystemId){
			case MODULE_SYSTEM: handleSystemMessage(message); break;
			case MODULE_GPIO: handleFlightMessage(message); break;
			case MODULE_SENSOR: handleSensorMessage(message); break;
		}
	}
}

byte message [] = { 0x73, 0x00, 0x0A, 0x01, 0x01, 0x50, 0x01, 0x00, 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0xB8};

void sendBasicTelemetry(){
	/* Building message takes 35 uSeconds*/
	NiVekMessage *msg = ToRadio.beginMessage(0x00, LocalAddress, BROADCAST_ADDR, MODULE_SENSOR, OUT_SENSOR_UPDATE);
	
	msg->serialize16(att.angle[PITCH]); //20
	msg->serialize16(att.angle[ROLL]); //20

	msg->serialize16(0);
	msg->serialize16(alt.EstAlt);

	PicopterRadio.commit(msg);
}

void sendMotorStatus() {
	uint8_t i;

	/* Building message takes 35 uSeconds*/
	NiVekMessage *msg = ToRadio.beginMessage(0x00, LocalAddress, BROADCAST_ADDR, MODULE_SYSTEM, OUT_MOTOR_STATUS);

	for (i = 0; i < 4; i++)
		msg->serialize8((uint8_t) (((uint32_t) motor[i] - MINTHROTTLE) * 255 / (MAXTHROTTLE - MINTHROTTLE)));

	msg->serialize16(0);
	msg->serialize16(0);

	PicopterRadio.commit(msg);
}

void sendRCChannel() {
	NiVekMessage *msg = ToRadio.beginMessage(0x00, LocalAddress, BROADCAST_ADDR, MODULE_SYSTEM, OUT_SYSTEM_TARGETS);

	msg->serialize16(0); // Heading
	msg->serialize16(0); // Altitude

	msg->serialize8(0); // Power/Throttle

	msg->serialize8((int8_t) (rcCommand[PITCH] / 10));
	msg->serialize8((int8_t) (rcCommand[ROLL] / 10));
	msg->serialize8((int8_t) (rcCommand[YAW] / 10));

	PicopterRadio.commit(msg);
}

void sendStatus() {
	NiVekMessage *msg = ToRadio.beginMessage(0x00, LocalAddress, BROADCAST_ADDR, MODULE_SYSTEM, OUT_SYSTEM_STATUS);
	msg->serialize16(VERSION); /* Firmware Version */
	msg->serialize8(PlatformType); /* Platform Version */
	msg->serialize8(SystemState); /* System State */
	msg->serialize8(SensorState); /* Sensor State */
	msg->serialize8(GPIOState); /* GPIO State */
	msg->serialize8(IsArmed); /* Armed Status */

	msg->serialize8(ControlMethod); /* Control Method */
	msg->serialize8(AltitudeHold); /* Altitude Hold */

	msg->serialize8(FrameConfig); /* Frame Config */

	msg->serialize8(FlightMode); /* Armed Status */

	PicopterRadio.commit(msg);
}

void sendPID(NiVekMessage *requestorMessage){
	uint8_t i;

	NiVekMessage *msg = ToRadio.beginMessage(0xFF, LocalAddress, requestorMessage->SourceAddress, MODULE_GPIO, OUT_WII_PID);

	for (i = 0; i < 3; i++) {
		msg->serialize8(P8[i]);
		msg->serialize8(I8[i]);
		msg->serialize8(D8[i]);
	}

	msg->serialize8(P8[PIDLEVEL]);
	msg->serialize8(I8[PIDLEVEL]);
	msg->serialize8(0);

	PicopterRadio.commit(msg);
}

void sendAck(NiVekMessage *receivedMessage) {
	NiVekMessage *msg = ToRadio.beginMessage(0x00, LocalAddress, receivedMessage->DestAddress, MODULE_SYSTEM, ACK);
	msg->serialize8((receivedMessage->SerialNumber >> 8) & 0xFF);
	msg->serialize8(receivedMessage->SerialNumber & 0xFF);

	PicopterRadio.commit(msg);
}

void sendNak(NiVekMessage *receivedMessage) {
	NiVekMessage *msg = ToRadio.beginMessage(0x00, LocalAddress, receivedMessage->DestAddress, MODULE_SYSTEM, NAK);
	msg->serialize8((receivedMessage->SerialNumber >> 8) & 0xFF);
	msg->serialize8(receivedMessage->SerialNumber & 0xFF);

	PicopterRadio.commit(msg);
}

void commsHandler() {
	while (Serial.available()){
		SerialMessageParser.handleCh(Serial.read());
		if (SerialMessageParser.available())
			handleMessage(SerialMessageParser.currentMessage());
	}

	while (PicopterRadio.available()){
		uint8_t ch = PicopterRadio.read();
		RadioMessageParser.handleCh(ch);
		if (RadioMessageParser.available())
			handleMessage(RadioMessageParser.currentMessage());
	}
}

void initComms() {
	Serial.begin(SERIAL_COM_SPEED);

	initReceiver();
}