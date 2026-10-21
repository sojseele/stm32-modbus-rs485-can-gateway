
#ifndef __BSP_USART_H__
#define __BSP_USART_H__

#include "main.h"

#include "stm32f1xx_hal.h"
#include <stdio.h>

extern UART_HandleTypeDef huart1;

extern UART_HandleTypeDef huart2;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_USART1_UART_Init(void);
void MX_USART2_UART_Init(void);
void MX_DMA_Init(void);
int fputc(int ch, FILE *f);


#endif /* __BSP_USART_H__ */

