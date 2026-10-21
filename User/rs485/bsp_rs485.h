

#ifndef __BSP_RS485_H__
#define __BSP_RS485_H__

#include "main.h"
#include "stm32f1xx_hal.h"
#include "string.h"
#include "usart/bsp_usart.h"
extern UART_HandleTypeDef huart2;   

#define UART_BUFF_SIZE         1024
//#define RX_BUFFER_SIZE         256

//extern uint16_t Receive_Buffer[UART_BUFF_SIZE];
//extern uint8_t Receive_Buff[RX_BUFFER_SIZE];
extern volatile uint16_t Receive_len;
extern uint8_t uart_rx_byte;

extern uint8_t uart_Buffer[UART_BUFF_SIZE];


void RS485_Config(void);

void RS485_SendByte(uint8_t ch);//未使用的测试函数
int  RS485_SendByte_Length(uint8_t *str, uint32_t strlen);
void RS485_SendString(uint8_t *str);//未使用的测试函数

void clean_ReBuff(void);
void Receive_Buf();
char *get_rebuff(uint16_t *len);
void RS485_DEBUG_ARRAY(uint8_t *arr, uint16_t num);


#endif /* __BSP_RS485_H__ */