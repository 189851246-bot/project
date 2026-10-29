#include "delay.h"

static volatile uint32_t sys_tick_ms = 0;

void delay_Init(void)
{
	SysTick_Config(SystemCoreClock/1000);// 72MHz  1ms产生一次中断
	NVIC_SetPriority(SysTick_IRQn,15); //设置最低优先级
}

void SysTick_Handler(void)//系统时钟中断次数
{
	sys_tick_ms++;
}

uint32_t gettick(void)//获取当前时间
{
	return sys_tick_ms;
}

void delay_ms(uint32_t ms)
{
	if(ms == 0) return;
	uint32_t start = sys_tick_ms;
	while((sys_tick_ms-start)<ms)
	{
		
	}
}
