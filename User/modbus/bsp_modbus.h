#ifndef __BSP_MODBUS_H__
#define __BSP_MODBUS_H__
#include "main.h"
#include <stdio.h>

#define Base_address             0x01
#define REG_TOTAL_NUM            8
#define CAN_REG_TOTAL_NUM        8
#define REGIST_COUNT             125

/* 控制寄存器：主机可以读 */
#define REG_MOTOR_START    0x0000  // 电机启停：0=停止，1=启动，2=制动
#define REG_MOTOR_DIR      0x0001  // 电机方向：0=正转，1=反转
#define REG_MOTOR_PWM      0x0002  // PWM 占空比：例如 0~100 表示 0%~100%
#define REG_MOTOR_TARGET   0x0003  // 目标值：例如目标转速、目标位置，自定义 未实现 

/*
//状态寄存器 主要用于写
#define REG_MODBUS_START   0x0004
#define REG_MODBUS_DIR     0x0005
#define REG_MODBUS_PWM     0x0006
#define REG_MODBUS_TARGET  0x0007
*/
/* 状态寄存器：主要给主机读取 */
// TODO: 需要编码器硬件支持
#define REG_MOTOR_SPEED    0x0004  // 当前速度：例如 rpm
// TODO: 需要电流传感器支持
#define REG_MOTOR_CURRENT  0x0005  // 当前电流：例如 mA

#define REG_MOTOR_FAULT    0x0006  // 故障标志：不同 bit 表示不同故障

#define REG_MOTOR_STATUS   0x0007  // 当前状态：0=停止，1=运行，2=故障等

typedef struct
{
    //uint16_t regist;   // 寄存器地址
    uint16_t value;     // 寄存器当前值
    uint8_t  writable;  // 是否可写：1=可写，0=只读
    uint8_t  updated;   // 是否被主机写过：1=有更新待处理
} REG_Entry;

typedef struct
{
    uint8_t Handle_03_E;
    uint8_t Handle_06_W;//寄存器是否可写标志
    uint8_t Handle_10_W;//寄存器是否可写标志
    uint8_t USE_FRAMR; 
    uint8_t pending_response_type;//待处理响应类型

}REQ_Frame;

typedef enum 
{
    ADDR_IGNORE    = 0,   // 不是给我的，丢弃
    ADDR_MATCH     = 1,   // 是本机地址，正常处理
    ADDR_BROADCAST = 2    // 广播地址，执行不回复
} AddrResult;

typedef enum
{
    TEST_NULL           = 0, //没有使用帧
    TEST_USE            = 1, //帧使用中
    TEST_ADD_ERROR      = 2, //帧地址错误 在 10 中   帧地址错误优先级高于帧数据错误
    TEST_DATA_ERROR     = 3, //帧数据错误
    TEST_03             = 4, //03请求帧
    TEST_06             = 5, //现在是06请求帧
    TEST_10             = 6, //10请求帧
    TEST_ERROR_Frame    = 7  //错误帧
}REQUESTFRAME;

typedef struct 
{
    uint8_t rx_wait;//是否等待接收数据1=等待结束 0=在等待
    uint8_t rx_frame_ready;//是否收到完整数据帧 1=拿到完整 0= 未拿到
    uint8_t tx_pending;//是否有数据等待发送0= 有数据等待 1=没有等待
    uint8_t tx_busy;//是否正在发送 1= 发送完成处于空闲    0= 正在发送处于忙状态
}Rs485State_t;

typedef enum
{
    WAIT        = 0,//未完成，等待
    FINISH      = 1 //完成功能，进行下一步
}WaitData;

extern REG_Entry registers[REG_TOTAL_NUM];
extern REG_Entry can_reg[CAN_REG_TOTAL_NUM];
extern volatile REQ_Frame Frame;
extern volatile Rs485State_t RsState;
extern uint32_t Modbus_reg_data;//写入寄存器的值
extern uint8_t Modbus_Error_Data[5];
//extern volatile uint8_t Modbus_busy_frame_flag;


//CRC校验 逐位法
void registers_init(void);
void Check_Send_Modbus_Rs485(void);
uint16_t CRC_CheckP(uint8_t *data, uint16_t len);
uint16_t CRC_CheckList(uint8_t data);
uint16_t CRC_CheckL(uint8_t *data, uint16_t len);
AddrResult Add_filtration(uint8_t *data);

void handle_03(uint8_t *data, uint16_t len);
void REG03_Back(uint8_t *data, uint16_t len,uint16_t *addh,uint16_t *addl,uint8_t regist_count);
void Handle_06(uint8_t *data, uint16_t len);
void Handle_10(uint8_t* data,uint16_t len);
void REG10_Back(uint8_t* data,uint16_t len,uint16_t reg_num);
void Handle_ErrorFrame(uint8_t* data,uint16_t len);

#endif
