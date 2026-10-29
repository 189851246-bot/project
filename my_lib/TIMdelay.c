#include "TIMdelay.h"


void TIM2_TimeBaseInit(void)
{
	//使用TIM2基础定时器实现1us的延迟，TIM2没有重复计数器RCR
	//使能TIM2
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	
	//配置参数
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_Period = 0xFFFF;//自动重装载寄存器
	TIM_TimeBaseInitStruct.TIM_Prescaler = 71;//预分频器
	TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0;//重复计数器，没有但配置上了
	
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStruct);
	//闭合时基单元开关
	TIM_Cmd(TIM2,ENABLE);
	
}
//延时函数 65535us = 65.535ms,最多计时65ms超过65ms返回
void delay_us(uint16_t us)
{
	if (us == 0 || us > 60000) return; 
	uint16_t start = TIM_GetCounter(TIM2);
	 while ((uint16_t)(TIM_GetCounter(TIM2) - start) < us);
}


