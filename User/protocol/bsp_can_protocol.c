
#include "protocol/bsp_can_protocol.h"
#include "can/bsp_can.h"
#include "modbus/bsp_modbus.h"
#include "pwm/bsp_pwm.h"

volatile CanTxRequest_t CanState;
static volatile uint8_t cmd_sequence = 0;  // 全局序号
uint8_t Can_Data[8];
uint8_t data[8];
volatile uint16_t Can_Len;
volatile uint8_t Can_Tx_Busy_Flag;
volatile uint8_t Can_Rx_Busy_Flag;
CAN_TxHeaderTypeDef txHeader;
volatile CAN_RxHeaderTypeDef RxHeader;

//REG_Entry can_reg[CAN_REG_TOTAL_NUM];
volatile uint8_t Can_use_flag = Can_Use_Motor; // 0=电机版 1=网关版
uint32_t wait_start_tick;



void CanState_Init()
{
    CanState.Can_Motor_pending = Can_Request_rx;
    CanState.Can_Modbus_pending = Can_Modbus_request_tx;
    CanState.Can_Modbus_State = CAN_MODBUS_IDLE;
    Can_Tx_Busy_Flag = Can_Leisure;
    Can_Rx_Busy_Flag = Can_Leisure;

    //HAL_CAN_GetRxMessage(&hcan,CAN_FILTER_FIFO0,&RxHeader,Can_Data);
    
    //registers[REG_MOTOR_START].regist = 0;   // 默认停止
    can_reg[REG_IDX_MOTOR_START].value = 0;
    can_reg[REG_IDX_MOTOR_START].writable = 1;
    can_reg[REG_IDX_MOTOR_START].updated = 0;

    //registers[REG_MOTOR_DIR].regist = 0;     // 默认正转
    can_reg[REG_IDX_MOTOR_DIR].value = 0;
    can_reg[REG_IDX_MOTOR_DIR].writable = 1;
    can_reg[REG_IDX_MOTOR_DIR].updated = 0;

    //registers[REG_MOTOR_PWM].regist = 0;     // 默认占空比 0%
    can_reg[REG_IDX_MOTOR_PWM].value = 0;
    can_reg[REG_IDX_MOTOR_PWM].writable = 1;
    can_reg[REG_IDX_MOTOR_PWM].updated = 0;

    //registers[REG_MOTOR_TARGET].regist = 0;  // 默认目标值 0
    can_reg[REG_IDX_MOTOR_TARGET].value = 0;
    can_reg[REG_IDX_MOTOR_TARGET].writable = 0;
    can_reg[REG_IDX_MOTOR_TARGET].updated = 0;


     //registers[REG_MOTOR_SPEED].regist = 0;    // 当前速度
    can_reg[REG_IDX_MOTOR_SPEED].value = 0;
    can_reg[REG_IDX_MOTOR_SPEED].writable = 0;
    can_reg[REG_IDX_MOTOR_SPEED].updated = 0;

    //registers[REG_MOTOR_CURRENT].regist = 0;  // 当前电流
    can_reg[REG_IDX_MOTOR_CURRENT].value = 0;
    can_reg[REG_IDX_MOTOR_CURRENT].writable = 0;
    can_reg[REG_IDX_MOTOR_CURRENT].updated = 0;

    //registers[REG_MOTOR_FAULT].regist = 0;    // 故障标志
    can_reg[REG_IDX_MOTOR_FAULT].value = 0;
    can_reg[REG_IDX_MOTOR_FAULT].writable = 0;
    can_reg[REG_IDX_MOTOR_FAULT].updated = 0;
    
    //registers[REG_MOTOR_STATUS].regist = 0;   // 当前运行状态
    can_reg[REG_IDX_MOTOR_STATUS].value = 0;
    can_reg[REG_IDX_MOTOR_STATUS].writable = 0;
    can_reg[REG_IDX_MOTOR_STATUS].updated = 0;



}

void Can_task()
{
    if (CanState.Can_Modbus_State == CAN_MODBUS_WAIT_STATUS &&
        HAL_GetTick() - wait_start_tick > 800)
    {
        CanState.Can_Modbus_State = CAN_MODBUS_IDLE;
        CanState.Can_Modbus_pending = Can_Modbus_request_rx;
        Can_Tx_Busy_Flag = Can_Leisure;
        Can_Rx_Busy_Flag = Can_Leisure;
    }

    if (Can_Tx_Busy_Flag != Can_Leisure)
    {
        return;
    }
    Can_ModbustoMotor_Receive();
    Can_MotortoModbus_Send();
    Can_MotortModbus_Receive();
    if(HAL_GetTick() - wait_start_tick > 800 && CanState.Can_Modbus_State == CAN_MODBUS_WAIT_STATUS)
    {
        CanState.Can_Modbus_State = CAN_MODBUS_IDLE;
        CanState.Can_Modbus_pending = Can_Modbus_request_rx;
    }

}

//在modus发送数据
void Can_ModbustoMotor_Send()
{
    if (Can_Tx_Busy_Flag != Can_Leisure)
    {
        return;
    }
    if(Can_use_flag != Can_Use_Motor)
    {
        if(CanState.Can_Modbus_pending == Can_Modbus_request_tx)
        {
            if(CanState.Can_Modbus_State == CAN_MODBUS_TX_PENDING)
            {
                //Can_use_flag = Can_Use_Modbus;
                //CAN_TxHeaderTypeDef txHeader;
                uint8_t Can_ModbustxData[8];
                uint32_t txMailbox;
                
                Can_ModbustxData[0] = CAN_CMD_MOTOR_CONTROL;
                Can_ModbustxData[1] = Can_GetModbus_REG(REG_MOTOR_START);
                Can_ModbustxData[2] = Can_GetModbus_REG(REG_MOTOR_DIR);
                Can_ModbustxData[3] = Can_GetModbus_REG(REG_MOTOR_PWM) >> 8;
                Can_ModbustxData[4] = Can_GetModbus_REG(REG_MOTOR_PWM) & 0xFF;
                Can_ModbustxData[5] = cmd_sequence++;
                Can_ModbustxData[6] = 0x00;
                
                Can_ModbustxData[7] = Check_XOR(Can_ModbustxData);

                txHeader.StdId = GATEWAY_TO_MOTOR_CTRL;
                txHeader.ExtId = 0x00;
                txHeader.IDE = CAN_ID_STD;
                txHeader.RTR = CAN_RTR_DATA;
                txHeader.DLC = 8;
                txHeader.TransmitGlobalTime = DISABLE;
                wait_start_tick = HAL_GetTick();
                if(HAL_CAN_AddTxMessage(&hcan, &txHeader, Can_ModbustxData, &txMailbox) == HAL_OK)//是否发送成功
                {
                    printf("Modbus向Can总线数据发送成功！！！\r\n");
                    Can_Tx_Busy_Flag = Can_Busy;
                }
                else
                {
                    CanState.Can_Modbus_pending = Can_Modbus_request_rx;
                    CanState.Can_Modbus_State = CAN_MODBUS_IDLE;
                }
            }
            
        }
    }
    
    
}

void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan)//回调函数检测是否发送完成 检测邮箱0
{
    if(hcan->Instance == CAN1)
    {
        if(Can_use_flag == Can_Use_Modbus)
        {
            CanState.Can_Modbus_pending = Can_Modbus_request_rx;
            Can_Tx_Busy_Flag = Can_Leisure;
            Can_Rx_Busy_Flag = Can_Leisure;
            CanState.Can_Modbus_State = CAN_MODBUS_WAIT_STATUS;
            
        }
        else
        {
            Can_Tx_Busy_Flag = Can_Leisure;
            Can_Rx_Busy_Flag = Can_Leisure;
            CanState.Can_Motor_pending = Can_Request_rx;
            
        }
    }
}
void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan)
{
    if(hcan->Instance == CAN1)
    {
        if(Can_use_flag == Can_Use_Modbus)
        {
            CanState.Can_Modbus_pending = Can_Modbus_request_rx;
            Can_Tx_Busy_Flag = Can_Leisure;
            Can_Rx_Busy_Flag = Can_Leisure;
            CanState.Can_Modbus_State = CAN_MODBUS_WAIT_STATUS;
            
        }
        else
        {
            Can_Tx_Busy_Flag = Can_Leisure;
            Can_Rx_Busy_Flag = Can_Leisure;
            CanState.Can_Motor_pending = Can_Request_rx;
            
        }
    }
}

void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan)
{
    if(hcan->Instance == CAN1)
    {
        if(Can_use_flag == Can_Use_Modbus)
        {
            CanState.Can_Modbus_pending = Can_Modbus_request_rx;
            Can_Tx_Busy_Flag = Can_Leisure;
            Can_Rx_Busy_Flag = Can_Leisure;
            CanState.Can_Modbus_State = CAN_MODBUS_WAIT_STATUS;
            
        }
        else
        {
            Can_Tx_Busy_Flag = Can_Leisure;
            Can_Rx_Busy_Flag = Can_Leisure;
            CanState.Can_Motor_pending = Can_Request_rx;
            
        }
    }
}



//在motor接收数据
void Can_ModbustoMotor_Receive()
{
    
    if (Can_use_flag != Can_Use_Motor)
    {
        return;
    }

    if (CanState.Can_Motor_pending != Can_Request_rx)
    {
        return;
    }

    if (Can_Rx_Busy_Flag != Can_Busy)
    {
        return;
    }
/*
    if (HAL_CAN_GetRxMessage(&hcan,CAN_RX_FIFO0,&RxHeader,Can_Data) != HAL_OK)
    {
        Can_Rx_Busy_Flag = Can_Leisure;
        return;
    }
*/
    

    if (RxHeader.StdId == GATEWAY_TO_MOTOR_CTRL && Can_Len == 8 &&
        Can_Data[0] == CAN_CMD_MOTOR_CONTROL &&
        Check_XOR(Can_Data) == Can_Data[7] &&
        Set_Motor())
    {
        // 保持接收忙，必须等状态帧发送完成后才能再次接收
        CanState.Can_Motor_pending = Can_Request_tx;
    }
    else
    {
        // 错误帧不发送，直接恢复接收
        CanState.Can_Motor_pending = Can_Request_rx;
        Can_Rx_Busy_Flag = Can_Leisure;
    }
    
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    // 1. 检查是否是预期的CAN外设
    if (hcan->Instance == CAN1)
    {
        //Can_Len = RxHeader.DLC;
        if(HAL_CAN_GetRxMessage(hcan,CAN_RX_FIFO0,&RxHeader,Can_Data) == HAL_OK)
        {
            if (Can_use_flag == Can_Use_Motor &&
                CanState.Can_Motor_pending == Can_Request_rx &&
                RxHeader.StdId == GATEWAY_TO_MOTOR_CTRL)
            {
                Can_Len = RxHeader.DLC;
                Can_Rx_Busy_Flag = Can_Busy;
            }
            else if (Can_use_flag == Can_Use_Modbus &&
                    CanState.Can_Modbus_State == CAN_MODBUS_WAIT_STATUS &&
                    RxHeader.StdId == MOTOR_TO_GATEWAY_STATUS)
            {
                Can_Len = RxHeader.DLC;
                Can_Rx_Busy_Flag = Can_Busy;
            }
            else
            {
                Can_Rx_Busy_Flag = Can_Leisure;
            }
        }
        
        else
        {
            Can_Rx_Busy_Flag = Can_Leisure;
        }
    }
}

uint8_t Set_Motor()
{
    if(Can_Tx_Busy_Flag == Can_Leisure)
    {
        uint16_t pwm;

        if (Can_Len != 8)
        {
            return 0;
        }

        if (RxHeader.StdId != GATEWAY_TO_MOTOR_CTRL)
        {
            return 0;
        }

        if (Can_Data[0] != CAN_CMD_MOTOR_CONTROL)
        {
            return 0;
        }

        if (Check_XOR(Can_Data) != Can_Data[7])
        {
            return 0;
        }


        can_reg[REG_IDX_MOTOR_START].value = Can_Data[1];
        can_reg[REG_IDX_MOTOR_START].updated = 1;
        Motor_Control(REG_IDX_MOTOR_START);

        can_reg[REG_IDX_MOTOR_DIR].value = Can_Data[2];
        can_reg[REG_IDX_MOTOR_DIR].updated = 1;
        Motor_Control(REG_IDX_MOTOR_DIR);

        pwm = ((uint16_t)Can_Data[3] << 8) | Can_Data[4];

        can_reg[REG_IDX_MOTOR_PWM].value = pwm;
        can_reg[REG_IDX_MOTOR_PWM].updated = 1;

        Motor_Control(REG_IDX_MOTOR_PWM);
        
        return 1;
    }
    return 0;
}
//再motor发送数据
void Can_MotortoModbus_Send()
{
    if(Can_use_flag == Can_Use_Motor)
    {
        if(CanState.Can_Motor_pending == Can_Request_tx)
        {
            //Can_use_flag = Can_Use_Motor;
            //CAN_TxHeaderTypeDef txHeader;
            uint8_t Can_MotortxData[8];
            uint16_t Can_tx_len;
            //Can_tx_len = len;
            uint32_t txMailbox;
            
            Can_MotortxData[0] = CAN_CMD_MOTOR_STATUS;
            Can_MotortxData[1] = Can_GetMotor_REG(REG_IDX_MOTOR_START);
            Can_MotortxData[2] = Can_GetMotor_REG(REG_IDX_MOTOR_DIR);
            Can_MotortxData[3] = Can_GetMotor_REG(REG_IDX_MOTOR_PWM) >> 8;
            Can_MotortxData[4] = Can_GetMotor_REG(REG_IDX_MOTOR_PWM) & 0xFF;
            Can_MotortxData[5] = Can_Data[5];
            Can_MotortxData[6] = 0x00;
            
            Can_MotortxData[7] = Check_XOR(Can_MotortxData);

            txHeader.StdId = MOTOR_TO_GATEWAY_STATUS;
            txHeader.ExtId = 0x00;
            txHeader.IDE = CAN_ID_STD;
            txHeader.RTR = CAN_RTR_DATA;
            txHeader.DLC = 8;
            txHeader.TransmitGlobalTime = DISABLE;
            
            if(HAL_CAN_AddTxMessage(&hcan, &txHeader, Can_MotortxData, &txMailbox) == HAL_OK)//是否发送成功
            {
                printf("Can总线向modbus数据发送成功！！！\r\n");
                //Can_Tx_Busy_Flag = Can_Busy;
            }
        }
    }
    
}
//再modbus接收数据
void Can_MotortModbus_Receive()
{
    if(Can_use_flag != Can_Use_Motor)
    {
        if(CanState.Can_Modbus_pending == Can_Modbus_request_rx)
        {
            if(CanState.Can_Modbus_State == CAN_MODBUS_WAIT_STATUS)
            {
                if(Can_Rx_Busy_Flag == Can_Busy)
                {
                    /*
                    if(HAL_CAN_GetRxMessage(&hcan,CAN_FILTER_FIFO0,&RxHeader,Can_Data) == HAL_OK)
                    {
                        
                        Can_Len = RxHeader.DLC;
                        Can_Modbus_Set_REG();
                    }
                        */
                    Can_Modbus_Set_REG();
                }
            }
        }
    }
    
}


//XOR校验
int Check_XOR(uint8_t *can_data)
{
    uint8_t checksum = 0;
    for (int i = 0; i < 7; i++)
    {
        checksum ^= can_data[i];
    }
    return checksum;
}

void Can_Modbus_Set_REG()
{
    
    if(CanState.Can_Modbus_State == CAN_MODBUS_WAIT_STATUS)
    {
        if(RxHeader.StdId == MOTOR_TO_GATEWAY_STATUS && Can_Data[0] == CAN_CMD_MOTOR_STATUS)
        {
            if (Can_Len != 8)
            {
                CanState.Can_Modbus_pending = Can_Modbus_request_rx;
                Can_Rx_Busy_Flag = Can_Leisure;
                return;
            }


            if (Check_XOR(Can_Data) != Can_Data[7])
            {
                CanState.Can_Modbus_pending = Can_Modbus_request_rx;
                Can_Rx_Busy_Flag = Can_Leisure;
                return;
            }
            registers[REG_MOTOR_START].value = Can_Data[1];
            registers[REG_MOTOR_DIR].value = Can_Data[2];
            registers[REG_MOTOR_PWM].value = (Can_Data[3] << 8) | Can_Data[4];
            CanState.Can_Modbus_State = CAN_MODBUS_IDLE;
            Can_Rx_Busy_Flag = Can_Leisure;
            CanState.Can_Modbus_pending = Can_Modbus_request_rx;
        }
        else
        {
            CanState.Can_Modbus_pending = Can_Modbus_request_rx;
            Can_Rx_Busy_Flag = Can_Leisure;
        }
        
    }
}

//网关版
int Can_GetModbus_REG(uint16_t REG_ADD)
{
    return registers[REG_ADD].value;
}
//电机版
uint16_t Can_GetMotor_REG(uint16_t index)
{
    if (index >= CAN_REG_TOTAL_NUM)
    {
        return 0;
    }
    return can_reg[index].value;
}



