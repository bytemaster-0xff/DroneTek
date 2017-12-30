#define SEND_CONFIG 90


void HandleGPIOMessage(NiVekMessage *msg){

	
	Logger.println(msg->TypeId);
	switch (msg->TypeId)
	{
		case 90: 
			sendPong(msg); 
			break;
		case 100:
			Logger.println("UP");
			break;
		case 200:
			SetTurretPanTilt(msg->MsgBuffer[0], msg->MsgBuffer[1]);
			break;
	}

	if (msg->ExpectACK)
		sendAck(msg);
}