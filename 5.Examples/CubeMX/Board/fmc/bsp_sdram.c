#include "bsp_sdram.h"

#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)

#define FMC_COMMAND_TARGET_BANK                  FMC_SDRAM_CMD_TARGET_BANK1

#define SDRAM_TIMEOUT                            ((uint32_t)0x1000)

void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef* hsdram)
{
    FMC_SDRAM_CommandTypeDef Command;

    /* Configure a clock configuration enable command */
    Command.CommandMode            = FMC_SDRAM_CMD_CLK_ENABLE;  // 开启时钟
    Command.CommandTarget          = FMC_COMMAND_TARGET_BANK;
    Command.AutoRefreshNumber      = 1;
    Command.ModeRegisterDefinition = 0;

    HAL_SDRAM_SendCommand(hsdram, &Command, SDRAM_TIMEOUT);
    HAL_Delay(1);

    /* Configure a PALL (precharge all) command */
    Command.CommandMode            = FMC_SDRAM_CMD_PALL;  // 预充电
    Command.CommandTarget          = FMC_COMMAND_TARGET_BANK;
    Command.AutoRefreshNumber      = 1;
    Command.ModeRegisterDefinition = 0;

    HAL_SDRAM_SendCommand(hsdram, &Command, SDRAM_TIMEOUT);

    /* Configure a Auto-Refresh command */
    Command.CommandMode            = FMC_SDRAM_CMD_AUTOREFRESH_MODE;  // 自动刷新
    Command.CommandTarget          = FMC_COMMAND_TARGET_BANK;
    Command.AutoRefreshNumber      = 8;  // 刷新次数
    Command.ModeRegisterDefinition = 0;

    HAL_SDRAM_SendCommand(hsdram, &Command, SDRAM_TIMEOUT);

    /* Program the external memory mode register */

    Command.CommandMode            = FMC_SDRAM_CMD_LOAD_MODE;  // 加载模式
    Command.CommandTarget          = FMC_COMMAND_TARGET_BANK;
    Command.AutoRefreshNumber      = 1;
    Command.ModeRegisterDefinition = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_2 |
                                     SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL |
                                     SDRAM_MODEREG_CAS_LATENCY_3 |
                                     SDRAM_MODEREG_OPERATING_MODE_STANDARD |
                                     SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

    HAL_SDRAM_SendCommand(hsdram, &Command, SDRAM_TIMEOUT);

    HAL_SDRAM_ProgramRefreshRate(hsdram, 918);  // 刷新率
}

void SDRAM_ReadWords(uint16_t* pBuffer, uint32_t Offset, uint32_t Len)
{
    uint16_t*      pDst = &pBuffer[0];
    __IO uint16_t* pSrc = &SDRAM_Word(Offset);

    while (Len--)
    {
        *pDst = *pSrc;
    }
}

void SDRAM_WriteWords(uint16_t* pBuffer, uint32_t Offset, uint32_t Len)
{
    uint16_t*      pSrc = &pBuffer[0];
    __IO uint16_t* pDst = &SDRAM_Word(Offset);

    while (Len--)
    {
        *pDst = *pSrc;
    }
}
