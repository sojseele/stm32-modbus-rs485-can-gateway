
#include "main.h"
#include "can/bsp_can.h"
#include "usart/bsp_usart.h"
#include "adc/bsp_adc.h"
#include "key/bsp_key.h"
#include "led/bsp_led.h"
#include "modbus/app_modbus.h"
#include "modbus/bsp_modbus.h"
#include "tim/bsp_tim.h"
#include "pwm/bsp_pwm.h"
#include "protocol/bsp_can_protocol.h"
#include "can/bsp_can.h"
#include "rs485/bsp_rs485.h"

void Error_Handler(void);

void SystemClock_Config(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
     
  
  HAL_Init();

  /* Configure the system clock to 72 MHz */
  SystemClock_Config();

  KEY_GPIO_Init();
  LED_GPIO_Config();
  //MX_ADC1_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_TIM1_Init();
  MX_CAN_Init();
  CAN_Filter_Config();

  if (HAL_CAN_Start(&hcan) != HAL_OK)
  {
      Error_Handler();
  }

  HAL_CAN_ActivateNotification(
      &hcan,
      CAN_IT_RX_FIFO0_MSG_PENDING |
      CAN_IT_TX_MAILBOX_EMPTY);
  
  //HAL_CAN_ActivateNotification(&hcan, CAN_IT_TX_MAILBOX_EMPTY);
  RS485_Config();
  Modbus_Init();
  Motor_Init();
  CanState_Init();
 

  /* Add your application code here */
  /* Infinite loop */
  while (1)
  {
    modbustest();
    Motor_Task();
    Check_Send_Modbus_Rs485();
    Can_task();
  }
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 72000000
  *            HCLK(Hz)                       = 72000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 2
  *            APB2 Prescaler                 = 1
  *            HSE Frequency(Hz)              = 8000000
  *            HSE PREDIV1                    = 1
  *            PLLMUL                         = 9
  *            Flash Latency(WS)              = 2
  * @param  None
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef clkinitstruct = {0};
  RCC_OscInitTypeDef oscinitstruct = {0};
  
  /* Enable HSE Oscillator and activate PLL with HSE as source */
  oscinitstruct.OscillatorType  = RCC_OSCILLATORTYPE_HSE;
  oscinitstruct.HSEState        = RCC_HSE_ON;
  oscinitstruct.HSEPredivValue  = RCC_HSE_PREDIV_DIV1;
  oscinitstruct.PLL.PLLState    = RCC_PLL_ON;
  oscinitstruct.PLL.PLLSource   = RCC_PLLSOURCE_HSE;
  oscinitstruct.PLL.PLLMUL      = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&oscinitstruct)!= HAL_OK)
  {
    /* Initialization Error */
    while(1);
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  clkinitstruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  clkinitstruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  clkinitstruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  clkinitstruct.APB2CLKDivider = RCC_HCLK_DIV1;
  clkinitstruct.APB1CLKDivider = RCC_HCLK_DIV2;  
  if (HAL_RCC_ClockConfig(&clkinitstruct, FLASH_LATENCY_2)!= HAL_OK)
  {
    /* Initialization Error */
    while(1);
  }
}
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



