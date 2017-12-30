
typedef struct sys_config_t{
	void *  var;
	uint8_t size;
};

static uint8_t sysConfigVersion = 12;

static sys_config_t sysconfig[] = {
	&sysConfigVersion, sizeof(sysConfigVersion),
	&LocalAddress, sizeof(LocalAddress),
	&P8, sizeof(P8),
	&I8, sizeof(I8),
	&D8, sizeof(D8),
	&rcRate8, sizeof(rcRate8),
	&rcExpo8, sizeof(rcExpo8),
	&rollPitchRate, sizeof(rollPitchRate),
	&yawRate, sizeof(yawRate),
	&dynThrPID, sizeof(dynThrPID),
	&accTrim, sizeof(accTrim),
	&GyroAccCompFilter, sizeof(GyroAccCompFilter),
	&GyroMagCompFilter, sizeof(GyroMagCompFilter)
};

#define SYSCONFIG_SIZE sizeof(sysconfig)/sizeof(sys_config_t)

#define MSG_OUTGOING_MSG_NAME 200

#define MSG_Ping 100

#define MSG_SetName 200
#define MSG_GetName 201

bool getOkToArm(){
	//return NiVekRadio.isPresent();
	return true;
}

void sendConfiguration(uint8_t destAddress) {
	Serial.println("Sending Configuration");

	NiVekMessage *msg = ToRadio.beginMessage(0x00, LocalAddress, destAddress, MODULE_SYSTEM, OUT_SYSTEM_CONFIG);

	uint8_t i, _address = 0;
	for (i = 0; i < SYSCONFIG_SIZE; i++) 
		msg->serialize((uint8_t *)sysconfig[i].var, sysconfig[i].size);

	PicopterRadio.commit(msg);
}

void sendName(uint8_t destAddress){
	Serial.println("Sending Name");

	uint8_t idx;
	NiVekMessage *msg = ToRadio.beginMessage(0x00, LocalAddress, destAddress, MODULE_SYSTEM, MSG_OUTGOING_MSG_NAME);

	msg->serialize((uint8_t*) DroneName, 32);// sizeof(DroneName));

	PicopterRadio.commit(msg);
}

void setName(NiVekMessage *msg){
	Serial.print("Updating drone name: ");

	memcpy(DroneName, msg->MsgBuffer, sizeof(DroneName));

	Serial.println(DroneName);

	writeParams();

	sendAck(msg);
}


void handleSystemMessage(NiVekMessage *msg){

	switch (msg->TypeId){
		case MSG_GetName: sendName(msg->SourceAddress); break;
		case MSG_SetName: setName(msg); break;
		case MSG_Ping: 
			break;
	}
}

void initSys(){
	Serial.println("Initialize System");

	readEEPROM();
	checkFirstTime();

	SystemState = SysReady;
}