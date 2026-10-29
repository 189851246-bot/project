#ifndef __TIMDEALY__H
#define __TIMDEALY__H
#include "stm32f10x.h"

void TIM2_TimeBaseInit(void);
void delay_us(uint16_t us);

#endif
