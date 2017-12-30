/*
 * cli.h
 *
 *  Created on: Jul 31, 2013
 *      Author: Kevin
 */

#ifndef CLI_H_
#define CLI_H_

#include "common/twb_common.h"

void TWB_CLI_HandleTXE(void);
void TWB_CLI_HandleRX(uint8_t ch);
void TWB_CLI_HandleRX_Overrun(void);

iOpResult_e TWB_CLI_Init(void);
iOpResult_e TWB_CLI_ParseCH(uint8_t ch);
iOpResult_e TWB_CLI_ParseBuffer(const char *buffer, uint32_t len);
iOpResult_e TWB_CLI_ProcessCommands(void);

iOpResult_e TWB_CLI_OutCH(uint8_t ch);
iOpResult_e TWB_CLI_OutStr(const char *buffer);
iOpResult_e TWB_CLI_OutBuffer(const char *buffer, uint32_t len);

#endif /* CLI_H_ */
