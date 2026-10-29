#include "spi.h"
#include <stdio.h>
#include <string.h>

void MY_SPI_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    SPI_InitTypeDef SPI_InitStruct;
    SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
    SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
    SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
    SPI_Init(SPI1, &SPI_InitStruct);
    SPI_NSSInternalSoftwareConfig(SPI1, SPI_NSSInternalSoft_Set);
}

void SPI_MasterTransimtReceiveData(SPI_TypeDef *SPIx, uint8_t *pDataTx, uint8_t *pDataRx, uint16_t Size)
{
    uint32_t timeout = 0;
    SPI_Cmd(SPIx, ENABLE);
    SPI_NSSInternalSoftwareConfig(SPIx, SPI_NSSInternalSoft_Set);

    for(uint16_t i = 0; i < Size; i++)
    {
        timeout = 0;
        while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET) {
            if(++timeout > 100000) { SPI_Cmd(SPIx, DISABLE); return; }
        }
        SPI_I2S_SendData(SPIx, pDataTx ? pDataTx[i] : 0xFF);
        timeout = 0;
        while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET) {
            if(++timeout > 100000) { SPI_Cmd(SPIx, DISABLE); return; }
        }
        uint8_t rxData = SPI_I2S_ReceiveData(SPIx);
        if(pDataRx) pDataRx[i] = rxData;
    }
    timeout = 0;
    while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_BSY) == SET) {
         if(++timeout > 100000) break;
    }
    SPI_Cmd(SPIx, DISABLE);
}

void W25Q_WriteEnable(void) {
    uint8_t cmd = 0x06;
    GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_RESET);
    SPI_MasterTransimtReceiveData(SPI1, &cmd, NULL, 1);
    GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);
}

void W25Q_WaitBusy(void) {
    uint8_t cmd, status;
    uint32_t timeout = 0;
    do {
        cmd = 0x05;
        GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_RESET);
        SPI_MasterTransimtReceiveData(SPI1, &cmd, NULL, 1);
        SPI_MasterTransimtReceiveData(SPI1, NULL, &status, 1);
        GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);
        if(++timeout > 500000) break;
    } while(status & 0x01);
}

void W25Q_EraseSector(uint32_t addr) {
    uint8_t cmd[4];
    W25Q_WriteEnable();
    cmd[0] = 0x20; cmd[1] = (addr >> 16) & 0xFF; cmd[2] = (addr >> 8) & 0xFF; cmd[3] = addr & 0xFF;
    GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_RESET);
    SPI_MasterTransimtReceiveData(SPI1, cmd, NULL, 4);
    GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);
    W25Q_WaitBusy();
}

void W25Q_WritePage(uint32_t addr, uint8_t *pData, uint16_t len) {
    uint8_t cmd[4];
    W25Q_WriteEnable();
    cmd[0] = 0x02; cmd[1] = (addr >> 16) & 0xFF; cmd[2] = (addr >> 8) & 0xFF; cmd[3] = addr & 0xFF;
    GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_RESET);
    SPI_MasterTransimtReceiveData(SPI1, cmd, NULL, 4);
    SPI_MasterTransimtReceiveData(SPI1, pData, NULL, len);
    GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);
    W25Q_WaitBusy();
}

void W25Q_ReadData(uint32_t addr, uint8_t *pData, uint16_t len) {
    uint8_t cmd[4];
    cmd[0] = 0x03; cmd[1] = (addr >> 16) & 0xFF; cmd[2] = (addr >> 8) & 0xFF; cmd[3] = addr & 0xFF;
    GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_RESET);
    SPI_MasterTransimtReceiveData(SPI1, cmd, NULL, 4); 
    SPI_MasterTransimtReceiveData(SPI1, NULL, pData, len); 
    GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);
}

#define LOG_START_ADDR   0x000000       
#define FLASH_SECTOR_SIZE 4096          
#define LOG_MAX_SECTORS  4              
#define LOG_END_ADDR     (LOG_START_ADDR + FLASH_SECTOR_SIZE * LOG_MAX_SECTORS) 
#define LOG_MAGIC_NUMBER 0x5A5A5A5A     
#define LOGS_PER_SECTOR  255 

typedef __packed struct {
    uint32_t magic;      // 4
    uint32_t global_seq; // 4
    uint32_t timestamp;  // 4
    float    distance;   // 4
} LogItem_t;             // 16字节

uint32_t g_current_log_addr = LOG_START_ADDR; 
uint32_t g_current_global_seq = 1;            
uint32_t g_total_log_count = 0;
float g_max_dist = 0.0f, g_min_dist = 99.0f, g_avg_dist = 0.0f;

void Flash_Log_Init(void)
{
    g_current_log_addr = LOG_START_ADDR;
    g_current_global_seq = 1;
    
    LogItem_t temp_log;
    // 寻找最后一个有效的数据
    for(uint32_t addr = LOG_START_ADDR; addr < LOG_END_ADDR - 16; addr += 16) {
        W25Q_ReadData(addr, (uint8_t*)&temp_log, sizeof(LogItem_t));
        if(temp_log.magic == LOG_MAGIC_NUMBER && temp_log.distance >= 0.0f && temp_log.distance <= 10.0f) {
            g_current_global_seq = temp_log.global_seq + 1;
        }
    }
}

void Flash_Log_Write(float distance, uint32_t time)
{
    LogItem_t new_log;
    uint8_t check_byte;

    // 简单粗暴的防跨页和擦除逻辑
    if((g_current_log_addr % FLASH_SECTOR_SIZE) == 0) {
        // 换扇区了，先擦
        W25Q_EraseSector(g_current_log_addr);
    } else {
        W25Q_ReadData(g_current_log_addr, &check_byte, 1);
        if(check_byte != 0xFF) {
            // 被写过了，强制换下一个扇区
            g_current_log_addr = (g_current_log_addr & ~(FLASH_SECTOR_SIZE - 1)) + FLASH_SECTOR_SIZE;
            if(g_current_log_addr >= LOG_END_ADDR) g_current_log_addr = LOG_START_ADDR;
            W25Q_EraseSector(g_current_log_addr);
        }
    }
    
    if ((g_current_log_addr % 256) + 16 > 256) {
        g_current_log_addr = (g_current_log_addr & ~0xFF) + 256;
    }

    new_log.magic = LOG_MAGIC_NUMBER;
    new_log.global_seq = g_current_global_seq++;
    new_log.timestamp = time;
    new_log.distance = distance;

    W25Q_WritePage(g_current_log_addr, (uint8_t*)&new_log, sizeof(LogItem_t));
    g_current_log_addr += sizeof(LogItem_t);
}

void Flash_Log_Statistics(void)
{
    LogItem_t temp_log;
    float sum = 0.0f;
    g_total_log_count = 0; 
    g_max_dist = 0.0f;
    g_min_dist = 99.0f;
    
    for(uint32_t addr = LOG_START_ADDR; addr < LOG_END_ADDR - 16; addr += 16) {
        W25Q_ReadData(addr, (uint8_t*)&temp_log, sizeof(LogItem_t));
        if(temp_log.magic == LOG_MAGIC_NUMBER && temp_log.distance >= 0.0f && temp_log.distance <= 10.0f) {
            g_total_log_count++;
            sum += temp_log.distance;
            if(temp_log.distance > g_max_dist) g_max_dist = temp_log.distance;
            if(temp_log.distance < g_min_dist) g_min_dist = temp_log.distance;
        }
    }
    if(g_total_log_count > 0) g_avg_dist = sum / g_total_log_count;
}

// 彻底放弃计算，全盘暴力寻找！
uint8_t Flash_Log_Get_By_Index(uint16_t index, float *dist, uint32_t *time)
{
    LogItem_t temp_log;
    uint32_t target_seq = (g_current_global_seq - 1) - index;
    if (target_seq < 1) return 0;
    
    for(uint32_t addr = LOG_START_ADDR; addr < LOG_END_ADDR - 16; addr += 16) {
        W25Q_ReadData(addr, (uint8_t*)&temp_log, sizeof(LogItem_t));
        // 必须校验 magic、合法数值，以及序号！
        if(temp_log.magic == LOG_MAGIC_NUMBER && 
           temp_log.distance >= 0.0f && temp_log.distance <= 10.0f && 
           temp_log.global_seq == target_seq) 
        {
            *dist = temp_log.distance;
            *time = temp_log.timestamp;
            return 1; 
        }
    }
    return 0; 
}
/* ================== 仅 Max 阈值掉电保存 ================== */
#define CONFIG_START_ADDR  0x004000 // 第 5 个扇区（避开日志区）
#define CONFIG_MAGIC       0x12345678 

// 配置结构体（只存防伪码和 max）
typedef __packed struct {
    uint32_t magic;      
    float max_val;       
} Config_t;

void Flash_Config_Load(float *p_max)
{
    Config_t cfg;
    W25Q_ReadData(CONFIG_START_ADDR, (uint8_t*)&cfg, sizeof(Config_t));
    
    // 防伪校验和合理范围（0.1~1.5米）
    if(cfg.magic == CONFIG_MAGIC && cfg.max_val >= 0.1f && cfg.max_val <= 1.5f) {
        *p_max = cfg.max_val;
        printf("Config Loaded OK!\r\n");
    } else {
        printf("No Config, use Default.\r\n");
    }
}

void Flash_Config_Save(float max_val)
{
    Config_t cfg;
    cfg.magic = CONFIG_MAGIC;
    cfg.max_val = max_val;
    
    W25Q_EraseSector(CONFIG_START_ADDR);
    W25Q_WritePage(CONFIG_START_ADDR, (uint8_t*)&cfg, sizeof(Config_t));
    printf("Config Saved!\r\n");
}


