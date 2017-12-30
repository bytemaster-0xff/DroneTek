/* 
	Editor: http://www.visualmicro.com
	        visual micro and the arduino ide ignore this code during compilation. this code is automatically maintained by visualmicro, manual changes to this file will be overwritten
	        the contents of the Visual Micro sketch sub folder can be deleted prior to publishing a project
	        all non-arduino files created by visual micro and all visual studio project or solution files can be freely deleted and are not required to compile a sketch (do not delete your own code!).
	        note: debugger breakpoints are stored in '.sln' or '.asln' files, knowledge of last uploaded breakpoints is stored in the upload.vmps.xml file. Both files are required to continue a previous debug session without needing to compile and upload again
	
	Hardware: Intel® Galileo, Platform=x86, Package=arduino
*/

#ifndef _VSARDUINO_H_
#define _VSARDUINO_H_
#define __BIN_I586__
#define ARDUINO 153
#define ARDUINO_MAIN
#define printf iprintf
#define __X86__
#define __x86__
#define F_CPU -m32
#define __cplusplus


void writeWelcomeMessage();
//
//
void oneHzLoop();
void fiveHzLoop();
void tenHzLoop();
void twentyHzLoop();
void fiftyHzLoop();
void oneHundredHzLoop();
void twoHundredHzLoop();
void fourHundredHzLoop();
void handleMessage(NiVekMessage *message);
void sendBasicTelemetry();
void sendAck(NiVekMessage *receivedMessage);
void sendNak(NiVekMessage *receivedMessage);
void commsHandler();
void CommitMessage(NiVekMessage *msg);
void initComms();
void HandleGPIOMessage(NiVekMessage *msg);
void initGPIO4();
void writeGPIO4(byte reg, byte val);
byte readByteGPIO4(byte reg);
byte readShortGPIO4(byte reg);
void HandleMotorMessage(NiVekMessage *msg);
void MotorsUpdate();
void HandleSensorMessage(NiVekMessage *msg);
void sendPong(NiVekMessage *receivedMessage);
void HandleSystemMessage(NiVekMessage* msg);
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
void InitTurret();
void SetTurretPanTilt(int pan, int tilt);
void UpdateTurret();
void ReadSonar();
void HandleTurretMessge(NiVekMessage *msg);

#include "K:\Electronics\Galileo\arduino-1.5.3\hardware\arduino\x86\cores\arduino\arduino.h"
#include "K:\Electronics\Galileo\arduino-1.5.3\hardware\arduino\x86\variants\galileo_fab_d\pins_arduino.h" 
#include "K:\Electronics\Galileo\arduino-1.5.3\hardware\arduino\x86\variants\galileo_fab_d\variant.h" 
#include "D:\NiVek\Micro\Firmware\NiVekRover\NiVekRover.ino"
#include "D:\NiVek\Micro\Firmware\NiVekRover\AppData.h"
#include "D:\NiVek\Micro\Firmware\NiVekRover\Comms.pde"
#include "D:\NiVek\Micro\Firmware\NiVekRover\GPIO.pde"
#include "D:\NiVek\Micro\Firmware\NiVekRover\GPIO14.pde"
#include "D:\NiVek\Micro\Firmware\NiVekRover\Motor.cpp"
#include "D:\NiVek\Micro\Firmware\NiVekRover\Motor.h"
#include "D:\NiVek\Micro\Firmware\NiVekRover\Motors.pde"
#include "D:\NiVek\Micro\Firmware\NiVekRover\Sensors.pde"
#include "D:\NiVek\Micro\Firmware\NiVekRover\System.pde"
#include "D:\NiVek\Micro\Firmware\NiVekRover\Timer.pde"
#include "D:\NiVek\Micro\Firmware\NiVekRover\Turret.cpp"
#include "D:\NiVek\Micro\Firmware\NiVekRover\Turret.h"
#include "D:\NiVek\Micro\Firmware\NiVekRover\Turret.pde"
#include "D:\NiVek\Micro\Firmware\NiVekRover\TurretManager.pde"
#include "D:\NiVek\Micro\Firmware\NiVekRover\def.h"
#endif
