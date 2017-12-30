
#define RC_INPUT 100;
#define USE_WII_CONTROLLER 200
#define USE_PC_CONTROLLER 201

#define MODULE_SENSOR	70
#define MODULE_GPIO		80
#define MODULE_WIFI		90
#define MODULE_SYSTEM	100
#define MODULE_VEST	110
#define MODULE_PROXY	200

#define SET_VEST_ALL_ON 101
#define SET_VEST_ALL_OFF 102
#define SET_VEST_SET_POWER 100

void sendAck(NiVekMessage *receivedMessage) {
	NiVekMessage *msg = ToRadio.beginMessage(0x00, LocalAddress, receivedMessage->DestAddress, MODULE_SYSTEM, ACK);
	msg->serialize8((receivedMessage->SerialNumber >> 8) & 0xFF);
	msg->serialize8(receivedMessage->SerialNumber & 0xFF);

	SendMessageToController(msg);
}

void sendNak(NiVekMessage *receivedMessage) {
	NiVekMessage *msg = ToRadio.beginMessage(0x00, LocalAddress, receivedMessage->DestAddress, MODULE_SYSTEM, NAK);
	msg->serialize8((receivedMessage->SerialNumber >> 8) & 0xFF);
	msg->serialize8(receivedMessage->SerialNumber & 0xFF);

	SendMessageToController(msg);
}


void HandleMessage(NiVekMessage *msg, bool fromRadio){
	setLEDState(LEDOnline, LEDOn);
	if (msg->SystemId == MODULE_VEST)
	{
		switch (msg->TypeId){
			case SET_VEST_SET_POWER:{
				uint8_t idx = msg->MsgBuffer[0];
				uint16_t value = msg->MsgBuffer[2];
				value |= msg->MsgBuffer[1] << 8;
				vestPower(idx, value);
				sendAck(msg);
			}
				break;
			case SET_VEST_ALL_ON:{
				vestAllOn();
				sendAck(msg);
			}
				break;

			case SET_VEST_ALL_OFF:{
				vestAllOff();
				sendAck(msg);
			}
			break;
		}
	}

	setLEDState(LEDOnline, LEDOff);


	/*Serial.write("GOT A MESSAGE!");

	if (message->DestAddress == 10){
	switch (message->TypeId)
	{
	case USE_PC_CONTROLLER:
	pcr_rcChan[0] = 1250 + (int8_t) message->MsgBuffer[0] * 3;
	pcr_rcChan[1] = 1500 + (int8_t) message->MsgBuffer[1] * 3;

	pcr_rcChan[2] = 1500 + (int8_t)message->MsgBuffer[2] * 3;
	pcr_rcChan[3] = 1500 + (int8_t)message->MsgBuffer[3] * 3;
	pcr_rcChan[4] = 1500 + message->MsgBuffer[4];

	break;
	}
	}*/

}

void SendMessageToController(NiVekMessage *msg){
	uint8_t idx;
	BluetoothComms.write(SOH);
	BluetoothComms.write(msg->ExpectACK);
	BluetoothComms.write(msg->SourceAddress);
	BluetoothComms.write(msg->DestAddress);
	BluetoothComms.write(msg->SystemId);
	BluetoothComms.write(msg->TypeId);
	BluetoothComms.write(msg->SerialNumber >> 8);
	BluetoothComms.write(msg->SerialNumber & 0xFF);
	BluetoothComms.write(msg->MsgSize >> 8);
	BluetoothComms.write(msg->MsgSize & 0xFF);

	BluetoothComms.write(STX);

	for (idx = 0; idx < msg->MsgSize; ++idx)
		BluetoothComms.write(msg->MsgBuffer[idx]);

	BluetoothComms.write(ETX);
	BluetoothComms.write(msg->CheckSum);
	BluetoothComms.write(EOT);
}
