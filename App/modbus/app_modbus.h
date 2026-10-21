#ifndef __APP_MODBUS_H__
#define __APP_MODBUS_H__


#include "main.h"

#define RX_BUFFER_SIZE 256

extern volatile uint16_t Receive_Size;
extern volatile uint8_t Receive_flag;
extern uint8_t Receive_Buff[RX_BUFFER_SIZE];
extern volatile uint8_t Error_frame_flag;//0=没有错误 1= 发生错误

void Modbus_Init(void);
int modbustest(void);
void Check_Fuction(uint8_t *data ,uint16_t len);





#endif
