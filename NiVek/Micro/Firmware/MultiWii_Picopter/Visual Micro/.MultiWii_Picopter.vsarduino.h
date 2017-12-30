/* 
	Editor: http://www.visualmicro.com
	        visual micro and the arduino ide ignore this code during compilation. this code is automatically maintained by visualmicro, manual changes to this file will be overwritten
	        the contents of the Visual Micro sketch sub folder can be deleted prior to publishing a project
	        all non-arduino files created by visual micro and all visual studio project or solution files can be freely deleted and are not required to compile a sketch (do not delete your own code!).
	        note: debugger breakpoints are stored in '.sln' or '.asln' files, knowledge of last uploaded breakpoints is stored in the upload.vmps.xml file. Both files are required to continue a previous debug session without needing to compile and upload again
	
	Hardware: picopter_flier, Platform=avr, Package=arduino
*/

#ifndef _VSARDUINO_H_
#define _VSARDUINO_H_
#define __AVR_ATmega128rfa1__
#define __AVR_ATmega128RFA1__
#define ARDUINO 23
#define ARDUINO_MAIN
#define __AVR__
#define __avr__
#define F_CPU 16000000L
#define __cplusplus
#define __inline__
#define __asm__(x)
#define __extension__
#define __ATTR_PURE__
#define __ATTR_CONST__
#define __inline__
#define __asm__ 
#define __volatile__

#define __builtin_va_list
#define __builtin_va_start
#define __builtin_va_end
#define __DOXYGEN__
#define __attribute__(x)
#define NOINLINE __attribute__((noinline))
#define prog_void
#define PGM_VOID_P int
            
typedef unsigned char byte;
extern "C" void __cxa_pure_virtual() {;}

//
void fourHundredHzLoop();
void twoHundredHzLoop();
void oneHundredHzLoop();
void fiftyHzLoop();
void twentyHzLoop();
void tenHzLoop();
void fiveHzLoop();
void oneHzLoop();
//
void clearAccTrim();
void clearAccZero();
void calibrateACC();
void beginAccZero();
void ACC_Common();
void ACC_Read();
void calibrationFinished();
void beginSensorZero(NiVekMessage *msg);
void handleMessage(NiVekMessage *message);
void sendBasicTelemetry();
void sendMotorStatus();
void sendRCChannel();
void sendStatus();
void sendPID(NiVekMessage *requestorMessage);
void sendAck(NiVekMessage *receivedMessage);
void sendNak(NiVekMessage *receivedMessage);
void commsHandler();
void initComms();
void readEEPROM();
void writeParams();
void setDefaults();
void checkFirstTime();
void armNiVek(NiVekMessage *msg);
void safeNiVek(NiVekMessage *msg);
void setConfigValue(NiVekMessage *msg);
void getConfigValue(NiVekMessage *msg);
void handleFlightMessage(NiVekMessage *msg);
void clearIntegral();
void onTheGround();
void turnOnStableMode(NiVekMessage *msg);
void turnOffStableMode(NiVekMessage *msg);
void calibrateGyros();
void clearGyroZero();
void beginGyroZero();
void GYRO_Common();
bool GYRO_Read();
void getADC();
bool Mag_init();
void initHardware();
void updateCompFilterValues();
void updateCompFilterSetting(NiVekMessage *msg);
void sendCompFilterConfig(NiVekMessage *incomingMsg);
void getEstimatedAttitude();
void getEstimatedAltitude();
void imuHandler();
void initIMU();
void setLEDState(LEDTypes led, LEDStates newState);
void updateLEDS();
void initLEDS();
bool MPU60x0_ReadAcc();
bool MPU60x0_ReadGyro();
bool MPU60x0_init();
void i2c_MS561101BA_reset();
void i2c_MS561101BA_readCalibration();
bool  Baro_init();
void i2c_MS561101BA_UT_Start();
void i2c_MS561101BA_UP_Start();
void i2c_MS561101BA_UP_Read();
void i2c_MS561101BA_UT_Read();
void i2c_MS561101BA_Calculate();
uint8_t Baro_Update();
void Baro_Common();
int16_t _atan2(float y, float x);
void rotateV(struct fp_vector *v, float* delta);
float InvSqrt(float x);
int32_t isq(int32_t x);
void writeMotors();
void writeAllMotors(int16_t mc);
void initOutput();
void mixTable();
void updateMotors();
void beginMotorConfig(NiVekMessage *msg);
void endMotorConfig(NiVekMessage *msg);
void setMotorPower(NiVekMessage *msg);
void printMotors();
void initMotors();
void motorsHandler();
void initReceiver();
void computeRC();
void handleModeSwitch();
void radioHandler();
void transitionToSensorState(SensorStates newState);
bool initSensors();
void sensorHandler();
void updateSensorValue(NiVekMessage *msg);
void handleSensorMessage(NiVekMessage *msg);
void UartSendData();
bool getOkToArm();
void sendConfiguration(uint8_t destAddress);
void sendName(uint8_t destAddress);
void setName(NiVekMessage *msg);
void handleSystemMessage(NiVekMessage *msg);
void initSys();
void initTimings();
void updateTimings();
void setPreviousTime();
bool is400HzLoop();
bool is200HzLoop();
bool is100HzLoop();
bool is50HzLoop();
bool is25HzLoop();
bool is20HzLoop();
bool is5HzLoop();
bool is10HzLoop();
bool is1HzLoop();
void timerHandler();
uint32_t getCurrentTime();
uint16_t getCycleTime();
void writeWelcomeMessage();
void blinkLED(uint8_t num, uint8_t wait, uint8_t repeat);
void fastBlink();
void checkArmedStateChange();
void batteryCheck();
void connectionMonitor();
void isFlipped();
void watchdog();
void i2c_init(void);
void i2c_rep_start(uint8_t address);
void i2c_stop(void);
void i2c_write(uint8_t data);
uint8_t i2c_readAck();
uint8_t i2c_readNak(void);
void waitTransmissionI2C();
void i2c_getSixRawADC(uint8_t add, uint8_t reg);
void i2c_writeReg(uint8_t add, uint8_t reg, uint8_t val);
uint8_t i2c_readReg(uint8_t add, uint8_t reg);
void getPIDValue(NiVekMessage *msg);
void setPIDValue(NiVekMessage *msg);
void pid();
void dynamicPID();
void pidHandler();

#include "k:\Electronics\AVR\arduino-0023\hardware\arduino\cores\picopter_core\wprogram.h"
#include "D:\NiVek\Micro\Firmware\MultiWii_Picopter\MultiWii_Picopter.pde"
#include "D:\NiVek\Micro\Firmware\MultiWii_Picopter\Accelerometer.pde"
#include "D:\NiVek\Micro\Firmware\MultiWii_Picopter\AppData.h"
#include "D:\NiVek\Micro\Firmware\MultiWii_Picopter\Calibration.pde"
#include "D:\NiVek\Micro\Firmware\MultiWii_Picopter\Comms.pde"
#include "D:\NiVek\Micro\Firmware\MultiWii_Picopter\EEPROM.pde"
#include "D:\NiVek\Micro\Firmware\MultiWii_Picopter\Enums.h"
#include "D:\NiVek\Micro\Firmware\MultiWii_Picopter\FlightCtl.pde"
#include "D:\NiVek\Micro\Firmware\MultiWii_Picopter\Gyro.pde"
#include "D:\NiVek\Micro\Firmware\MultiWii_Picopter\HMC5883.pde"
#include "D:\NiVek\Micro\Firmware\MultiWii_Picopter\Hardware.pde"
#include "D:\NiVek\Micro\Firmware\MultiWii_Picopter\IMU.pde"
#include "D:\NiVek\Micro\Firmware\MultiWii_Picopter\LEDManager.pde"
#include "D:\NiVek\Micro\Firmware\MultiWii_Picopter\MPU60X0.pde"
#include "D:\NiVek\Micro\Firmware\MultiWii_Picopter\MS5611.pde"
#include "D:\NiVek\Micro\Firmware\MultiWii_Picopter\Math.pde"
#include "D:\NiVek\Micro\Firmware\MultiWii_Picopter\Motors.pde"
#include "D:\NiVek\Micro\Firmware\MultiWii_Picopter\Receiver.pde"
#include "D:\NiVek\Micro\Firmware\MultiWii_Picopter\Sensors.pde"
#include "D:\NiVek\Micro\Firmware\MultiWii_Picopter\Serial.pde"
#include "D:\NiVek\Micro\Firmware\MultiWii_Picopter\System.pde"
#include "D:\NiVek\Micro\Firmware\MultiWii_Picopter\Timings.pde"
#include "D:\NiVek\Micro\Firmware\MultiWii_Picopter\Utils.pde"
#include "D:\NiVek\Micro\Firmware\MultiWii_Picopter\Watchdog.pde"
#include "D:\NiVek\Micro\Firmware\MultiWii_Picopter\config.h"
#include "D:\NiVek\Micro\Firmware\MultiWii_Picopter\def.h"
#include "D:\NiVek\Micro\Firmware\MultiWii_Picopter\i2c.pde"
#include "D:\NiVek\Micro\Firmware\MultiWii_Picopter\pid.pde"
#endif
