
#ifndef __BSP_PWM_H__
#define __BSP_PWM_H__

#include "main.h"

#define MOTOR_PWM_MIN  50
#define MOTOR_PWM_MAX  1000

#define PWM_GPIO_Port_1    GPIOC
#define PWM_Pin_1          GPIO_PIN_5
#define PWM_GPIO_Port_2    GPIOC
#define PWM_Pin_2          GPIO_PIN_1

#define MOTOR_IN1_ON()      HAL_GPIO_WritePin(PWM_GPIO_Port_1,PWM_Pin_1,GPIO_PIN_SET);
#define MOTOR_IN1_OFF()     HAL_GPIO_WritePin(PWM_GPIO_Port_1,PWM_Pin_1,GPIO_PIN_RESET);

#define MOTOR_IN2_ON()      HAL_GPIO_WritePin(PWM_GPIO_Port_2,PWM_Pin_2,GPIO_PIN_SET);
#define MOTOR_IN2_OFF()     HAL_GPIO_WritePin(PWM_GPIO_Port_2,PWM_Pin_2,GPIO_PIN_RESET);

#define MOTOR_MALFUCTION    0x05//超时 state状态

typedef enum
{
    MOTOR_STOP          = 0,//停止
    MOTOR_RUN           = 1,//启动
    MOTOR_BRAKE         = 2,//制动
    MOTOR_FAULT         = 3,//错误
    MOTOR_WAIT_REVERSE  = 4//等待时间
}MotorState_t;

typedef enum
{
    FAULT_NOERROR         = 0x00,  //无错误
    // 故障位定义
    FAULT_COMM_TIMEOUT    = 0x01,  // bit 0: 通信超时
    FAULT_STATE_ERROR     = 0x02,  // bit 1: 启动参数错误
    FAULT_DIR_ERROR       = 0x04,  // bit 2: 方向参数错误
    FAULT_PWM_ERROR       = 0x08,  // bit 3: 占空比参数错误
    // bit 4-7: 预留
    FAULT_OVERCURRENT     = 0x10,  // bit 4: 过流（预留，需要传感器）
    FAULT_OVERVOLTAGE     = 0x20,  // bit 5: 过压（预留，需要传感器）
    FAULT_OVERSPEED       = 0x40,  // bit 6: 超速（预留，需要编码器）
    FAULT_OVERHEAT        = 0x80,  // bit 7: 过热（预留，需要温度传感器）

}MotorFault_t;

typedef struct 
{
    uint8_t MotorState;//1=启动，0=未启动/停止 实际值
    uint8_t TargetState;//理想值
    uint8_t CurrentMotorDir;//0=正转，1=反转 实际值
    uint8_t TargetDir;
    uint16_t CurrentPWM;//是否需要更改占空比 0=不更改，1=更改
    uint16_t TargetPWM;
    uint32_t currentime;
    uint32_t Last_command_time;//时间戳
}Motor;


extern Motor MOTOR;
void Motor_Control(uint16_t reg_add);
int  Motor_Task(void);
void Motor_Run(void);//电机是否启动
void Motor_Stop(void);//电机是否停止
void Motor_Break(void);//电机是否制动
int  Motor_CheckTurnRequest(void);//判断电机转动方向
void Motor_DoTurn(void);//实际转动方向
void Motor_duty_cycle(void);
void PWM_SetDuty(uint16_t ccr);
void Motor_StopForReverse(void);
void Get_MotorState(void);//获取电机状态
void Motor_CheckTimeout(void);//检测是否超时
int  Motor_CheckState(void);
void Motor_ApplyState(void);
int  Motor_CheckDir(void);
void Motor_ApplyDir(void);
int  Motor_CheckPWM(void);
void Motor_ApplyPWM(void);
void Motor_Set_State(uint8_t state);
void Motor_Set_Fault(uint8_t fault);
int  Motor_Get_State(void);
int  Motor_Get_Fault(void);

#endif

