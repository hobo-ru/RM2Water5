#ifndef WATER5_H_INCLUDED
#define WATER5_H_INCLUDED

#include "rm2_hal.h"

void Water5OneSec();
void Water5Init(uint8_t clear);
bool Water5Loop();
void ClearAllParams();


typedef struct _W5_TAGS_TO_SAVE
{
  uint32_t PulseCounter0;
  uint32_t PulseCounter1;
  uint16_t MesNum;
  uint8_t Protocol;  
  uint8_t poehali;
  uint8_t initcal;
  uint8_t reservedMas[16];
} W5_TAGS_TO_SAVE;


extern W5_TAGS_TO_SAVE W5_Tags;
extern uint8_t Voltage ;
extern int8_t  PowLev;
extern uint8_t  Calibrate;

#endif // WATER5_H_INCLUDED
