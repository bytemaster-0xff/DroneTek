#ifndef KALMAN_H_
#define KALMAN_H_

#include "common/twb_common.h"

iOpResult_e TWB_SNSR_Kalman_Init(void);

iOpResult_e TWB_SNSR_Kalman_ResetDefaults(void);
iOpResult_e TWB_SNSR_Kalman_UpdateSetting(uint16_t addr, uint8_t value);
iOpResult_e TWB_SNSR_Kalman_Process(float deltaT);
void TWB_SNSR_Kalman_SendDiag(void);

#endif /* KALMAN_H_ */
