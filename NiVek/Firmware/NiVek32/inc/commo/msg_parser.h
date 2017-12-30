/*
 * msg_parser.h
 *
 *  Created on: Oct 4, 2012
 *      Author: kevinw
 */

#ifndef MSG_PARSER_H_
#define MSG_PARSER_H_

#include "common/twb_common.h"


void TWB_MsgParser_HandleTXE(void);
void TWB_MsgParser_HandleRX(uint8_t ch);
void TWB_MsgParser_HandleRX_Overrun(void);


#endif /* MSG_PARSER_H_ */
