#include "SR04.h"
#include "TIMdelay.h"
#include "delay.h"
void SR04_Init(void)
{
	//初始化时基单元
	//时钟TIM4
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);
	
	TIM_TimeBaseInitTypeDef TIM4_TimeBaseStruct;
	
	TIM4_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM4_TimeBaseStruct.TIM_Prescaler = 71;
	TIM4_TimeBaseStruct.TIM_Period = 65534;
	TIM4_TimeBaseStruct.TIM_RepetitionCounter = 0;
	
	TIM_TimeBaseInit(TIM4,&TIM4_TimeBaseStruct);
	//初始化输入捕获的IO引脚
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
	
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	
	//初始化输入捕获通道
	TIM_ICInitTypeDef TIM_ICInitStruct;
	//CH3
	TIM_ICInitStruct.TIM_Channel = TIM_Channel_3;
	TIM_ICInitStruct.TIM_ICFilter = 0;
	TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;
	
	TIM_ICInit(TIM4,&TIM_ICInitStruct);
	//CH4
	TIM_ICInitStruct.TIM_Channel = TIM_Channel_4;
	TIM_ICInitStruct.TIM_ICFilter = 0;
	TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Falling;
	TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_IndirectTI;
	
	TIM_ICInit(TIM4,&TIM_ICInitStruct);
	//初始化trigger引脚
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	
}


float Distance_test(void)
{
	//1.对CNT清零
	TIM_SetCounter(TIM4,0);
	//2.对cc1和cc2清零
	TIM_ClearFlag(TIM4,TIM_FLAG_CC3);
	TIM_ClearFlag(TIM4,TIM_FLAG_CC4);
	//3.开启定时器
	TIM_Cmd(TIM4,ENABLE);
	//4.trigger发送>10us脉冲
	GPIO_WriteBit(GPIOB,GPIO_Pin_5,Bit_SET);
	delay_us (15);
	GPIO_WriteBit(GPIOB,GPIO_Pin_5,Bit_RESET);
	//5.等待cc1和cc2分别检测出上升沿，下降沿
	uint32_t t0 = gettick();
	while (TIM_GetFlagStatus(TIM4, TIM_FLAG_CC3) == RESET)
		{
    if (gettick() - t0 > 50) { TIM_Cmd(TIM4, DISABLE); return -1.0f; }  // 上升沿超时
		}
	while (TIM_GetFlagStatus(TIM4, TIM_FLAG_CC4) == RESET)
		{
    if (gettick() - t0 > 50) { TIM_Cmd(TIM4, DISABLE); return -2.0f; }  // 下降沿超时
		}
	//6.关闭定时器
	TIM_Cmd(TIM4,DISABLE);
	//7.距离测量公式
		//#1.读取ccr3的值
		uint16_t ccr3 = TIM_GetCapture3(TIM4);
		//#2.读取ccr4的值
		uint16_t ccr4 = TIM_GetCapture4(TIM4);
		//#3.计算距离
		float distance = (ccr4-ccr3)*1.0e-6f*340.0f/2;
		return distance;
}
