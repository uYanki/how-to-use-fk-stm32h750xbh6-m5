/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_cdc_if.h"

#include "bsp.h"

#include "motdrv/enc/hall.h"
#include "motdrv/foc/motdrv.h"

#include "mb.h"
#include "mbport.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void        SystemClock_Config(void);
void        PeriphCommonClock_Config(void);
static void MPU_Config(void);
/* USER CODE BEGIN PFP */
void        SpeedCalc(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

#define HVBUS_COEFF   (40.2157f)  // 单位 V, 计算公式 (5.1k+200k) / (5.1k)
#define CURRENT_COEFF (2.32558f)  // 单位 A, 放大倍数 A = (33K/10K+1), 采样电阻 Rshunt = 100mR, 计算公式 1/4.3/0.1

extern void Modbus_StartReceive(void);

ALIGN_32BYTES(uint16_t ADC1_Conv[4]);
ALIGN_32BYTES(uint16_t ADC2_Conv[2]);

#define AD_MOT_UE    ADC1_Conv[0]
#define AD_MOT_VE    ADC1_Conv[1]
#define AD_MOT_WE    ADC1_Conv[2]

#define AD_MOT_CUR_U ADC2_Conv[0]  // current feedback
#define AD_MOT_CUR_V ADC2_Conv[1]  // current feedback

#define AD_POT       ADC1_Conv[3]

// https://www.zhihu.com/question/600128914
// Speed_ElecAng_Rad =  PolePair * 2 * PI * Speed_ElecAng_RPM / 60

#define I_COEFF    1 / 65535.f * 3300 / 4.3 / 0.1        // mA, 0.1R
#define VBUS_COEFF 1 / 65535.f * 33 / 5.1 * (5.1 + 200)  // 0.1V

void cur_rs2()
{
    ParaTbl.s16CurPhAFb = ADC1_Conv[0];
    ParaTbl.s16CurPhBFb = ADC1_Conv[1];
}

void cur_rs()
{
    SCB_InvalidateDCache_by_Addr((uint32_t*)&ADC2_Conv[0], ARRAY_SIZE(ADC2_Conv));

    uint16_t Iz[2] = {
        ((float32_t)AD_MOT_CUR_U / 65535 * 3300 - 1000) / 4 / 0.1,  // mA
        ((float32_t)AD_MOT_CUR_V / 65535 * 3300 - 1000) / 4 / 0.1,  // mA
    };

    void current_reconstruct(uint16_t Iz[2]);
    current_reconstruct(Iz);  // 电流重构
}

BEMF_Speed_Struct bemf = {
    .u8BEMFPoleNumber               = 2,
    .u16BEMF1msCounter              = 0,
    .u16BEMFSpeed                   = 0,
    .u16BEMFPhaseABMiddlePoint12bit = 0,
    .u8BEMFDirectionFlag            = BEMF_DIR_STANDSTILL,
    .u16BEMFDetectionTime           = 0,
    .bBEMFResultValidFlag           = 0,
    .bBEMFMotorIsRotatingFlag       = 0,
    .u16BEMFComparatorHystersis     = ((BEMF_COMPARATOR_HYSTERESIS * 4) / 5),
    .u16BEMFStandstillThresholdVolt = ((BEMF_STANDSTILL_THRESHOLD_VOLTAGE * 4) / 5),
};

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MPU Configuration--------------------------------------------------------*/
    MPU_Config();
    /* Enable the CPU Cache */

    /* Enable I-Cache---------------------------------------------------------*/
    SCB_EnableICache();

    /* Enable D-Cache---------------------------------------------------------*/
    SCB_EnableDCache();

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* Configure the peripherals common clocks */
    PeriphCommonClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_ADC1_Init();
    MX_ADC2_Init();
    MX_TIM1_Init();
    MX_TIM6_Init();
    MX_TIM8_Init();
    MX_USART1_UART_Init();
    MX_USB_DEVICE_Init();
    /* USER CODE BEGIN 2 */

    DelayInit();

    const uint8_t ucSlaveID[] = {0xAA, 0xBB, 0xCC};
    eMBInit(MB_RTU, 1, 1, 115200, MB_PAR_EVEN);
    eMBSetSlaveID(0x34, true, ucSlaveID, ARRAY_SIZE(ucSlaveID));
    eMBEnable();
    Modbus_StartReceive();

    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&ADC1_Conv[0], ARRAY_SIZE(ADC1_Conv));
    HAL_ADC_Start_DMA(&hadc2, (uint32_t*)&ADC2_Conv[0], ARRAY_SIZE(ADC2_Conv));

    HallEnc_Init(&HallEnc);
    HAL_TIM_Base_Start_IT(TIM_HALL);

    ParaTbl_Init();

    // DbgTbl.u16Buf[0] = 2;
    // DbgTbl.u16Buf[1] = 3;
    // DbgTbl.u16Buf[2] = 4;
    // DbgTbl.u16Buf[3] = 5;
    // DbgTbl.u16Buf[4] = 6;
    // DbgTbl.u16Buf[5] = 1;

    // ParaTbl.u16FocMode = FOC_MODE_MCU;

    ParaTbl.u16ElecAngSrc = ELEC_ANG_SRC_HALL;  // closeloop
    // ParaTbl.u16ElecAngSrc = ELEC_ANG_SRC_NONE;  // openloop

    ParaTbl.u16PwmDutyMax   = htim1.Init.Period + 1;
    ParaTbl.u16MotPolePairs = 2;
    ParaTbl.u16Umdc         = (int)11.2;
    ParaTbl.u16CarryFreq    = 16000;

    if (ParaTbl.u16ElecAngSrc == ELEC_ANG_SRC_NONE)  // zero inject
    {
        // ParaTbl.s16VqRef       = 32000;
        // ParaTbl.s16VdRef       = 0;
        // ParaTbl.u16ElecAngInc  = 24;
        // ParaTbl.u16WaitTimeInc = 8;

        ParaTbl.s16VqRef       = 8000;
        ParaTbl.s16VdRef       = 0;
        ParaTbl.u16ElecAngInc  = 4;
        ParaTbl.u16WaitTimeInc = 40;
    }
    else  // svpwm7
    {
        ParaTbl.u16ElecOffset = HALL_ANGLE_30 * 2;
        ParaTbl.s16VqRef      = 16000;
        ParaTbl.s16VdRef      = 0;
    }

    ParaTbl.u16RunState = 1;

    // Hall as Inc
    ParaTbl.u32EncRes    = 6 * ParaTbl.u16MotPolePairs;
    ParaTbl.s16SpdDigRef = 100;

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */

    while (1)
    {
        eMBPoll();

        //-------------------------------------------------

        static uint32_t tBlink = 0;

        if (DelayNonBlockMS(tBlink, 500))
        {
            tBlink = DelayNonGetTick();
            LED_TGL();
        }

        //-------------------------------------------------

        static uint32_t tAdTask = 0;

        if (DelayNonBlockMS(tAdTask, 1))
        {
            // extern volatile int n;

            // usb_printf("%d,%d,%d,%d,%d\n", ParaTbl.s16CurPhAFb, ParaTbl.s16CurPhBFb, ParaTbl.s16CurPhCFb, AD_MOT_CUR_U, AD_MOT_CUR_V);
            // usb_printf("%d,%d,%d,%d,%d,%d,%d,%d\n", ParaTbl.s16CurPhAFb, ParaTbl.s16CurPhBFb, ParaTbl.s16CurPhCFb, ParaTbl.s16IdFb, ParaTbl.s16IqFb, HallEnc.HallState, ParaTbl.u16Sector, DbgTbl.u16Buf[0]);

            // usb_printf("%d,%d,%d,%d,%d,%d\n",
            //            (s16)DbgTbl.u16Buf[0],
            //            (s16)DbgTbl.u16Buf[1],
            //            (s16)DbgTbl.u16Buf[2],
            //            (s16)DbgTbl.u16Buf[3],
            //            (s16)DbgTbl.u16Buf[4],
            //            (s16)DbgTbl.u16Buf[5]);

            // https://www.zhihu.com/question/606311981

            BEMF_Speed_Detect(&bemf, AD_MOT_UE >> 4, AD_MOT_VE >> 4);
            // usb_printf("%d,%f,%f\n", bemf.u16BEMFSpeed, AD_MOT_UE / 65535.f * 3.3, AD_MOT_VE / 65535.f * 3.3);

            // 反电动势与磁链关系 https://blog.csdn.net/qq_38190041/article/details/110713520
            // https://blog.csdn.net/qq_38190041/article/details/110848202

            // matlab 反电动势仿真 https://blog.csdn.net/weixin_42362056/article/details/116766351
            // Kt=9.55Ke, 9.55=60/2/pi

            float32_t e = AD_MOT_UE / 65535.f * 3.3;
            usb_printf("%f,%f,%f\n", e, 30 * e / 3.1415926 / ParaTbl.s16SpdFb / ParaTbl.u16MotPolePairs, ParaTbl.u16MotPolePairs * ParaTbl.s16SpdFb * 0.028595 * 2 * 3.14 / 60);
            // e_max = 0.627773, wb = 0.028595

            // usb_printf("%d,%d,%d,%d\n", AD_MOT_UE, AD_MOT_VE, AD_MOT_WE, HallEnc.HallState);

            ParaTbl.s16AiU = AD_POT * 3300 / 65535 + ParaTbl.s16AiBias;

            tAdTask = DelayNonGetTick();
        }

        SCB_InvalidateDCache_by_Addr((uint32_t*)&ADC1_Conv[0], ARRAY_SIZE(ADC1_Conv));

        //-------------------------------------------------

        SpeedCalc();

        static u16 u16RunStatePre = 0;

        if (u16RunStatePre != ParaTbl.u16RunState)
        {
            u16RunStatePre = ParaTbl.u16RunState;
            ParaTbl.u16RunState ? PWM_Start() : PWM_Stop();
        }

        //-------------------------------------------------

        ifoc();

        if (ParaTbl.u16RunState)
        {
            if (ParaTbl.u16ElecAngSrc == ELEC_ANG_SRC_NONE)
            {
                ParaTbl.u16ElecAngRef += ParaTbl.u16ElecAngInc;

                // if (ParaTbl.u16ElecAngRef == 0)
                // {
                //     DbgTbl.u16Buf[0]++;
                // }

                //  usb_printf("%d,%d\n", ParaTbl.u16ElecAngRef, ParaTbl.u16HallAngFb);  // 霍尔偏置 ParaTbl.u16ElecOffset
            }
            else if (ParaTbl.u16ElecAngSrc == ELEC_ANG_SRC_HALL)
            {
                ParaTbl.u16ElecAngRef = ((u32)ParaTbl.u16HallAngFb + (u32)ParaTbl.u16ElecOffset) % UINT16_MAX;
            }

            spdloop();
            curloop();
            ofoc();

            if (ParaTbl.u16ElecAngSrc == ELEC_ANG_SRC_NONE)
            {
                DelayBlockUS(ParaTbl.u16WaitTimeInc);
            }
        }

        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Supply configuration update enable
     */
    HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

    /** Configure the main internal regulator output voltage
     */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

    while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM       = 5;
    RCC_OscInitStruct.PLL.PLLN       = 192;
    RCC_OscInitStruct.PLL.PLLP       = 2;
    RCC_OscInitStruct.PLL.PLLQ       = 20;
    RCC_OscInitStruct.PLL.PLLR       = 2;
    RCC_OscInitStruct.PLL.PLLRGE     = RCC_PLL1VCIRANGE_2;
    RCC_OscInitStruct.PLL.PLLVCOSEL  = RCC_PLL1VCOWIDE;
    RCC_OscInitStruct.PLL.PLLFRACN   = 0;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.SYSCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
    RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief Peripherals Common Clock Configuration
 * @retval None
 */
void PeriphCommonClock_Config(void)
{
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    /** Initializes the peripherals clock
     */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInitStruct.PLL2.PLL2M           = 20;
    PeriphClkInitStruct.PLL2.PLL2N           = 160;
    PeriphClkInitStruct.PLL2.PLL2P           = 2;
    PeriphClkInitStruct.PLL2.PLL2Q           = 2;
    PeriphClkInitStruct.PLL2.PLL2R           = 2;
    PeriphClkInitStruct.PLL2.PLL2RGE         = RCC_PLL2VCIRANGE_0;
    PeriphClkInitStruct.PLL2.PLL2VCOSEL      = RCC_PLL2VCOWIDE;
    PeriphClkInitStruct.PLL2.PLL2FRACN       = 0;
    PeriphClkInitStruct.AdcClockSelection    = RCC_ADCCLKSOURCE_PLL2;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */

// placed within the function will be optimized, even if volatile、__USED、__attribute__((optimize("O0"))) is added
static tick_t tSpdCalc = 0;

void SpeedCalc(void)
{
#if CONFIG_ENCODER_TYPE == ENC_HALL

    ParaTbl.u16HallAngFb = HallEnc.ElecAngle;

    // T 测�?�法：脉冲间�? (适用于低�?)

    if (DelayNonBlockMS(tSpdCalc, 1000))  // M 测�?�法: �?定时间内脉冲�? (适用于高�?)
    {
        // ParaTbl.s32EncPos += HallEnc.EdgeCount;
        // ParaTbl.u32EncTurns += ParaTbl.u32EncRes / ParaTbl.u32EncRes;
        // ParaTbl.s32EncPos %= ParaTbl.u32EncRes;

        ParaTbl.s16SpdFb  = 60.f * HallEnc.EdgeCount / ParaTbl.u16MotPolePairs / 6;
        HallEnc.EdgeCount = 0;

        tSpdCalc = DelayNonGetTick();
    }

#endif
}

void EncCycle()
{
    if (ParaTbl.u16EncTurnClr != 0)
    {
        ParaTbl.u16EncTurnClr = 0;
        ParaTbl.u32EncTurns   = 0;
    }
}

/* USER CODE END 4 */

/* MPU Configuration */

void MPU_Config(void)
{
    MPU_Region_InitTypeDef MPU_InitStruct = {0};

    /* Disables the MPU */
    HAL_MPU_Disable();

    /** Initializes and configures the Region and the memory to be protected
     */
    MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
    MPU_InitStruct.Number           = MPU_REGION_NUMBER0;
    MPU_InitStruct.BaseAddress      = 0x0;
    MPU_InitStruct.Size             = MPU_REGION_SIZE_4GB;
    MPU_InitStruct.SubRegionDisable = 0x87;
    MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
    MPU_InitStruct.IsShareable      = MPU_ACCESS_SHAREABLE;
    MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;

    HAL_MPU_ConfigRegion(&MPU_InitStruct);
    /* Enables the MPU */
    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1)
    {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
