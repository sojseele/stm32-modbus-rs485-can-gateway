#ifndef __BSP_CAN_PROTOCOL_H__
#define __BSP_CAN_PROTOCOL_H__
#include "main.h"
#include <stdio.h>

#define GATEWAY_TO_MOTOR_CTRL    0x101  // 控制命令 网关->电机
#define MOTOR_TO_GATEWAY_STATUS  0x201  // 状态上报 电机->网关
//#define MOTOR_TO_GATEWAY_ACK     0x301  // 应答

//#define CAN_REG_TOTAL_NUM        8

#define CAN_CMD_MOTOR_CONTROL  0x01     // 控制
#define CAN_CMD_MOTOR_STATUS   0x02     //状态

//状态寄存器 主要用于写
#define REG_IDX_MOTOR_START   0x0000
#define REG_IDX_MOTOR_DIR     0x0001
#define REG_IDX_MOTOR_PWM     0x0002
#define REG_IDX_MOTOR_TARGET  0x0003

// TODO: 需要编码器硬件支持
#define REG_IDX_MOTOR_SPEED    0x0004  // 当前速度：例如 rpm
// TODO: 需要电流传感器支持
#define REG_IDX_MOTOR_CURRENT  0x0005  // 当前电流：例如 mA

#define REG_IDX_MOTOR_FAULT    0x0006  // 故障标志：不同 bit 表示不同故障

#define REG_IDX_MOTOR_STATUS   0x0007  // 当前状态：0=停止，1=运行，2=故障等

typedef struct
{
    uint8_t Can_Motor_pending;        // 电机版是否有待发送请求 0= 接收请求， 1=发送请求
    //uint8_t Can_Motor_State;
    uint8_t Can_Modbus_pending;       //网管版是否有待发送请求 0= 接收请求， 1=发送请求
    uint8_t Can_Modbus_State;         //网关版 所处的状态
    uint32_t can_id;        // CAN ID
    uint8_t data[8];        // 数据
    uint8_t dlc;            // 数据长度
} CanTxRequest_t;

typedef enum
{
    CAN_MODBUS_IDLE = 0,      // 空闲
    CAN_MODBUS_TX_PENDING,    // 网关已经准备好一组新数据，下一步要发送给电机。
                              //pc发来的数据进行配置寄存器，不是电机版发来的数据进行配置的
    CAN_MODBUS_WAIT_STATUS    // 网关已经把控制帧 0x101 发送给电机，现在等待电机返回状态帧 0x201。
} CanModbusState;

typedef enum
{
    Can_Request_rx  = 0,
    Can_Request_tx  = 1
}Can_Request;

typedef enum
{
    Can_Modbus_request_rx = 0,
    Can_Modbus_request_tx = 1
}Modbus_Request;


typedef enum
{
    Can_Leisure    = 0,
    Can_Busy       = 1
}Can_TxState;

typedef enum
{
    Can_Use_Motor  = 0,
    Can_Use_Modbus = 1

}Can_use_t;

//extern REG_Entry can_reg[CAN_REG_TOTAL_NUM];
extern volatile CanTxRequest_t CanState;
extern volatile uint8_t Can_use_flag;
extern volatile uint8_t Can_Tx_Busy_Flag;
extern volatile uint8_t Can_Rx_Busy_Flag;

void CanState_Init(void);
void Can_task(void);
void Can_ModbustoMotor_Send();
void Can_ModbustoMotor_Receive(void);
uint8_t Set_Motor(void);
void Can_MotortoModbus_Send(void);
void Can_MotortModbus_Receive(void);
int Check_XOR(uint8_t *can_data);
void Can_Modbus_Set_REG(void);
int Can_GetModbus_REG(uint16_t REG_ADD);
uint16_t Can_GetMotor_REG(uint16_t index);

#endif

