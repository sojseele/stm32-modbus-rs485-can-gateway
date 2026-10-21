#ifndef __CAN_H__
#define __CAN_H__


/* Includes ------------------------------------------------------------------*/
#include  "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern CAN_HandleTypeDef hcan;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_CAN_Init(void);
void CAN_Filter_Config(void);



#endif /* __CAN_H__ */

