#include "NiVekMessageParser.h"

extern uint16_t _currentPan;
extern uint16_t _currentTilt;

extern uint16_t _frontSonar;
extern uint16_t _rearSonar;
extern uint16_t _currentLeft;
extern uint16_t _currentRight;


void InitTurret();
void HandleTurretMessge(NiVekMessage *message);

void ReadTurretSensors();