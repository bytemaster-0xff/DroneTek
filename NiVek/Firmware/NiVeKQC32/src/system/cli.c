/*
 * cli.c
 *
 *  Created on: Jul 31, 2013
 *      Author: Kevin
 */


#include <stdlib.h>
#include <string.h>

#include "system/cli.h"
#include "common/memory_mgmt.h"
#include "common/twb_strings.h"
#include "common/twb_debug.h"
#include "commo/usb/usbd_conf.h"
#include "commo/wifi.h"
#include "nivek_system.h"
#include "commo/usb/usbd_cdc_core.h"
#include "system/cli.h"

#define CLI_BUFFER_SIZE 255
#define CLI_COMMAND_QUEUE_SIZE 4
#define CLI_COMMAND_MAX_LENGTH 16

uint8_t __cli_temp_IntBuffer[10];

//In buffer resets to zero once we parse a command.
uint8_t _cli_in_buffer_idx = 0;
uint8_t *_cli_in_buffer;

//Out buffer is a ring buffer.
uint8_t *_cli_out_buffer;
uint8_t _cli_out_buffer_head = 0;
uint8_t _cli_out_buffer_tail = 0;

/*
 * No work should be done in the primary USB character based IRQ handler,
 * if work needs to be done, queue it up and let it get handled in our
 * main loop.
 */
uint8_t *_cli_commandQueue[CLI_COMMAND_QUEUE_SIZE];
uint8_t _cli_commandQueue_head = 0;
uint8_t _cli_commandQueue_tail = 0;


iOpResult_e TWB_CLI_Init(void) {
	_cli_in_buffer_idx = 0;
	_cli_out_buffer_head = 0;
	_cli_out_buffer_tail = 0;

	_cli_in_buffer = pm_malloc(CLI_BUFFER_SIZE);
	_cli_out_buffer = pm_malloc(CLI_BUFFER_SIZE);

	for(uint8_t idx = 0; idx < CLI_COMMAND_QUEUE_SIZE; ++idx)
		_cli_commandQueue[idx] = pm_malloc(CLI_COMMAND_MAX_LENGTH);

	TWB_Debug_Print("CLI - Online\r\n");

	return OK;
}

iOpResult_e TWB_CLI_ParseCH(uint8_t ch) {
	if(ch == '\r'){
		if(WiFiState != WiFi_CLIMode)
			TWB_CLI_OutStr("\r\n"); /* Finish echoing back the command  */

		strncpy((char *)_cli_commandQueue[_cli_commandQueue_tail],(const char *)_cli_in_buffer, _cli_in_buffer_idx);
		_cli_commandQueue[_cli_commandQueue_tail][_cli_in_buffer_idx] = 0x00;

		_cli_commandQueue_tail++; /* Yeah, it's a not-needed line, but makes things very explicit on what's happening */

		TWB_Debug_Print2Int("CLI CMD Queue: ", _cli_commandQueue_head, _cli_commandQueue_tail);

		if(_cli_commandQueue_tail == CLI_COMMAND_QUEUE_SIZE)
			_cli_commandQueue_tail = 0;

		_cli_in_buffer_idx = 0;
	}
	else
		_cli_in_buffer[_cli_in_buffer_idx++] = ch;

	if(WiFiState != WiFi_CLIMode)
		TWB_CLI_OutCH(ch);

	return OK;
}

iOpResult_e TWB_CLI_ProcessCommands(void){
	while(_cli_commandQueue_head != _cli_commandQueue_tail){
		const char *command = (const char *)_cli_commandQueue[_cli_commandQueue_head++];
		TWB_Debug_Print("PROCESS ->");
		TWB_Debug_Print(command);
		TWB_Debug_Print("<-\r\n");

		uint8_t commandLength = strlen(command);

		if(_cli_commandQueue_head == CLI_COMMAND_QUEUE_SIZE)
			_cli_commandQueue_head = 0;

				if(strcmp("ping", command) == 0){
			TWB_CLI_OutStr("pong\r");
			return OK;
		}
		else if(strcmp("cmd", command) == 0){
			TWB_Debug_Print("OK\r\n");

			TWB_CLI_OutStr("Welcome NiVek - ");
			TWB_CLI_OutStr("V");

			uint16_t version = (uint16_t)FIRMWARE_VERSION / 100.0f;
			uint16_t revision = (uint16_t)((FIRMWARE_VERSION - (float)version) * 100.0f);

			twb_itoa(version, __cli_temp_IntBuffer, 10);
			TWB_CLI_OutStr((const char *)__cli_temp_IntBuffer);

			twb_itoa(revision, __cli_temp_IntBuffer, 10);

			TWB_CLI_OutStr((revision < 10) ? ".0" : ".");
			TWB_CLI_OutStr((const char *)__cli_temp_IntBuffer);
			TWB_CLI_OutStr("\r");
		}
		else if(strcmp("help", command) == 0){
			TWB_CLI_OutStr("Help\r");
			TWB_CLI_OutStr("============\r");
			TWB_CLI_OutStr(" UART");
			TWB_CLI_OutStr("    comms b gets baud rate\r");
			TWB_CLI_OutStr(" ");
			TWB_CLI_OutStr("	comms b 3 => 38400\r");
			TWB_CLI_OutStr("	comms b 5 => 57600\r");
			TWB_CLI_OutStr("	comms b 1 => 115200\r");
			TWB_CLI_OutStr("	comms b 2 => 203400\r");
			TWB_CLI_OutStr("	comms b 9 => 9600\r");
			TWB_CLI_OutStr("\r");
			TWB_CLI_OutStr("    comms - Direct access to send/receive to primary USART\r");
			TWB_CLI_OutStr("	exit  - Exit direct access to send/receive to primary USART\r");
			TWB_CLI_OutStr("\r");
			TWB_CLI_OutStr("cmd - Return version\r");
			TWB_CLI_OutStr("ping - will return 'pong' \r");
		}
		else if (strncmp("set", command, 3) == 0 && WiFiState != WiFi_CLIMode){
			if (command[4] == 'a' && commandLength == 9){
				char tmpAddr[4];
				strncpy(tmpAddr, &command[6], 3);
				tmpAddr[3] = 0x00;
				uint8_t address = atoi(tmpAddr);
				assert_succeed(TWB_Sys_Set_Address(address));
				twb_itoa(address, __cli_temp_IntBuffer, 10);
				TWB_CLI_OutStr("Set new address: ");
				TWB_CLI_OutStr(__cli_temp_IntBuffer);
				TWB_CLI_OutStr("\r");
			}
			else if (command[4] == 'n'){
				for (uint8_t idx = 0; idx < 10; ++idx) __cli_temp_IntBuffer[idx] = 0;
				strncpy(__cli_temp_IntBuffer, &command[6], commandLength - 6);
				assert_succeed(TWB_Sys_Set_NiVekName(__cli_temp_IntBuffer));
				TWB_CLI_OutStr("OK - Set Name: ");
				TWB_CLI_OutStr(NiVekName);
				TWB_CLI_OutStr("\r");
			}
			else if (command[4] == 's'){
				for (uint8_t idx = 0; idx < 10; ++idx) __cli_temp_IntBuffer[idx] = 0;
				strncpy(__cli_temp_IntBuffer, &command[6], commandLength - 6);
				TWB_CLI_OutStr("\r\n");
				uint32_t sn = atoi(__cli_temp_IntBuffer);
				assert_succeed(TWB_Sys_Set_SerialNumber(sn));
				TWB_CLI_OutStr("OK - Set Serial Number: ");
				TWB_CLI_OutStr(__cli_temp_IntBuffer);
				TWB_CLI_OutStr("\r");

			}
			else{
				TWB_CLI_OutStr("Unknown Set -> ");
				TWB_CLI_OutStr(command);
				TWB_CLI_OutStr("\r");
			}
		}
		else if (strncmp("get", command, 3) == 0 && WiFiState != WiFi_CLIMode){
			switch (command[4]){
				case 'a':
					TWB_CLI_OutStr("a=");
					twb_itoa(NiVekAddress, __cli_temp_IntBuffer, 10);
					TWB_CLI_OutStr(__cli_temp_IntBuffer);
					TWB_CLI_OutStr("\r");
					break;
				case 's':
					TWB_CLI_OutStr("s=");
					twb_itoa(NivekSerialNumber, __cli_temp_IntBuffer, 10);
					TWB_CLI_OutStr(__cli_temp_IntBuffer);
					TWB_CLI_OutStr("\r");
					break;
				case 'n':
					TWB_CLI_OutStr("n=");
					TWB_CLI_OutStr(NiVekName);
					TWB_CLI_OutStr("\r");
					break;
				default:
					TWB_CLI_OutStr("Unknown Get\r");
					TWB_CLI_OutStr("a - address\r");
					TWB_CLI_OutStr("n - device name\r");
					TWB_CLI_OutStr("s - serial number\r");
					break;
			}
		}
		else if(strcmp("debug on", command) == 0){
			TWB_Debug_SetShouldForwardDebugOverUSB(ForwardDebugMessages_On);
			TWB_CLI_OutStr("DEBUG - ON\r");
		}
		else if(strcmp("debug off", command) == 0){
			TWB_Debug_SetShouldForwardDebugOverUSB(ForwardDebugMessages_Off);
			TWB_CLI_OutStr("DEBUG - OFF\r");
		}
		else if(strncmp("comms", command, 5) == 0){
			TWB_Debug_Print("Comms Messge\r\n");
			if(commandLength == 7 && command[6] == 'b'){
				switch(TWB_WiFi_GetBaudRate()){
					case Baud_38400: TWB_CLI_OutStr("BAUD - 38400\r"); break;
					case Baud_57600: TWB_CLI_OutStr("BAUD - 57600\r"); break;
					case Baud_115200: TWB_CLI_OutStr("BAUD - 115200\r"); break;
					case Baud_230400: TWB_CLI_OutStr("BAUD - 203400\r"); break;
					case Baud_9600: TWB_CLI_OutStr("BAUD - 9600\r"); break;
					default: TWB_CLI_OutStr("Unknown\r");break;
				}

			}
			else if(commandLength == 9 && command[6] == 'b'){
				switch(command[8]){
					case '3': TWB_CLI_OutStr("BAUD - 38400 SAVED\r"); TWB_WiFi_SetBaud(3); break;
					case '5': TWB_CLI_OutStr("BAUD - 57600 SAVED\r"); TWB_WiFi_SetBaud(5);  break;
					case '1': TWB_CLI_OutStr("BAUD - 115200 SAVED\r"); TWB_WiFi_SetBaud(1);  break;
					case '2': TWB_CLI_OutStr("BAUD - 203400 SAVED\r"); TWB_WiFi_SetBaud(2);  break;
					case '9': TWB_CLI_OutStr("BAUD - 9600 SAVED\r"); TWB_WiFi_SetBaud(9);  break;
					default: TWB_CLI_OutStr("BAUD -? (9,3,5,1,2) - ?\r");break;
				}
			}
			else if(commandLength == 5){
				TWB_CLI_OutStr("COMMS - DIRECT\r");
				WiFiState = WiFi_CLIMode;
			}
			else
				TWB_CLI_OutStr("COMMS - ?\r");

		}
		else if(strcmp("exit", command) == 0){
			if(WiFiState == WiFi_CLIMode){
				TWB_CLI_OutStr("COMMS - EXIT\r");
				WiFiState = WiFi_Ready;
			}
			else
				TWB_CLI_OutStr("BYE\r");
		}
		else{
			if(WiFiState != WiFi_CLIMode){
				TWB_CLI_OutStr("? -> ");
				TWB_CLI_OutStr(command);
				TWB_CLI_OutStr("\r");
			}
			else{
				for(uint8_t idx = 0; idx < strlen(command); ++idx){
					_cli_out_buffer[_cli_out_buffer_tail++] = command[idx];

					if(_cli_out_buffer_tail == CLI_BUFFER_SIZE)
						_cli_out_buffer_tail = 0;
				}

				if(strcmp("$$$", command) != 0){
					_cli_out_buffer[_cli_out_buffer_tail++] = '\r';

					if(_cli_out_buffer_tail == CLI_BUFFER_SIZE)
						_cli_out_buffer_tail = 0;

				}

				USART_ITConfig(USART2, USART_IT_TXE, ENABLE);

			}
		}
	}
	return OK;
}

iOpResult_e TWB_CLI_ParseBuffer(const char *buffer, uint32_t len) {
	for(int idx = 0; idx < len; ++idx)
		TWB_CLI_ParseCH(buffer[idx]);

	return OK;
}

iOpResult_e TWB_CLI_OutStr(const char *strBuffer){
	char ch;
	while((ch = *strBuffer++) != '\0'){
		 USB_Console_Buffer->Buffer[USB_Console_Buffer->PtrIn++] = ch;

		 /*To avoid buffer overflow */
		 if (USB_Console_Buffer->PtrIn == USB_BUFFER_SIZE)
			 USB_Console_Buffer->PtrIn = 0;
	}

	return OK;
}

iOpResult_e TWB_CLI_OutCH(uint8_t ch) {
	 USB_Console_Buffer->Buffer[USB_Console_Buffer->PtrIn++] = ch;

	 /*To avoid buffer overflow */
	 if (USB_Console_Buffer->PtrIn == USB_BUFFER_SIZE)
		 USB_Console_Buffer->PtrIn = 0;

	return OK;
}

iOpResult_e TWB_CLI_OutBuffer(const char *buffer, uint32_t len){
	for(int idx = 0; idx < len; ++idx){
		USB_Console_Buffer->Buffer[USB_Console_Buffer->PtrIn++] = buffer[idx];

		 /* To avoid buffer overflow */
		 if (USB_Console_Buffer->PtrIn == USB_BUFFER_SIZE)
			 USB_Console_Buffer->PtrIn = 0;
	}

	return OK;
}

/* Methods that are a direct line/handler to the UART for the communications channel */
void TWB_CLI_HandleTXE(){
	if(_cli_out_buffer_tail == _cli_out_buffer_head)
		USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
	else{
		USART2->DR = _cli_out_buffer[_cli_out_buffer_head++];

		if(_cli_out_buffer_head == CLI_BUFFER_SIZE)
			_cli_out_buffer_head = 0;

		if(_cli_out_buffer_tail == _cli_out_buffer_head)
			USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
		else
			USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
	}
}

void TWB_CLI_HandleRX(uint8_t ch) {
	TWB_CLI_OutCH(ch);
}

void TWB_CLI_HandleRX_Overrun() {

}
