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
void CheckOK(HardwareSerial port);
void sendBlueToothCommand(HardwareSerial port, char command[]);
void setupBluetooth(HardwareSerial port);
void calibrateJoyStickes();
void noJoystickData();
void controlLoop();
void waitLoop();
void checkBattery();
void commo();
void flightControls(dual_stick_info_t data);
bool is1HzLoop(uint32_t currentTime);
//
void isp_setup();
void isp_loop(void);
uint8_t getch();
void readbytes(int n);
void putch(char c);
void spi_init();
uint8_t spi_send(uint8_t b);
uint8_t spi_transaction(uint8_t a, uint8_t b, uint8_t c, uint8_t d);
void empty_reply();
void breply(uint8_t b);
void get_version(uint8_t c);
void set_parameters();
void start_pmode();
void end_pmode();
void universal();
void flash(uint8_t hilo, int addr, uint8_t data);
void commit(int addr);
int current_page(int addr);
uint8_t write_flash(int length);
uint8_t write_eeprom(int length);
void program_page();
uint8_t flash_read(uint8_t hilo, int addr);
char flash_read_page(int length);
char eeprom_read_page(int length);
void read_page();
void read_signature();
int avrisp();
void sendAck(NiVekMessage *receivedMessage);
void sendNak(NiVekMessage *receivedMessage);
void HandleMessage(NiVekMessage *msg, bool fromRadio);
void SendMessageToController(NiVekMessage *msg);
void setLEDState(LEDTypes led, LEDStates newState);
void updateLEDS();
void initLEDS();
void writeMotors();
void writeAllMotors(uint16_t power);
void vestPower(uint8_t idx, uint16_t value);
void vestAllOff();
void vestAllOn();
void updateMotors();
void initOutput();

#include "k:\Electronics\AVR\arduino-0023\hardware\arduino\cores\picopter_core\wprogram.h"
#include "D:\NiVek\Micro\Firmware\PicopterTransmitter\PicopterTransmitter.pde"
#include "D:\NiVek\Micro\Firmware\PicopterTransmitter\ArduinoISP.pde"
#include "D:\NiVek\Micro\Firmware\PicopterTransmitter\Common.ino"
#include "D:\NiVek\Micro\Firmware\PicopterTransmitter\LEDManager.pde"
#include "D:\NiVek\Micro\Firmware\PicopterTransmitter\VestManager.pde"
#include "D:\NiVek\Micro\Firmware\PicopterTransmitter\def.h"
#endif
