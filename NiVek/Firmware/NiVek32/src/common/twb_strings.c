/*
 * twb_strings.c
 *
 *  Created on: Aug 1, 2013
 *      Author: Kevin
 */

#include "common/twb_strings.h"
#include <stdlib.h>

void twb_strreverse(uint8_t* begin, uint8_t* end) {
	char aux;
	while(end>begin)
		aux=*end, *end--=*begin, *begin++=aux;
}

void twb_itoa(uint32_t value, uint8_t* str, uint8_t base) {
	static char num[] = "0123456789abcdefghijklmnopqrstuvwxyz";

	uint8_t* wstr=str;
	int sign;
//	div_t res;

	uint32_t quot;
	uint32_t rem;

	if (base<2 || base>35){ *wstr='\0'; return; }

	if ((sign=value) < 0) value = -value;

	do {
		quot = value / base;
		rem = value % base;
//		res = div(value,base);

		*wstr++ = num[rem];

	}while((value=quot));

	if(sign<0) *wstr++='-';

	*wstr='\0';

	twb_strreverse(str,wstr-1);
}


