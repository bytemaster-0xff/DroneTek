#include "NiVekMessage.h"

#define OUTPUT_BUFFER_SIZE 128

void initComms();

void sendAck(NiVekMessage *msg);

void CustomLogging(char* str);

extern char OutputBuffer[OUTPUT_BUFFER_SIZE];

void WriteOutput();

void commsHandler();

void SendTelemetry();

