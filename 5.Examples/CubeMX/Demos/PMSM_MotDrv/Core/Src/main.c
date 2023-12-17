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
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_cdc_if.h"
#include "bsp.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(*arr))
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

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
ALIGN_32BYTES(uint16_t ADC1_Conv[4]);
ALIGN_32BYTES(uint16_t ADC2_Conv[2]);

#define AD_MOT_UE  ADC1_Conv[0]
#define AD_MOT_VE  ADC1_Conv[1]
#define AD_MOT_WE  ADC1_Conv[2]

#define AD_MOT_CUR ADC1_Conv[3]  // current feedback

#define AD_POT     ADC2_Conv[0]

//

//

typedef struct {
    uint32_t DataPre;
    uint32_t DataCur;
    uint32_t Th;
} LevelCheck_t;

typedef enum {
    LEVEL_CHECK_RISING_EDGE,
    LEVEL_CHECK_FALLING_EDGE,
    LEVEL_CHECK_KEEP_LOW,
    LEVEL_CHECK_KEEP_HIGH,
} LevelCheck_e;

LevelCheck_e LevelCheck(LevelCheck_t* p)
{
    LevelCheck_e res;

    if (p->DataCur < p->Th)
    {
        if (p->DataPre < p->Th)
        {
            res = LEVEL_CHECK_KEEP_LOW;
        }
        else
        {
            res = LEVEL_CHECK_FALLING_EDGE;
        }
    }
    else
    {
        if (p->DataPre > p->Th)
        {
            res = LEVEL_CHECK_KEEP_HIGH;
        }
        else
        {
            res = LEVEL_CHECK_RISING_EDGE;
        }
    }

    p->DataPre = p->DataCur;

    return res;
}

//

#include "motdrv.h"

#include "motdrv/foc/svpwm.h"
#include "motdrv/enc/hall.h"

void svgen_teset(float32_t iq, float32_t MechAngle)
{
    MotorInfo_t motor;
    foc_t       foc;

    motor.PolePairs = 4;
    motor.Umdc      = 11.2;
    motor.DutyMax   = htim1.Init.Period + 1;
    motor.CarryFreq = 8000;

    foc.d = 0;
    foc.q = iq;

    foc.theta = MechAngle * motor.PolePairs;  // elecAngle

    // 查表法要取余

    foc.sin = sin(foc.theta);
    foc.cos = cos(foc.theta);

    ipark(&foc);
    iclarke(&foc);
    ph_order(&foc);

    float32_t V0 = -0.5 * (foc.phase_min + foc.phase_max);

    foc.Ta = (foc.phase_a + V0) / motor.Umdc;
    foc.Tb = (foc.phase_b + V0) / motor.Umdc;
    foc.Tc = (foc.phase_c + V0) / motor.Umdc;

#if 1
    foc.Ta *= -1;
    foc.Tb *= -1;
    foc.Tc *= -1;
#endif

    foc.Ta += 0.5;
    foc.Tb += 0.5;
    foc.Tc += 0.5;

    foc.Ta *= motor.DutyMax;
    foc.Tb *= motor.DutyMax;
    foc.Tc *= motor.DutyMax;

    PWM_SetDuty(foc.Ta, foc.Tb, foc.Tc);
}

void openloop()
{
    static float32_t MechAngle = 0;

#if 0


        static uint16_t v = 0;

        if (AD_POT > 48000)
        {
            if (v == 0)
            {
                PWM_Start();

                v = 1;
            }
            else
            {
                MechAngle += 0.05;
                if (MechAngle >= 6.28) { MechAngle = 0; }
                svgen_teset(3, MechAngle);
            }
        }
        else
        {
            if (v == 1)
            {
                v = 0;

                PWM_Stop();
            }
        }

        usb_printf("%d,%d,%d,%d,%d,%d,%d,%f,%d\n", ADC1_Conv[0], ADC1_Conv[1], ADC1_Conv[2], ADC1_Conv[3], ADC2_Conv[0], ADC2_Conv[1], MechAngle * M_RAD2DGE, HallEnc_ReadSector() * 60);

#else

    static LevelCheck_t run_state = {
        .DataCur = 0,
        .DataPre = 0,
        .Th      = 20000,
    };
    run_state.DataCur = AD_POT;
    switch (LevelCheck(&run_state))
    {
        case LEVEL_CHECK_RISING_EDGE:
            usb_printf("start\n");
            PWM_Start();
            break;
        case LEVEL_CHECK_FALLING_EDGE:
            PWM_Stop();
            usb_printf("stop\n");
            break;
        case LEVEL_CHECK_KEEP_LOW:
            break;
        case LEVEL_CHECK_KEEP_HIGH:
            MechAngle += 0.03;
            if (MechAngle >= 6.28) { MechAngle = 0; }
            svgen_teset(3, MechAngle);
            usb_printf("%d,%d,%d,%d,%d,%d,%d,%f,%d\n", ADC1_Conv[0], ADC1_Conv[1], ADC1_Conv[2], ADC1_Conv[3], ADC2_Conv[0], ADC2_Conv[1], MechAngle * M_RAD2DGE, HallEnc_ReadSector() * 60);
            break;
    }

#endif
}

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
    MX_USB_DEVICE_Init();
    MX_TIM1_Init();
    MX_TIM6_Init();
    /* USER CODE BEGIN 2 */
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&ADC1_Conv[0], ARRAY_SIZE(ADC1_Conv));
    HAL_ADC_Start_DMA(&hadc2, (uint32_t*)&ADC2_Conv[0], ARRAY_SIZE(ADC2_Conv));

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */

    while (1)
    {
        HAL_Delay(10);
        LED_TGL();
        SCB_InvalidateDCache_by_Addr((uint32_t*)&ADC1_Conv[0], ARRAY_SIZE(ADC1_Conv));
        SCB_InvalidateDCache_by_Addr((uint32_t*)&ADC2_Conv[0], ARRAY_SIZE(ADC2_Conv));
        openloop();
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
