#ifndef __USART__H
#define __USART__H

#include "stm32f10x.h"
#include "stdio.h"

void MY_USART1_Init(void);
void USART1_SendBytes(USART_TypeDef *USARTx, uint8_t *pData, uint16_t Size);
#endif

