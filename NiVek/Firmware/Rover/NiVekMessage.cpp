


#include "NiVekMessage.h"

NiVekMessage::NiVekMessage(){

}

void NiVekMessage::clear(){
	CheckSum = 0x00;
	MsgSize = 0;
}

void NiVekMessage::add_xor(uint8_t ch){
	CheckSum ^= ch;
}

void NiVekMessage::serialize(uint8_t *ptr, uint8_t len){
	uint8_t ch;
	uint8_t idx;
	for (idx = 0; idx < len; ++idx){
		ch = ptr[idx];
		add_xor(ch);
		MsgBuffer[MsgSize++] = ch;
	}

}

void NiVekMessage::serialize16(int16_t a) {
	add_xor((uint8_t) (0xFF & a));
	add_xor((uint8_t) (0xFF & (a >> 8)));

	MsgBuffer[MsgSize++] = a & 0xff;
	MsgBuffer[MsgSize++] = a >> 8 & 0xff;
}

void NiVekMessage::serialize8(uint8_t a)  {
	add_xor((uint8_t) (0xFF & a));

	MsgBuffer[MsgSize++] = a & 0xff;
}