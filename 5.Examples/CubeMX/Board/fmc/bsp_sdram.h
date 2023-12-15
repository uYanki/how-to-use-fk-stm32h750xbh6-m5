#ifndef __FMC_SDRAM_H__
#define __FMC_SDRAM_H__

#include "stm32h7xx_hal.h"

// 需同步配置到 MPU_Config() 中的 BaseAddress
#define SDRAM_ADDR_BASE    0xC0000000

// 用强制内联也会有约20%的性能损失
#define SDRAM_Byte(Offset) *(__IO uint8_t*)(SDRAM_ADDR_BASE + (Offset))
#define SDRAM_Word(Offset) *(__IO uint16_t*)(SDRAM_ADDR_BASE + 2 * (Offset))

void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef* hsdram);
void SDRAM_ReadWords(uint16_t* pBuffer, uint32_t Offset, uint32_t Len);
void SDRAM_WriteWords(uint16_t* pBuffer, uint32_t Offset, uint32_t Len);

#endif
