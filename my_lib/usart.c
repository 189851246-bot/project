#include "usart.h"

void MY_USART1_Init(void)
{
	//初始化IO引脚
	GPIO_InitTypeDef GPIO_InitStruct;
	//PA9 TX AF_PP 2MHz
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	//PA10 RX IPU 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
	
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	//初始化USART模块
	
	//使能USART模块
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	
	USART_InitTypeDef USART_InitSturct;
	//配置参数
	USART_InitSturct.USART_BaudRate = 115200;//波特率 115200
	USART_InitSturct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//软硬件流控不设置
	USART_InitSturct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//串口通信设置为双向通信
	USART_InitSturct.USART_Parity = USART_Parity_No;//校验位不设置
	USART_InitSturct.USART_StopBits = USART_StopBits_1;//一位停止位
	USART_InitSturct.USART_WordLength = USART_WordLength_8b;//一次传输8比特
	
	USART_Init(USART1,&USART_InitSturct);
	
	//开启USART模块总开关
	USART_Cmd(USART1,ENABLE);

}
//用printf重定向到PA9，PA10
int fputc(int ch, FILE *f)
{
    USART_SendData(USART1, (uint8_t)ch);
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET); //等待发送完成
    return ch;
}
void USART1_SendBytes(USART_TypeDef *USARTx, uint8_t *pData, uint16_t Size)
{
	for(uint16_t i=0;i<Size;i++)
	{
		//发送寄存器空
		/*
		1. **TXE：Transmit Data Register Empty，发送数据寄存器空标志**

			- TXE=SET：发送寄存器为空，可以送入下一个新字节。
			- TXE=RESET：寄存器还有数据，**不能写新数据**，强行写会覆盖丢失旧数据。
			`while`就是阻塞等待，直到寄存器空
		*/
		while(USART_GetFlagStatus(USARTx,USART_FLAG_TXE) == RESET);
		//将要发送的数据写入到发送数据寄存器
		//TC = Transmission Complete **发送完成标志**
		USART_SendData(USARTx,pData[i]);
	}
		//等待数据发送完成
	while(USART_GetFlagStatus(USARTx,USART_FLAG_TC)== RESET);
}


