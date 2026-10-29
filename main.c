#include "stm32f10x.h"
#include "boardLED.h"
#include "delay.h"
#include "usart.h"
#include "TIMdelay.h"
#include "SR04.h"
#include "OLED.h"
#include "OLED_Data.h"
#include "button.h"
#include "exti.h"
#include "spi.h"
#include <stdio.h>

#define N 5

// ?? 如果还是以前的数据，请确保第一次烧录时 FORMAT_FLASH 为 1，清空后再改回 0
#define FORMAT_FLASH 0 

typedef enum { MODE_REALTIME = 0, MODE_HISTORY } SystemMode_t;
SystemMode_t sys_mode = MODE_REALTIME; 
uint16_t history_page_index = 0;       
static char str_buf[64]; 

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    MY_OnBoardLED_Init();
    delay_Init();
    MY_USART1_Init();
    TIM2_TimeBaseInit();
    SR04_Init();
    OLED_Init();
    button_init();
    MY_EXTI_Init();
    MY_SPI_Init();
    
#if FORMAT_FLASH == 1
    W25Q_EraseSector(0);
    W25Q_EraseSector(4096);
    W25Q_EraseSector(8192);
    W25Q_EraseSector(12288);
    W25Q_EraseSector(16384); 
    printf("Flash Formatted!\r\n");
#endif

    Flash_Log_Init(); 
    
    uint8_t count = 0;
    uint8_t level = 0;          
    uint32_t led_period = 500;
    uint32_t last_trig = 0, last_led = 0, last_oled = 0;
    uint8_t led_on = 0;
    float mini = 0.2f; 
    
    float temp_max = max; 
    Flash_Config_Load(&temp_max);
    max = temp_max; 
    
    float dis[N], sum = 0, avg = 0;
    
    while(1)
    {		
        uint32_t now = gettick();
        
        if(Key_Mode_Flag == 1) 
        {
            Key_Mode_Flag = 0; 
            delay_ms(30);
            if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14) == 0) 
            {
                if(sys_mode == MODE_REALTIME) {
                    sys_mode = MODE_HISTORY;
                    Flash_Log_Statistics(); 
                    history_page_index = 0; 
                    OLED_Clear();
                } else {
                    sys_mode = MODE_REALTIME; 
                    OLED_Clear();
                }
                last_oled = 0; 
            }
        }

        if(Key_Up_Flag == 1)
        {
            Key_Up_Flag = 0; 
            delay_ms(30);    
            if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) == 0) 
            {
                if(sys_mode == MODE_REALTIME) {
                    max += 0.1f;
                    if(max > 1.0f) max = 1.0f;
                    
                    // 【恢复原汁原味的打印】
                    printf("Max set to: %.2f\r\n", max); 
                    
                    Flash_Config_Save(max); 
                    
                } else {
                    if(g_total_log_count > 0) {
                        if(history_page_index < g_total_log_count - 1) history_page_index++;
                        else history_page_index = 0; 
                    }
                    last_oled = 0; 
                }
            }
        }

        if(Key_Down_Flag == 1)
        {
            Key_Down_Flag = 0; 
            delay_ms(30);      
            if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13) == 0) 
            {
                if(sys_mode == MODE_REALTIME) {
                    max -= 0.1f;
                    if(max < 0.2f) max = 0.2f;
                    
                    // 【恢复原汁原味的打印】
                    printf("Max set to: %.2f\r\n", max); 
                    
                    Flash_Config_Save(max); 
                    
                } else {
                    if(g_total_log_count > 0) {
                        if(history_page_index > 0) history_page_index--;
                        else history_page_index = g_total_log_count - 1; 
                    }
                    last_oled = 0; 
                }
            }
        }

        if(sys_mode == MODE_REALTIME)
        {
            if(count < N && now - last_trig >= 100)
            {
                last_trig = now;
                float d = Distance_test();
                if(d > 0.0f) { dis[count++] = d; }
            }
            
            if(count >= 5) 
            {
                for(uint8_t i = 0; i < count - 1; i++)
                    for(uint8_t j = 0; j < count - 1 - i; j++)
                        if(dis[j] < dis[j+1]) { float t = dis[j]; dis[j] = dis[j+1]; dis[j+1] = t; }
                
                sum = 0;
                for (uint8_t i = 1; i < count - 1; i++) sum += dis[i];
                avg = sum / (count - 2);
                
                if(avg < mini) { 
                    level = 1; led_period = 1000; 
                    Flash_Log_Write(avg, now); 
                }
                else if(avg >= mini && avg < max) { 
                    level = 2; led_period = 500; 
                }
                else if(avg >= max) { 
                    level = 3; led_period = 200; 
                    Flash_Log_Write(avg, now); 
                }
                count = 0;
            }
            
            if(now - last_oled >= 200)
            {
                last_oled = now;
                OLED_Clear();
                OLED_ShowString(0, 0, "Real Time:", OLED_8X16);
                OLED_ShowFloatNum(0, 16, avg, 1, 4, OLED_8X16);
                OLED_Update();
            }
        }
        else if (sys_mode == MODE_HISTORY)
        {
            if(now - last_oled >= 100) 
            {
                last_oled = now;
                OLED_Clear();
                
                if(g_total_log_count == 0) {
                    OLED_ShowString(0, 0, "No History!", OLED_8X16);
                } else {
                    float hist_dist; uint32_t hist_time;
                    if(Flash_Log_Get_By_Index(history_page_index, &hist_dist, &hist_time))
                    {
                        // 【顺带优化】既然开了 MicroLIB，OLED 的 sprintf 也可以直接用 %.2f 了，极其清爽！
                        sprintf(str_buf, "Max: %.2f", g_max_dist);
                        OLED_ShowString(0, 0, str_buf, OLED_6X8);
                        
                        sprintf(str_buf, "Min: %.2f", g_min_dist);
                        OLED_ShowString(0, 10, str_buf, OLED_6X8);

                        sprintf(str_buf, "T:%d P:%d", g_total_log_count, history_page_index + 1);
                        OLED_ShowString(72, 0, str_buf, OLED_6X8);
                        
                        OLED_ShowString(0, 24, "Dist:", OLED_8X16);
                        OLED_ShowFloatNum(48, 24, hist_dist, 1, 4, OLED_8X16);
                        
                        if(hist_dist < mini) OLED_ShowString(0, 44, "State: CLOSE", OLED_8X16);
                        else if(hist_dist >= mini && hist_dist < max) OLED_ShowString(0, 44, "State: NORMAL", OLED_8X16);
                        else OLED_ShowString(0, 44, "State: FAR", OLED_8X16);
                    } else {
                        OLED_ShowString(0, 24, "Read Error!", OLED_8X16);
                    }
                }
                OLED_Update();
            }
        }
        
        if(level > 0 && now - last_led >= led_period / 2 && sys_mode == MODE_REALTIME)
        {
            last_led = now;
            led_on = !led_on;
            if(led_on) LED_ON(); else LED_OFF();
        }
    }
}
