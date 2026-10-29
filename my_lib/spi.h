#ifndef __SPI__H
#define __SPI__H
#include "stm32f10x.h"

void MY_SPI_Init(void);
void SPI_MasterTransimtReceiveData(SPI_TypeDef *SPIx, uint8_t *pDataTx, uint8_t *pDataRx, uint16_t Size);
void W25Q_WriteEnable(void); 
void W25Q_WaitBusy(void);    
void W25Q_EraseSector(uint32_t addr); 
void W25Q_WritePage(uint32_t addr, uint8_t *pData, uint16_t len); 
void W25Q_ReadData(uint32_t addr, uint8_t *pData, uint16_t len);   

extern uint32_t g_total_log_count; 
extern float g_max_dist, g_min_dist, g_avg_dist; 

void Flash_Log_Init(void); 
void Flash_Log_Write(float distance, uint32_t time); 
void Flash_Log_Statistics(void); 
// 注意：去掉了 level 参数
uint8_t Flash_Log_Get_By_Index(uint16_t index, float *dist, uint32_t *time); 

void Flash_Config_Load(float *p_max);
void Flash_Config_Save(float max_val);
#endif
