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
#include "bdma.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bsp.h"
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
static void MPU_Config(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* 方便Cache类的API操作，做32字节对齐 */
#if defined(__ICCARM__)
#pragma location = 0x38000000
uint16_t aAdVals[4];
#elif defined(__CC_ARM)
ALIGN_32BYTES(__attribute__((section(".RAM_D3"))) uint16_t aAdVals[4]);
#endif
// ALIGN_32BYTES(__attribute__((section(".RAM_D3"))) uint16_t aAdVals[4]);

// ALIGN_32BYTES(static uint16_t aAdVals[4])
//__attribute__((section(".ARM.__at_0x24000000")));

/* 选择ADC的时钟源 */
// #define ADC_CLOCK_SOURCE_AHB     /* 选择AHB时钟源 */
#define ADC_CLOCK_SOURCE_PLL /* 选择PLL时钟源 */

/* 方便Cache类的API操作，做32字节对齐 */
#if defined(__ICCARM__)
#pragma location = 0x38000000
uint16_t ADCxValues[4];
#elif defined(__CC_ARM)
ALIGN_32BYTES(__attribute__((section(".RAM_D3"))) uint16_t ADCxValues[4]);
#endif

//__attribute__((at(0x38000080))) uint16_t ADCxValues[2]={0x00,0x00};

ALIGN_32BYTES(static uint16_t ADCxValues[4])
__attribute__((section(".ARM.0x38000080")));
DMA_HandleTypeDef DMA_Handle = {0};

void bsp_GetAdcValues(void)
{
    float    AdcValues[5];
    uint16_t TS_CAL1;
    uint16_t TS_CAL2;

    /*
       使用此函数要特别注意，第1个参数地址要32字节对齐，第2个参数要是32字节的整数倍
    */
    SCB_InvalidateDCache_by_Addr((uint32_t*)ADCxValues, sizeof(ADCxValues));
    AdcValues[0] = ADCxValues[0] * 3.3 / 65536;
    AdcValues[1] = ADCxValues[1] * 3.3 / 65536;
    AdcValues[2] = ADCxValues[2] * 3.3 / 65536;

    /* 根据参考手册给的公式计算温度值 */
    TS_CAL1 = *(__IO uint16_t*)(0x1FF1E820);
    TS_CAL2 = *(__IO uint16_t*)(0x1FF1E840);

    AdcValues[3] = (110.0 - 30.0) * (ADCxValues[3] - TS_CAL1) / (TS_CAL2 - TS_CAL1) + 30;

    printf("PC0 = %5.3fV, Vbat/4 = %5.3fV, VrefInt = %5.3fV， TempSensor = %5.3f℃\n",
           AdcValues[0], AdcValues[1], AdcValues[2], AdcValues[3]);
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

    /* Enable D-Cache---------------------------------------------------------*/
    SCB_EnableDCache();

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_BDMA_Init();
    MX_USART1_UART_Init();
    MX_ADC3_Init();
    /* USER CODE BEGIN 2 */

    DelayNonInit();

    //  if (HAL_ADCEx_Calibration_Start(&hadc3, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED) != HAL_OK)
    {
        //   Error_Handler();
    }

    // HAL_ADC_Start(&hadc3);
    // HAL_ADC_Start_DMA(&hadc3, (uint32_t*)aAdVals, 4);
    // bsp_InitADC();
    ADC_Enable(&hadc3);
    HAL_ADC_Start_DMA(&hadc3, (uint32_t*)ADCxValues, 4);
    HAL_ADC_Start(&hadc3);
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        static uint32_t tAdSample = 0;

        if (DelayNonBlockMS(tAdSample, 1000))
        {
            tAdSample = DelayNonGetTick();

            bsp_GetAdcValues();

            //            float afAdVals[4];

            //            SCB_InvalidateDCache_by_Addr((uint32_t*)aAdVals, sizeof(aAdVals));

            // HAL_ADC_Start(&hadc3);
            // HAL_ADC_PollForConversion(&hadc3, 0xFFFFF);
            // aAdVals[0] = HAL_ADC_GetValue(&hadc3);

            // HAL_ADC_Start(&hadc3);
            // HAL_ADC_PollForConversion(&hadc3, 0xFFFFF);
            // aAdVals[1] = HAL_ADC_GetValue(&hadc3);

            // HAL_ADC_Start(&hadc3);
            // HAL_ADC_PollForConversion(&hadc3, 0xFFFFF);
            // aAdVals[2] = HAL_ADC_GetValue(&hadc3);

            // HAL_ADC_Start(&hadc3);
            // HAL_ADC_PollForConversion(&hadc3, 0xFFFFF);
            // aAdVals[3] = HAL_ADC_GetValue(&hadc3);

            //            afAdVals[0] = aAdVals[0] * 3.3 / 65536;
            //            afAdVals[1] = aAdVals[1] * 3.3 / 65536;
            //            afAdVals[2] = aAdVals[2] * 3.3 / 65536;
            //            afAdVals[3] = aAdVals[3] * 3.3 / 65536;

            //            printf("%f,%f,%f,%f\n",
            //                   afAdVals[0],
            //                   afAdVals[1],
            //                   afAdVals[2],
            //                   afAdVals[3]);

            // HAL_ADCEx_Calibration_Start();
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
    RCC_OscInitStruct.PLL.PLLQ       = 2;
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
    MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
    MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
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
