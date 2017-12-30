/*
 * twb_strings.h
 *
 *  Created on: Aug 1, 2013
 *      Author: Kevin
 */

#ifndef TWB_STRINGS_H_
#define TWB_STRINGS_H_

#include "common/twb_common.h"

void twb_strreverse(uint8_t* begin, uint8_t* end);
void twb_itoa(uint32_t value, uint8_t* str, uint8_t base);


#endif /* TWB_STRINGS_H_ */
