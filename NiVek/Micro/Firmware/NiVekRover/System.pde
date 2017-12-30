
#define PING 100
#define WELCOME_PING 101
#define PONG 110


void sendPong(NiVekMessage *receivedMessage) {
	NiVekMessage *msg = ToRadio.beginMessage(0x00, LocalAddress, receivedMessage->DestAddress, MODULE_SYSTEM, PONG);

	CommitMessage(msg);
}


void HandleSystemMessage(NiVekMessage* msg){
	Logger.println("ping");
	switch (msg->TypeId)
	{
		case PING: sendPong(msg); break;
	}
}