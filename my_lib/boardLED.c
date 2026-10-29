#include "boardLED.h"


void MY_OnBoardLED_Init(void)
{
	//使能IO端口
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	//初始化板载LED
	GPIO_InitTypeDef GPIO_OnBoardLED_InitStruct;
	//配置板载LED参数 输出开漏 PC13 2MHz
	GPIO_OnBoardLED_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_OnBoardLED_InitStruct.GPIO_Pin = GPIO_Pin_13;
	GPIO_OnBoardLED_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	
	GPIO_Init(GPIOC,&GPIO_OnBoardLED_InitStruct);
}

void LED_ON(void)
{
	//初始板载LED状态--亮灯
	GPIO_WriteBit(GPIOC,GPIO_Pin_13,Bit_RESET);
}

void LED_OFF(void)
{
//初始板载LED状态--灭灯
	GPIO_WriteBit(GPIOC,GPIO_Pin_13,Bit_SET);
}
