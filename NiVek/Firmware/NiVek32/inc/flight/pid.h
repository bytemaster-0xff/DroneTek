/*
 * pid.h
 *
 *  Created on: May 16, 2013
 *      Author: Kevin
 */

#ifndef PID_H_
#define PID_H_

#include "../twb_config.h"

iOpResult_e TWB_PID_Update(PID_t *pid);
iOpResult_e TWB_PID_Reset(PID_t *pid);


#endif /* PID_H_ */
