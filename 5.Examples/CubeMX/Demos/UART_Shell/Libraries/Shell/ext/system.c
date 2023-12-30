#include "./system.h"
#include "../core.h"

//-----------------------------------------------------------------------------

int system_show_info(shell_t* shell, int argc, char* argv[])
{
    // clang-format off
    shell_printf(shell, "Build time: " __DATE__ " - "__TIME__ "\r\n");
    // clang-format on

    shell_printf(shell, "CPU ID: 0x%08X\r\n", HAL_GetCurrentCPUID());
    shell_printf(shell, "Device ID: 0x%08X\r\n", HAL_GetDEVID());
    shell_printf(shell, "Revision ID: 0x%08X\r\n", HAL_GetREVID());
    shell_printf(shell, "Unique ID (in hex): %08X - %08X - %08X\r\n", HAL_GetUIDw0(), HAL_GetUIDw1(), HAL_GetUIDw2());

    //

    shell_printf(shell, "System Clock Freq = %d Hz\r\n", HAL_RCC_GetSysClockFreq());
    shell_printf(shell, "HCLK Freq = %d Hz\r\n", HAL_RCC_GetHCLKFreq());
    shell_printf(shell, "PCLK1 Freq = %d Hz\r\n", HAL_RCC_GetPCLK1Freq());
    shell_printf(shell, "PCLK2 Freq = %d Hz\r\n", HAL_RCC_GetPCLK2Freq());

    //

    shell_printf(shell, "D1 PCLK Freq = %d Hz\r\n", HAL_RCCEx_GetD1PCLK1Freq());
    shell_printf(shell, "D3 PCLK Freq = %d Hz\r\n", HAL_RCCEx_GetD3PCLK1Freq());

    //

    PLL1_ClocksTypeDef PLL1;
    PLL2_ClocksTypeDef PLL2;
    PLL3_ClocksTypeDef PLL3;

    HAL_RCCEx_GetPLL1ClockFreq(&PLL1);
    HAL_RCCEx_GetPLL2ClockFreq(&PLL2);
    HAL_RCCEx_GetPLL3ClockFreq(&PLL3);

    shell_printf(shell, "PLL1 Freq: P = %d Hz, Q = %d Hz, R = %d Hz\r\n", PLL1.PLL1_P_Frequency, PLL1.PLL1_Q_Frequency, PLL1.PLL1_R_Frequency);
    shell_printf(shell, "PLL2 Freq: P = %d Hz, Q = %d Hz, R = %d Hz\r\n", PLL2.PLL2_P_Frequency, PLL2.PLL2_Q_Frequency, PLL2.PLL2_R_Frequency);
    shell_printf(shell, "PLL3 Freq: P = %d Hz, Q = %d Hz, R = %d Hz\r\n", PLL3.PLL3_P_Frequency, PLL3.PLL3_Q_Frequency, PLL3.PLL3_R_Frequency);

    //

    shell_printf(shell, "Periph Clock \r\n");
    shell_printf(shell, " - SAI1 Freq = %d Hz\r\n", HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SAI1));
    shell_printf(shell, " - SAI23 Freq = %d Hz\r\n", HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SAI23));
    // shell_printf(shell, " - SAI2A Freq = %d Hz\r\n", HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SAI2A));
    // shell_printf(shell, " - SAI2B Freq = %d Hz\r\n", HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SAI2B));
    shell_printf(shell, " - SAI4A Freq = %d Hz\r\n", HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SAI4A));
    shell_printf(shell, " - SAI4B Freq = %d Hz\r\n", HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SAI4B));
    shell_printf(shell, " - SPI123 Freq = %d Hz\r\n", HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SPI123));
    shell_printf(shell, " - ADC Freq = %d Hz\r\n", HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_ADC));
    shell_printf(shell, " - SDMMC Freq = %d Hz\r\n", HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SDMMC));
    shell_printf(shell, " - SPI6 Freq = %d Hz\r\n", HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SPI6));

    return 0;
}

CMD_EXPORT_FUNC(CMD_TYPE(CMD_TYPE_FUNC_MAIN), sysinfo, system_show_info, get system info)

//-----------------------------------------------------------------------------

int system_reset(shell_t* shell, int argc, char* argv[])
{
    NVIC_SystemReset();
    return 0;
}

CMD_EXPORT_FUNC(CMD_TYPE(CMD_TYPE_FUNC_MAIN), sysrst, system_reset, reboot system)
CMD_EXPORT_FUNC(CMD_TYPE(CMD_TYPE_FUNC_MAIN), reboot, system_reset, reboot system)

//-----------------------------------------------------------------------------

int system_show_runtime(shell_t* shell, int argc, char* argv[])
{
    uint32_t ms = HAL_GetTick();
    shell_printf(shell, "%d.%03d (s.ms)\r\n", /* s */ ms / 1000, /* ms */ ms % 1000);
    return 0;
}

CMD_EXPORT_FUNC(CMD_TYPE(CMD_TYPE_FUNC_MAIN), systime, system_show_runtime, get system runtime)