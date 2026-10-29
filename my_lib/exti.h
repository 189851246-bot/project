#ifndef __EXTI__H
#define __EXTI__H
#include "stm32f10x.h"

extern volatile float max;
extern volatile uint8_t Key_Up_Flag;   // PB12
extern volatile uint8_t Key_Down_Flag; // PB13
extern volatile uint8_t Key_Mode_Flag; // PB14

void MY_EXTI_Init(void);

#endif

