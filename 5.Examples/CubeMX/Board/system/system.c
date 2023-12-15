#include "system.h"

//-----------------------------------------------------------------------------
//

void DispChipID(void)
{
    uint32_t CPU_Sn0, CPU_Sn1, CPU_Sn2;

    CPU_Sn0 = *(__IO uint32_t*)(0x1FF1E800);
    CPU_Sn1 = *(__IO uint32_t*)(0x1FF1E800 + 4);
    CPU_Sn2 = *(__IO uint32_t*)(0x1FF1E800 + 8);

    printf("CPU : STM32H750XBH6, BGA240, SYSCLK: %d MHz\n", SystemCoreClock / 1000000);
    printf("UID = %08X %08X %08X\n", CPU_Sn2, CPU_Sn1, CPU_Sn0);
}

//-----------------------------------------------------------------------------
//

typedef void (*lpfnSysBoot_t)(void);

void JumpToSysBoot(void)
{
    uint32_t i = 0;

    lpfnSysBoot_t SysMemBootJump;

    // STM32H7 Bootloader Address
    __IO uint32_t BootAddr = 0x1FF09800;

    /* 关闭全局中断 */
    __disable_irq();  // __set_PRIMASK(1)

    /* 关闭滴答定时器，复位到默认值 */
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    /* 设置所有时钟到默认状态，使用HSI时钟 */
    HAL_RCC_DeInit();

    /* 关闭所有中断，清除所有中断挂起标志 */
    for (i = 0; i < 8; i++)
    {
        NVIC->ICER[i] = 0xFFFFFFFF;
        NVIC->ICPR[i] = 0xFFFFFFFF;
    }

    /* 使能全局中断 */
    __enable_irq();  //  __set_PRIMASK(0)

    /* 跳转到系统BootLoader，首地址是MSP，地址+4是复位中断服务程序地址 */
    SysMemBootJump = (lpfnSysBoot_t)(*(uint32_t*)(BootAddr + 4));

    /* 设置主堆栈指针 */
    __set_MSP(*(uint32_t*)BootAddr);

    /* 在RTOS工程，这条语句很重要，设置为特权级模式，使用MSP指针 */
    __set_CONTROL(0);

    /* 跳转到系统BootLoader */
    SysMemBootJump();

    /* 跳转成功的话，不会执行到这里，用户可以在这里添加代码 */
    while (1)
    {
    }
}
