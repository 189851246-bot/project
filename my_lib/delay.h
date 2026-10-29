#ifndef __DELAY__H
#define __DELAY__H
#include "stm32f10x.h"

void delay_Init(void);
void SysTick_Handler(void);
uint32_t gettick(void);
void delay_ms(uint32_t ms);

#endif
