
// **************************************************
// Simplified IMU based on "Complementary Filter"
// Inspired by http://starlino.com/imu_guide.html
//
// adapted by ziss_dm : http://wbb.multiwii.com/viewtopic.php?f=8&t=198
//
// The following ideas was used in this project:
// 1) Rotation matrix: http://en.wikipedia.org/wiki/Rotation_matrix
// 2) Small-angle approximation: http://en.wikipedia.org/wiki/Small-angle_approximation
// 3) C. Hastings approximation for atan2()
// 4) Optimization tricks: http://www.hackersdelight.org/
//
// Currently Magnetometer uses separate CF which is used only
// for heading approximation.
//
// Modified: 19/04/2011  by ziss_dm
// Version: V1.1
//
// code size deduction and tmp vector intermediate step for vector rotation computation: October 2011 by Alex
// **************************************************

//******  advanced users settings *******************
/* Set the Low Pass Filter factor for ACC */
/* Increasing this value would reduce ACC noise (visible in GUI), but would increase ACC lag time*/
/* Comment this if  you do not want filter at all.*/
/* Default WMC value: 8*/
#define ACC_LPF_FACTOR 8

/* Set the Low Pass Filter factor for Magnetometer */
/* Increasing this value would reduce Magnetometer noise (not visible in GUI), but would increase Magnetometer lag time*/
/* Comment this if  you do not want filter at all.*/
/* Default WMC value: n/a*/
//#define MG_LPF_FACTOR 4

/* Set the Gyro Weight for Gyro/Acc complementary filter */
/* Increasing this value would reduce and delay Acc influence on the output of the filter*/
/* Default WMC value: 300*/
//#define GYR_CMPF_FACTOR 310.0f

/* Set the Gyro Weight for Gyro/Magnetometer complementary filter */
/* Increasing this value would reduce and delay Magnetometer influence on the output of the filter*/
/* Default WMC value: n/a*/
//#define GYR_CMPFM_FACTOR 200.0f

//****** end of advanced users settings *************

//#define INV_GYR_CMPF_FACTOR   (1.0f / (GYR_CMPF_FACTOR  + 1.0f))
//#define INV_GYR_CMPFM_FACTOR  (1.0f / (GYR_CMPFM_FACTOR + 1.0f))
#if GYRO
  #define GYRO_SCALE ((2380 * PI)/((32767.0f / 4.0f ) * 180.0f * 1000000.0f)) //should be 2279.44 but 2380 gives better result
  // +-2000/sec deg scale
  //#define GYRO_SCALE ((200.0f * PI)/((32768.0f / 5.0f / 4.0f ) * 180.0f * 1000000.0f) * 1.5f)     
  // +- 200/sec deg scale
  // 1.5 is emperical, not sure what it means
  // should be in rad/sec
#else
  #define GYRO_SCALE (1.0f/200e6f)
  // empirical, depends on WMP on IDG datasheet, tied of deg/ms sensibility
  // !!!!should be adjusted to the rad/sec
#endif 
// Small angle approximation


/* Pulled from EEPROM */
/* Set the Gyro Weight for Gyro/Acc complementary filter */
/* Increasing this value would reduce and delay Acc influence on the output of the filter*/
/* Default WMC value: 300*/
static float __gyro_acc_factor;
static float __acc_gyro_factor;

imu_t imu;
att_t att;
alt_t alt;


void updateCompFilterValues(){
	__gyro_acc_factor = (float) GyroAccCompFilter;
	__acc_gyro_factor = (1.0f / (__gyro_acc_factor + 1));
}

void updateCompFilterSetting(NiVekMessage *msg){
	SensorIds_t sensor = (SensorIds_t) msg->MsgBuffer[0];

	Serial.write("COMP -");

	switch (sensor){
	case SensorIds_IMU_Attitude:{
									Serial.println("ATT");
									uint16_t newValue = msg->MsgBuffer[2] << 8;
									newValue |= msg->MsgBuffer[3];
									GyroAccCompFilter = newValue;
									writeParams();
									sendAck(msg);

									__gyro_acc_factor = (float) GyroAccCompFilter;
									__acc_gyro_factor = (1.0f / (__gyro_acc_factor + 1));
	}

		break;
	case SensorIds_IMU_Heading:{
								   Serial.println("HEAD");
								   uint16_t newValue = msg->MsgBuffer[2] << 8;
								   newValue |= msg->MsgBuffer[3];
								   GyroMagCompFilter = newValue;
								   writeParams();
								   sendAck(msg);
	}
		break;
	case SensorIds_IMU_Altitude:{
									Serial.println("ALT");
									uint16_t newValue = msg->MsgBuffer[2] << 8;
									newValue |= msg->MsgBuffer[3];
									AccBaroCompFilter = newValue;
									writeParams();
									sendAck(msg);
	}
		break;
	}
}

#define MSG_SensorFusionConfig 0x34

void sendCompFilterConfig(NiVekMessage *incomingMsg) {
	NiVekMessage *msg = ToRadio.beginMessage(0x00, LocalAddress, incomingMsg->SourceAddress, MODULE_SENSOR, MSG_SensorFusionConfig);

	msg->serialize16(GyroAccCompFilter); //20
	msg->serialize16(GyroMagCompFilter); //20
	msg->serialize16(AccBaroCompFilter);

	PicopterRadio.commit(msg);

	Serial.write("Sending Comp Filter Values");
}

void getEstimatedAttitude(){
	uint8_t axis;
	int16_t accMag = 0;
	static t_fp_vector EstG,EstM;
	static int16_t mgSmooth[3],accTemp[3];  //projection of smoothed and normalized magnetic vector on x/y/z axis, as measured by magnetometer
	static uint16_t previousT;
	uint16_t currentT = micros();
	float scale, deltaGyroAngle[3];

	scale = (currentT - previousT) * GYRO_SCALE;
	previousT = currentT;

	// Initialization
	for (axis = 0; axis < 3; axis++) {
		deltaGyroAngle[axis] = imu.gyroADC[axis]  * scale;
		accTemp[axis] = (accTemp[axis] - (accTemp[axis] >>4)) + imu.accADC[axis];
		accSmooth[axis] = accTemp[axis]>>4;
		accMag += (accSmooth[axis] * 10 / (int16_t) acc_1G) * (accSmooth[axis] * 10 / (int16_t) acc_1G);
	}

	rotateV(&EstG.V,deltaGyroAngle);

	if ( abs(accSmooth[ROLL])<acc_25deg && abs(accSmooth[PITCH])<acc_25deg && accSmooth[YAW]>0)
		smallAngle25 = 1;
	else
		smallAngle25 = 0;

	// Apply complimentary filter (Gyro drift correction)
	// If accel magnitude >1.4G or <0.6G and ACC vector outside of the limit range => we neutralize the effect of accelerometers in the angle estimation.
	// To do that, we just skip filter, as EstV already rotated by Gyro
	if ((36 < accMag && accMag < 196) || smallAngle25){
		for (axis = 0; axis < 3; axis++) {
			EstG.A[axis] = (EstG.A[axis] * __gyro_acc_factor + accSmooth[axis]) * __acc_gyro_factor;
		}
	}
  
	// Attitude of the estimated vector
	att.angle[ROLL]  =  _atan2(EstG.V.X , EstG.V.Z) ;
	att.angle[PITCH] = _atan2(EstG.V.Y, EstG.V.Z);
}

#define UPDATE_INTERVAL 25000    // 40hz update rate (20hz LPF on acc)
#define INIT_DELAY      4000000  // 4 sec initialization delay
#define Kp1 0.55f                // PI observer velocity gain 
#define Kp2 1.0f                 // PI observer position gain
#define Ki  0.001f               // PI observer integral gain (bias cancellation)
#define dt  (UPDATE_INTERVAL / 1000000.0f)

void getEstimatedAltitude(){
	static uint8_t inited = 0;
	static int16_t AltErrorI = 0;
	static float AccScale  = 0.0f;
	static uint32_t deadLine = INIT_DELAY;
	int16_t AltError;
	int16_t InstAcc;
	int16_t Delta;
  
	if (getCurrentTime() < deadLine) 
		return;

	deadLine = getCurrentTime() + UPDATE_INTERVAL; 

	if (!inited) {
		inited = 1;
		EstAlt = BaroAlt;
		EstVelocity = 0;
		AltErrorI = 0;
		AccScale = 100 * 9.80665f / acc_1G;
	}

	AltError = BaroAlt - EstAlt; 
	AltErrorI += AltError;
	AltErrorI=constrain(AltErrorI,-25000,+25000);
	InstAcc = AltErrorI / 1000;
  
	// Integrators
	Delta = InstAcc * dt + (Kp1 * dt) * AltError;
	EstAlt += (EstVelocity/5 + Delta) * (dt / 2) + (Kp2 * dt) * AltError;
	EstVelocity += Delta*10;
}

void imuHandler() {
	uint8_t axis;
	static int16_t gyroADCprevious[3] = { 0, 0, 0 };
	int16_t gyroADCp[3];
	int16_t gyroADCinter[3];
	static int16_t gyroYawSmooth = 0;

	for (axis = 0; axis < 3; axis++)
		gyroADCp[axis] = imu.gyroADC[axis];

	getEstimatedAttitude();

	for (axis = 0; axis < 3; axis++) {
		gyroADCinter[axis] = imu.gyroADC[axis] + gyroADCp[axis];
		// empirical, we take a weighted value of the current and the previous values
		imu.gyroData[axis] = (gyroADCinter[axis] + gyroADCprevious[axis] + 1) / 3;

		gyroADCprevious[axis] = gyroADCinter[axis] / 2;

		imu.accADC[axis] = 0;
	}
}


void initIMU(){
	Serial.println("Configure IMU");

	updateCompFilterValues();

	GPIOState = GPIOReady;
}