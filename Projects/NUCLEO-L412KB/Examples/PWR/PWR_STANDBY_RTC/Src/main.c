/**
  ******************************************************************************
  * @file    PWR/PWR_STANDBY_RTC/Src/main.c
  * @author  MCD Application Team
  * @brief   This sample code shows how to use STM32L4xx PWR HAL API to enter
  *          and exit the Standby mode using RTC.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/** @addtogroup STM32L4xx_HAL_Examples
  * @{
  */

/** @addtogroup PWR_STANDBY_RTC
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define LED_TOGGLE_DELAY         100

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* RTC handler declaration */
RTC_HandleTypeDef RTCHandle;
static __IO uint32_t TimingDelay;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void SystemPower_Config(void);
void RTC_Config(void);
void Error_Handler(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  uint32_t tickstart;

  /* STM32L4xx HAL library initialization:
       - Configure the Flash prefetch
       - Systick timer is configured by default as source of time base, but user
         can eventually implement his proper time base source (a general purpose
         timer for example or other time source), keeping in mind that Time base
         duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and
         handled in milliseconds basis.
       - Set NVIC Group Priority to 4
       - Low Level Initialization
     */
  HAL_Init();

  /* Configure the system clock to 80 MHz */
  SystemClock_Config();

  /* Configure LED3 */
  BSP_LED_Init(LED3);

  /* Configure RTC */
  RTC_Config();

  /* Configure the system Power */
  SystemPower_Config();

  /* Check if the system was resumed from StandBy mode */
  if(__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET)
  {
    /* Clear Standby flag */
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
  }

  /* Insert 5 seconds delay */
  HAL_Delay(5000);

  /* Enable ultra low power BOR and PVD supply monitoring */
  HAL_PWREx_EnableBORPVD_ULP();

  /* The Following Wakeup sequence is highly recommended prior to each Standby mode entry
     mainly when using more than one wakeup source this is to not miss any wakeup event.
     - Disable all used wakeup sources,
     - Clear all related wakeup flags,
     - Re-enable all used wakeup sources,
     - Enter the Standby mode.
  */
  /* Disable all used wakeup sources */
  HAL_RTCEx_DeactivateWakeUpTimer(&RTCHandle);

  /* Clear all related wakeup flags */
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

  /* Re-enable wakeup source */
  /* ## Setting the Wake up time ############################################*/
  /* RTC Wakeup Interrupt Generation:
    the wake-up counter is set to its maximum value to yield the longuest
    stand-by time to let the current reach its lowest operating point.
    The maximum value is 0xFFFF, corresponding to about 33 sec. when
    RTC_WAKEUPCLOCK_RTCCLK_DIV = RTCCLK_Div16 = 16 */

  /* Wakeup Time Base = (RTC_WAKEUPCLOCK_RTCCLK_DIV /(LSE))
     Wakeup Time = Wakeup Time Base * WakeUpCounter
       = (RTC_WAKEUPCLOCK_RTCCLK_DIV /(LSE)) * WakeUpCounter
       ==> WakeUpCounter = Wakeup Time / Wakeup Time Base   */

  /* To configure the wake up timer to 33s the WakeUpCounter is set to 0xFFFF:
     Wakeup Time Base = 16 /(~32 kHz RC) = ~0.5 ms
     Wakeup Time = 0.5 ms  * WakeUpCounter
     Therefore, with wake-up counter =  0xFFFF  = 65,535
       Wakeup Time =  0.5 ms *  65,535 = ~ 33 sec. */
  HAL_RTCEx_SetWakeUpTimer_IT(&RTCHandle, 0xFFFF, RTC_WAKEUPCLOCK_RTCCLK_DIV16, 0);

  /* Need to wait for 2 RTC clock cycles before entering Standby mode when RTC is clocked by LSE,
     so wait for RSF flag to ensure the delay is satisfied */
  tickstart = HAL_GetTick();
  while((RTCHandle.Instance->ICSR & RTC_ICSR_RSF) == 0U)
  {
    if((HAL_GetTick() - tickstart ) > RTC_TIMEOUT_VALUE)
    {
      Error_Handler() ;
    }
  }


  /* Enter the Standby mode */
  HAL_PWR_EnterSTANDBYMode();

  /* Program should never reach this point (program restart when exiting from standby mode) */
  Error_Handler();

  while (1)
  {
  }
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follows :
  *            System Clock source            = PLL (MSI)
  *            SYSCLK(Hz)                     = 80000000
  *            HCLK(Hz)                       = 80000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            MSI Frequency(Hz)              = 4000000
  *            PLL_M                          = 1
  *            PLL_N                          = 40
  *            PLL_R                          = 2
  *            PLL_Q                          = 4
  *            Flash Latency(WS)              = 4
  * @param  None
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};

  /* MSI is enabled after System reset, activate PLL with MSI as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    /* Initialization Error */
    while(1);
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    /* Initialization Error */
    while(1);
  }
}

/**
  * @brief  System Power Configuration
  *         The system Power is configured as follow:
  *            + Automatic Wakeup using RTC clocked by LSI (after ~4s)
  * @param  None
  * @retval None
  */
void SystemPower_Config(void)
{
  /* Enable Power Clock */
  __HAL_RCC_PWR_CLK_ENABLE();

}

/**
  * @brief  RTC Configuration
  *         RTC Clocked by LSE (see HAL_RTC_MspInit)
  * @param  None
  * @retval None
  */
void RTC_Config(void)
{
  /* Configure RTC */
  RTCHandle.Instance = RTC;
  /* Set the RTC time base to 1s */
  /* Configure RTC prescaler and RTC data registers as follow:
    - Hour Format = Format 24
    - Asynch Prediv = Value according to source clock
    - Synch Prediv = Value according to source clock
    - OutPut = Output Disable
    - OutPutPolarity = High Polarity
    - OutPutType = Open Drain */
  RTCHandle.Init.HourFormat = RTC_HOURFORMAT_24;
  RTCHandle.Init.AsynchPrediv = RTC_ASYNCH_PREDIV;
  RTCHandle.Init.SynchPrediv = RTC_SYNCH_PREDIV;
  RTCHandle.Init.OutPut = RTC_OUTPUT_DISABLE;
  RTCHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  RTCHandle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if(HAL_RTC_Init(&RTCHandle) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /* Set RTC calibration low power mode */
  HAL_RTCEx_SetLowPowerCalib(&RTCHandle, RTC_LPCAL_SET);
}

/**
  * @brief SYSTICK callback
  * @param None
  * @retval None
  */
void HAL_SYSTICK_Callback(void)
{
  HAL_IncTick();

  if (TimingDelay != 0)
  {
    TimingDelay--;
  }
  else
  {
    /* Toggle LED3 */
    BSP_LED_Toggle(LED3);
    TimingDelay = LED_TOGGLE_DELAY;
  }
}


/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
  while(1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */

/**
  * @}
  */

