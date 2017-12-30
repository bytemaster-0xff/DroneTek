
#ifndef __MAIN_H
#define __MAIN_H

#include "common/twb_timer.h"

typedef enum {
	Outgoing_Msg_Sensor,
	Outgoing_Msg_GPS,
	Outgoing_Msg_Battery
} OutgoingMsg_t;

void TWB_Status_QueueMsg(void);

/* Startup Methods */
void __twb_main_AllocateMemory(void);
iOpResult_e __startMainLoop(void);
iOpResult_e __initMain(void);
iOpResult_e __main_sendVersionInfo(void);
iOpResult_e __initModules(void);
iOpResult_e __initMainLoopTimer(void);
iOpResult_e __setDefaults(void);
void __configureEXTI(void);

#endif
