#include "exti.h"

volatile float max = 0.4f;
volatile uint8_t Key_Up_Flag = 0;
volatile uint8_t Key_Down_Flag = 0;
volatile uint8_t Key_Mode_Flag = 0;

void MY_EXTI_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource12);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource13);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource14);
    
    EXTI_InitTypeDef EXTI_InitStruct;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling; // 下降沿触发
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_InitStruct.EXTI_Line = EXTI_Line12 | EXTI_Line13 | EXTI_Line14;
    EXTI_Init(&EXTI_InitStruct);
    
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = EXTI15_10_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

// 中断服务函数：绝不在这里做延时或复杂运算，只置标志位
void EXTI15_10_IRQHandler(void)
{
    if(EXTI_GetFlagStatus(EXTI_Line12) == SET) {
        EXTI_ClearFlag(EXTI_Line12);
        Key_Up_Flag = 1; 
    }
    if(EXTI_GetFlagStatus(EXTI_Line13) == SET) {
        EXTI_ClearFlag(EXTI_Line13);
        Key_Down_Flag = 1; 
    }
    if(EXTI_GetFlagStatus(EXTI_Line14) == SET) {
        EXTI_ClearFlag(EXTI_Line14);
        Key_Mode_Flag = 1; 
    }
}
