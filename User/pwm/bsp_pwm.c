
#include "pwm/bsp_pwm.h"
#include "modbus/bsp_modbus.h"
#include "led/bsp_led.h"
#include "tim/bsp_tim.h"
#include "protocol/bsp_can_protocol.h"

Motor MOTOR;
volatile uint8_t Has_Set_flag = 0;//是否是第一次
volatile uint8_t Motor_stop_flag = 0;
volatile uint8_t Motor_dir_flag = 0;//是否转向
volatile uint8_t Motor_pwm_flag = 0;//是否配置pwm
volatile uint8_t Motor_tout_flag = 0;//是否超时
volatile uint8_t Has_set_state = 0;//上一次的状态

void Motor_Init()
{
    MOTOR.CurrentMotorDir = 0;
    MOTOR.TargetPWM = 300;
    MOTOR.Last_command_time = HAL_GetTick();
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
}

//假设 电机模块 连接在 gpioc 5 6 引脚
void Motor_Control(uint16_t reg_add)
{

    if(can_reg[reg_add].writable)
    {
        if(can_reg[reg_add].updated)
        {
            can_reg[reg_add].updated = 0;
            
            // 将寄存器值(0~1000)换算并写入PWM占空比
            //uint16_t pwm_value = reg[0x0002];  // 读取寄存器的值
            //uint16_t ccr = pwm_value * 1000 / 1000;  // ARR=1000，换算CCR
            //uint16_t ccr = registers[reg_add].value * 1000 / 1000; 
            //__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, ccr);
            /*
            switch(reg_add)
            {
                case REG_MOTOR_START:
                {
                    Motor_CheckState();
                    Motor_ApplyState();
                    break;
                }
                case REG_MOTOR_DIR:
                {
                    Motor_CheckDir();
                    Motor_ApplyDir();
                    break;
                }
                case REG_MOTOR_PWM:
                {
                    Motor_CheckPWM();
                    Motor_ApplyPWM();
                    break;
                }
                case REG_MOTOR_TARGET:
                    break;

            }
            */
            
            switch(reg_add)
            {
                case REG_IDX_MOTOR_START:
                {
                    Motor_CheckState();
                    Motor_ApplyState();
                    break;
                }
                case REG_IDX_MOTOR_DIR:
                {
                    Motor_CheckDir();
                    Motor_ApplyDir();
                    break;
                }
                case REG_IDX_MOTOR_PWM:
                {
                    Motor_CheckPWM();
                    Motor_ApplyPWM();
                    break;
                }
                case REG_IDX_MOTOR_TARGET:
                    break;

            }
            
            MOTOR.Last_command_time = HAL_GetTick();
            if(MOTOR.TargetState == MOTOR_RUN)
                Has_Set_flag = 1;
        }
        
    }
}

int Motor_Task()
{
    Has_set_state = MOTOR.TargetState;
    if(Has_Set_flag == 1 && MOTOR.TargetState == MOTOR_RUN)
        Motor_CheckTimeout();

    if(MOTOR.MotorState == MOTOR_WAIT_REVERSE)
    {
        if(HAL_GetTick() - MOTOR.currentime < 100)
        {
            return 1;
        }
        else
        {
             MOTOR.MotorState = Has_set_state;
             Motor_DoTurn();
             return 0;
        }
           
    }

    if(Motor_tout_flag == 1 && (Motor_dir_flag == 1 || Motor_pwm_flag == 1))
    {
        MOTOR.TargetState = MOTOR_RUN;
        Motor_tout_flag = 0;
        Motor_Set_Fault(FAULT_NOERROR);
    }

        

    /*
    if(MOTOR.MotorState != MOTOR.TargetState)
    {
        Motor_run();
    }
    */
    /*
    switch (MOTOR.TargetState)
    {
        case MOTOR_RUN:
            Motor_run();
            break;
        case MOTOR_STOP:
            //printf("电机未启动！\r\n");
            break;
    }

    switch (MOTOR.TargetDir)
    {
        case 0:
            Motor_CheckTurnRequest();
            break;
        case 1:
            //printf("未转向\r\n");
            break;
    }

    switch (MOTOR.TargetPWM)
    {
        case MOTOR_RUN:
            Motor_duty_cycle();
            break;
        case MOTOR_STOP:
            //printf("不更改占空比\r\n");
    }
    return 0;
    */

    switch (MOTOR.TargetState)
    {
        case MOTOR_STOP:
        {
            Motor_Stop();
            Motor_CheckTurnRequest();
            Motor_duty_cycle();
            
            break;
        }
        case MOTOR_RUN:
        {
            Motor_Run();
            Motor_CheckTurnRequest();
            Motor_duty_cycle();
            break;
        }
        case MOTOR_BRAKE:
        {
            Motor_Break();
            break;
        }
    }
    Get_MotorState();
    Motor_dir_flag = 0;
    
    Motor_pwm_flag = 0;
    return 0;
}

/*
void Motor_run()//电机是否启动
{
    if(MOTOR.MotorState != MOTOR.TargetState)
    {
        switch(MOTOR.TargetState)
        {
            case MOTOR_STOP:
            {
                
                MOTOR_IN1_OFF();
                MOTOR_IN2_OFF();
                R_LED_OFF();
                B_LED_OFF();

                printf("电机停止\r\n");
                MOTOR.MotorState = MOTOR_STOP;
                break;
            }
            case MOTOR_RUN:
            {
                Motor_duty_cycle();
                if(MOTOR.CurrentMotorDir == 0)
                {
                    MOTOR_IN1_ON();
                    MOTOR_IN2_OFF();
                    R_LED_ON_ONLY();
                    
                    printf("电机启动,现在是正转\r\n");
                    MOTOR.MotorState = MOTOR_RUN;
                    
                }
                else
                {
                    MOTOR_IN1_OFF();
                    MOTOR_IN2_ON();
                    B_LED_ON_ONLY();
                    printf("电机启动,现在是反转\r\n");
                    MOTOR.MotorState = MOTOR_RUN;
                }
                break;
            }
            case MOTOR_BRAKE:
            {
                MOTOR_IN1_OFF();
                MOTOR_IN2_OFF();
                MOTOR.MotorState = MOTOR_STOP;
                printf("电机制动\r\n");
                
            }

        }
    }

}
*/
void Motor_Stop()
{
    if(MOTOR.TargetState == MOTOR_STOP)
    {
        if(MOTOR.MotorState != MOTOR.TargetState)
        {
            MOTOR_IN1_OFF();
            MOTOR_IN2_OFF();
            R_LED_OFF();
            B_LED_OFF();

            printf("电机停止\r\n");
            MOTOR.MotorState = MOTOR_STOP;
            
        }
        if(Motor_stop_flag == 1)
        {
            Motor_tout_flag = 0;
            Motor_stop_flag = 0;
            printf("电机停止\r\n");
        }
        Motor_Set_State(MOTOR.MotorState);
    }
    
}

void Motor_Run()
{
    if(MOTOR.TargetState == MOTOR_RUN)
    {
        if(MOTOR.MotorState != MOTOR.TargetState)
        {
            if(MOTOR.CurrentMotorDir == 0)
            {
                MOTOR_IN1_ON();
                MOTOR_IN2_OFF();
                R_LED_ON_ONLY();
                printf("电机启动,现在是正转\r\n");
                MOTOR.MotorState = MOTOR_RUN;
                
            }
            else
            {
                MOTOR_IN1_OFF();
                MOTOR_IN2_ON();
                B_LED_ON_ONLY();
                printf("电机启动,现在是反转\r\n");
                MOTOR.MotorState = MOTOR_RUN;
            }
            Motor_Set_State(MOTOR.MotorState);
            printf("state的状态：%d\r\n",can_reg[REG_IDX_MOTOR_STATUS].value);
        }
    }
    
}

void Motor_Break()
{
    if(MOTOR.TargetState == MOTOR_BRAKE)
    {
        if(MOTOR.MotorState != MOTOR.TargetState)
        {
            MOTOR_IN1_OFF();
            MOTOR_IN2_OFF();
            MOTOR.MotorState = MOTOR_BRAKE;
            printf("电机制动\r\n");
            Motor_tout_flag = 0;
        }
        Motor_Set_State(MOTOR.MotorState);
        printf("state的状态：%d\r\n",can_reg[REG_IDX_MOTOR_STATUS].value);
    }
    
}

int Motor_CheckTurnRequest()//判断电机转动方向
{
    if(MOTOR.CurrentMotorDir != MOTOR.TargetDir)
    {
        if(MOTOR.MotorState != MOTOR_RUN)
        {
            MOTOR.CurrentMotorDir = MOTOR.TargetDir;
            printf("停止状态下更新方向:%d\r\n", MOTOR.TargetDir);
            return 0;
        }
        Motor_StopForReverse();
        if(MOTOR.MotorState == MOTOR_WAIT_REVERSE)
            return 1;
    }
    return 0;
}

void Motor_DoTurn()//实际转动方向
{
    if(MOTOR.TargetDir == 0)
    {
        //if(MOTOR.CurrentMotorDir == 1)

        MOTOR_IN1_ON();
        MOTOR_IN2_OFF();
        R_LED_ON_ONLY();
        printf("现在的转动方向是正转\r\n");
        MOTOR.CurrentMotorDir = 0;
    }
    else
    {
        printf("MOTOR.TargetState的值是：%d\r\n",MOTOR.TargetState);
        //if(MOTOR.CurrentMotorDir == 0)
        MOTOR_IN1_OFF();
        MOTOR_IN2_ON();
        B_LED_ON_ONLY();
        printf("现在的转动方向是反转\r\n");
        MOTOR.CurrentMotorDir = 1;
    }
}

void Motor_duty_cycle()//设置占空比
{
   
    uint16_t ccr ;
    if(MOTOR.CurrentPWM != MOTOR.TargetPWM)
    {
        ccr = MOTOR.TargetPWM;
        MOTOR.CurrentPWM = MOTOR.TargetPWM;
        printf("现在电机的占空比是:%d\r\n",ccr);
        PWM_SetDuty(ccr);
    }
    else
    {
        ccr = MOTOR.CurrentPWM;
    }
    
}

void PWM_SetDuty(uint16_t ccr)
{
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, ccr);
    MOTOR.CurrentPWM = ccr;
}

void Motor_StopForReverse()
{
    MOTOR_IN1_OFF();
    MOTOR_IN2_OFF();
    MOTOR.currentime = HAL_GetTick();
    MOTOR.MotorState = MOTOR_WAIT_REVERSE;

    //HAL_Delay(100);
}

void Get_MotorState()
{
    can_reg[REG_IDX_MOTOR_STATUS].value = MOTOR.MotorState;
}

void Motor_CheckTimeout()
{
    uint32_t elapsed = HAL_GetTick() - MOTOR.Last_command_time;
    if(elapsed > 10000)
    {
        Motor_Set_Fault(FAULT_COMM_TIMEOUT);
        MOTOR.TargetState = MOTOR_STOP;
        Motor_tout_flag = 1;
        printf("超时了！！！错误代码:%d\r\n",FAULT_COMM_TIMEOUT);
        Motor_Set_State(MOTOR_MALFUCTION);
        printf("故障代码：%d\r\n",can_reg[REG_IDX_MOTOR_STATUS].value);
    }
    else
    {
        Motor_Set_Fault(FAULT_NOERROR);
    }
    Motor_Set_State(MOTOR.MotorState);
}

int Motor_CheckState()
{
    uint32_t data;
    if(Frame.USE_FRAMR == TEST_06 || Frame.USE_FRAMR == TEST_10)
    {
         data = Modbus_reg_data;
    }
    else
    {
        data = can_reg[REG_IDX_MOTOR_START].value;
        //data = registers[REG_MOTOR_START].value;
    }
    if(data >2)
    {
        Motor_Set_Fault(FAULT_STATE_ERROR);
        printf("STATE数据非法！！！！,错误代码：%d,非法数据：%d\r\n",FAULT_STATE_ERROR,data);
        Motor_Set_State(MOTOR_FAULT);
        printf("故障代码：%d\r\n",can_reg[REG_IDX_MOTOR_STATUS].value);
        return 1;
    }
    else
    {
        
        Motor_Set_Fault(FAULT_NOERROR);
       
    }
    return 0; 
}

void Motor_ApplyState()
{
    if(Motor_Get_Fault() == FAULT_NOERROR)
    {
        //MOTOR.TargetState = registers[REG_MOTOR_START].value;
        MOTOR.TargetState = can_reg[REG_IDX_MOTOR_START].value;
        printf("MOTOR.TargetState的值是：%d\r\n",MOTOR.TargetState);
    }
    if(MOTOR.TargetState == MOTOR_STOP)//判断是否刹车
        Motor_stop_flag = 1;

    if(Motor_Get_State() == MOTOR_FAULT)
        Motor_Set_State(MOTOR.MotorState);
}

int Motor_CheckDir()
{
    uint32_t data;
    if(Frame.USE_FRAMR == TEST_06 || Frame.USE_FRAMR == TEST_10)
    {
         data = Modbus_reg_data;
    }
    else
    {
        data = can_reg[REG_IDX_MOTOR_DIR].value;
        //data = registers[REG_MOTOR_DIR].value;
    }
    if(data > 1)
    {
        Motor_Set_Fault(FAULT_DIR_ERROR);
        printf("DIR数据非法！！！！,错误代码：%d,非法数据：%d\r\n",FAULT_DIR_ERROR,data);
        Motor_Set_State(MOTOR_FAULT);
        printf("故障代码：%d\r\n",can_reg[REG_IDX_MOTOR_STATUS].value);
        return 1;
    }
    else
    {
        Motor_Set_Fault(FAULT_NOERROR);
    }
    return 0;
}
void Motor_ApplyDir()
{
    if(Motor_Get_Fault() == FAULT_NOERROR)
    {
        //MOTOR.TargetDir = registers[REG_MOTOR_DIR].value;
        MOTOR.TargetDir = can_reg[REG_IDX_MOTOR_DIR].value;
        Motor_dir_flag = 1;
        printf("MOTOR.TargetDir的值是：%d\r\n",MOTOR.TargetDir);
    }
    
    if(Motor_Get_State() == MOTOR_FAULT)
        Motor_Set_State(MOTOR.MotorState);
}

int Motor_CheckPWM()
{
    uint32_t data;
    if(Frame.USE_FRAMR == TEST_06 || Frame.USE_FRAMR == TEST_10)
    {
         data = Modbus_reg_data;
    }
    else
    {
        data = can_reg[REG_IDX_MOTOR_PWM].value;
        //data = registers[REG_MOTOR_PWM].value;
    }
    if(data < MOTOR_PWM_MIN || data > MOTOR_PWM_MAX)
    {
        Motor_Set_Fault(FAULT_PWM_ERROR);
        printf("PWM数据非法！！！！,错误代码：%d,非法数据：%d\r\n",FAULT_PWM_ERROR,data);
        Motor_Set_State(MOTOR_FAULT);
        printf("故障代码：%d\r\n",can_reg[REG_IDX_MOTOR_STATUS].value);
        return 1;
    }
    else
    {
        Motor_Set_Fault(FAULT_NOERROR);
    }
    return 0;
}
void Motor_ApplyPWM()
{
    if(Motor_Get_Fault() == FAULT_NOERROR)
    {
        //MOTOR.TargetPWM = registers[REG_MOTOR_PWM].value;
        MOTOR.TargetPWM = can_reg[REG_IDX_MOTOR_PWM].value;
        Motor_pwm_flag = 1;
        printf("MOTOR.TargetPWM的值是：%d\r\n",MOTOR.TargetPWM);
    }
    if(Motor_Get_State() == MOTOR_FAULT)
        Motor_Set_State(MOTOR.MotorState);
}


void Motor_Set_State(uint8_t state)
{
    can_reg[REG_IDX_MOTOR_STATUS].value = state;
}

void Motor_Set_Fault(uint8_t fault)
{
    can_reg[REG_IDX_MOTOR_FAULT].value = fault;
}
int Motor_Get_State()
{
    return can_reg[REG_IDX_MOTOR_STATUS].value;
}
int Motor_Get_Fault()
{
    return can_reg[REG_IDX_MOTOR_FAULT].value;
}

