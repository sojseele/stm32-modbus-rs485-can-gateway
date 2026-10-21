
#ifndef __BSP_TIM_H
#define __BSP_TIM_H



#include "main.h"



extern TIM_HandleTypeDef htim1;

extern TIM_HandleTypeDef htim2;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_TIM1_Init(void);
void MX_TIM2_Init(void);

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);





#endif /*__BSP_LED_H */
