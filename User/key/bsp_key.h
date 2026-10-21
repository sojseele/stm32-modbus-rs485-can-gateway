
#ifndef __BSP_KEY_H__
#define __BSP_KEY_H__

#include "main.h"
#include "stm32f1xx_hal.h"

typedef struct {
    volatile uint16_t rpm;
    volatile uint8_t right_turn;
    volatile uint8_t new_data_flag;
} ECU_Dashboard_turn;

typedef struct {
    volatile uint8_t break_car;
    volatile uint8_t new_data_flag;
} ECU_Dashboard_break;

extern volatile ECU_Dashboard_turn Ecu_Dashboard_TData;
extern volatile ECU_Dashboard_break Ecu_Dashboard_BData;

#define KEY3_Pin GPIO_PIN_13
#define KEY3_GPIO_Port GPIOC
#define KEY2_Pin GPIO_PIN_0
#define KEY2_GPIO_Port GPIOA

#define KEY_ON()                1
#define KEY_OFF()               0

void KEY_GPIO_Init(void);
uint8_t KeyScan(GPIO_TypeDef* GPIOx,uint16_t GPIO_PIN);




#endif
