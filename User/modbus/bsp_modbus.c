
#include "modbus/bsp_modbus.h"
#include "usart/bsp_usart.h"
#include "pwm/bsp_pwm.h"
#include "modbus/app_modbus.h"
#include "protocol/bsp_can_protocol.h"
#include "can/bsp_can.h"
#include "pwm/bsp_pwm.h"

//volatile uint8_t Modbus_busy_frame_flag = 0;//接收数据 是否空闲0=空闲，1=忙碌

uint8_t Modbus_Tx_data[REGIST_COUNT * 2 + 5];
uint8_t Modbus_Tx_data_03[REGIST_COUNT * 2 + 5];
uint8_t Modbus_Tx_data_06[8];
uint8_t Modbus_Tx_data_10[8];
REG_Entry registers[REG_TOTAL_NUM];
REG_Entry can_reg[CAN_REG_TOTAL_NUM];
volatile REQ_Frame Frame;
volatile Rs485State_t RsState;
volatile uint8_t Check_Frame_flag = 0;//检查帧数据是否错误
uint8_t Modbus_arr;//回复帧长度
uint32_t Modbus_reg_data;//写入寄存器的值
uint8_t Modbus_Error_Data[5];

//uint16_t registers[8];

// 转速寄存器更新接口（预留，待编码器硬件支持）
void Modbus_SetMotorSpeed(uint16_t speed)
{
    registers[REG_MOTOR_SPEED].value = speed;
    //registers[REG_MOTOR_SPEED].regist = speed;
}

void registers_init(void)
{
    
    Frame.Handle_03_E = 0;
    Frame.Handle_06_W = 0;
    Frame.Handle_10_W = 0;
    Frame.USE_FRAMR = 0;
    Frame.pending_response_type = 0;

    RsState.rx_wait = 0;
    RsState.rx_frame_ready = 0;
    RsState.tx_pending = 1;
    RsState.tx_busy = 1;

    CanState.Can_Modbus_State = CAN_MODBUS_IDLE;

    /*
     * 控制寄存器
     * 这些值通常由主机通过 0x06 或 0x10 写入，
     * 从机根据这些值去控制真实硬件。
     */
    //registers[REG_MOTOR_START].regist = 0;   // 默认停止
    registers[REG_MOTOR_START].value = 0;
    registers[REG_MOTOR_START].writable = 1;
    registers[REG_MOTOR_START].updated = 0;

    //registers[REG_MOTOR_DIR].regist = 0;     // 默认正转
    registers[REG_MOTOR_DIR].value = 0;
    registers[REG_MOTOR_DIR].writable = 1;
    registers[REG_MOTOR_DIR].updated = 0;

    //registers[REG_MOTOR_PWM].regist = 0;     // 默认占空比 0%
    registers[REG_MOTOR_PWM].value = 0;
    registers[REG_MOTOR_PWM].writable = 1;
    registers[REG_MOTOR_PWM].updated = 0;

    //registers[REG_MOTOR_TARGET].regist = 0;  // 默认目标值 0
    registers[REG_MOTOR_TARGET].value = 0;
    registers[REG_MOTOR_TARGET].writable = 0;
    registers[REG_MOTOR_TARGET].updated = 0;

    /*
     * 状态寄存器
     * 这些值通常由从机自己更新，
     * 主机通过 0x03 读取。
     */
    //registers[REG_MOTOR_SPEED].regist = 0;    // 当前速度
    registers[REG_MOTOR_SPEED].value = 0;
    registers[REG_MOTOR_SPEED].writable = 0;
    registers[REG_MOTOR_SPEED].updated = 0;

    //registers[REG_MOTOR_CURRENT].regist = 0;  // 当前电流
    registers[REG_MOTOR_CURRENT].value = 0;
    registers[REG_MOTOR_CURRENT].writable = 0;
    registers[REG_MOTOR_CURRENT].updated = 0;

    //registers[REG_MOTOR_FAULT].regist = 0;    // 故障标志
    registers[REG_MOTOR_FAULT].value = 0;
    registers[REG_MOTOR_FAULT].writable = 0;
    registers[REG_MOTOR_FAULT].updated = 0;
    
    //registers[REG_MOTOR_STATUS].regist = 0;   // 当前运行状态
    registers[REG_MOTOR_STATUS].value = 0;
    registers[REG_MOTOR_STATUS].writable = 0;
    registers[REG_MOTOR_STATUS].updated = 0;
}

void Check_Send_Modbus_Rs485()
{
    if(RsState.rx_frame_ready == FINISH && RsState.tx_pending == WAIT)
    {
        switch (Frame.pending_response_type)
        {
            case TEST_03:
            {
                if(RS485_SendByte_Length(Modbus_Tx_data_03,Modbus_arr) == HAL_OK)
                    Frame.pending_response_type = TEST_NULL;
                else
                    printf("ERROR: RS485 transmit failed!!!!Error Frame:03\r\n");
                break;
            }
            case TEST_06:
            {
                if(RS485_SendByte_Length(Modbus_Tx_data_06,Modbus_arr) == HAL_OK)
                    Frame.pending_response_type = TEST_NULL;
                else
                    printf("ERROR: RS485 transmit failed!!!!Error Frame:06\r\n");
                break;
            }
            case TEST_10:
            {
                if(RS485_SendByte_Length(Modbus_Tx_data_10,Modbus_arr) == HAL_OK)
                    Frame.pending_response_type = TEST_NULL;
                else
                    printf("ERROR: RS485 transmit failed!!!!Error Frame:10\r\n");
                break;
            }
            case TEST_ERROR_Frame:
            {
                if(RS485_SendByte_Length(Modbus_Tx_data,Modbus_arr) == HAL_OK)
                    Frame.pending_response_type = TEST_NULL;
                else
                    printf("ERROR: RS485 transmit failed!!!!Error Frame:ERROR_Frame\r\n");
                break;
            }
        }
    }
    
     
}

//检验寄存器是否可写
void registers_check(uint16_t reg_add)
{
    if(registers[reg_add].writable)
    {
        if(Frame.USE_FRAMR == TEST_06)
        {
            Frame.Handle_06_W = TEST_USE;
            printf("数据合法！！！\r\n");
        }
            
        if(Frame.USE_FRAMR == TEST_10)
            Frame.Handle_10_W = TEST_USE;
    }
    else
    {
        if(Frame.USE_FRAMR == TEST_06)
            Frame.Handle_06_W = TEST_NULL;
        if(Frame.USE_FRAMR == TEST_10)
            Frame.Handle_10_W = TEST_NULL;
        printf("数据非法！！！非法数据地址：%d\r\n",reg_add);
    }

}


//CRC校验 逐位法
uint16_t CRC_CheckP(uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xffff;
    uint16_t temp;

    for(; len ; len--)
    {    
        temp = crc ^ *data++;
        for(int i = 0 ;i<8;i++)
        {
            if((temp & 0x0001) == 1)
            {
                temp = temp >> 1;
                temp ^= 0xA001;
            }
            else
            {
                temp >>= 1;
            }

        }
        crc = temp;
    }

    return crc;
}
//图表法
uint16_t CRC_CheckList(uint8_t data)
{
    static uint16_t arr[256] = {0};
    static uint8_t flag = 0;
    if(flag == 0)
    {
        uint16_t temp = 0;
        for(int i = 0;i<256;i++)
        {
            temp = i;
                for(int h = 0 ;h<8;h++)
                {
                    if((temp & 0x0001) == 1)
                    {
                        temp = temp >> 1;
                        temp ^= 0xA001;
                    }
                    else
                    {
                        temp >>= 1;
                    }
                }
                arr[i] = temp;
        }
        flag = 1;
    }
    return arr[data];

}
/*
//全局
uint16_t crc_buf[256];
uint16_t CRC_CheckList(uint8_t data)
{
    uint16_t temp = 0;
        for(int i = 0;i<256;i++)
        {
            temp = i;
                for(int h = 0 ;h<8;h++)
                {
                    if((temp & 0x0001) == 1)
                    {
                        temp = temp >> 1;
                        temp ^= 0xA001;
                    }
                    else
                    {
                        temp >>= 1;
                    }
                }
                crc_buf[i] = temp;
        }
    return crc_buf[data];
}
*/


uint16_t CRC_CheckL(uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xffff;
    uint16_t templ;
    uint16_t temph;
    
    for(;len;len--)
    {
        crc ^= *data++;
        templ = crc & 0xFF;
        temph = crc >> 8 ;
        crc = temph ^ CRC_CheckList(templ);  // 静态方式
        //crc = temph ^ crc_buf[templ];        // 全局方式
    }
    return crc;
}

//地址过滤
AddrResult Add_filtration(uint8_t *data)
{
    if(*data == Base_address)
    {
        return ADDR_MATCH;
    }
    else if(*data == 0x00)
    {
        return ADDR_BROADCAST;
    }
    else
    {
        return ADDR_IGNORE;
    }
}



//功能码0x03
void handle_03(uint8_t *data, uint16_t len)
{
    Frame.pending_response_type = TEST_03;
    uint16_t add;//初始地址
    uint16_t regist_count;//数量
    
    uint16_t reg_backh[REGIST_COUNT];
    uint16_t reg_backl[REGIST_COUNT];

    add = (data[2] << 8) | data[3]; //起始地址
    regist_count = (data[4] << 8) | data[5];//数量

    

    if(add < REG_TOTAL_NUM && regist_count <= REG_TOTAL_NUM - add )
    {
        if(regist_count > 0 )
        {
            Frame.Handle_03_E = 0;
            for(int i = 0; i < regist_count ; i++)
            {
                reg_backh[i] = registers[add].value >> 8;    // 寄存器值的高字节
                reg_backl[i] = registers[add].value & 0xFF;
                add++;
            }
            REG03_Back(data, len, reg_backh, reg_backl, regist_count);
        }
        else
        {
            printf("DATA ERROR!!!,异常码：0x03\r\n");
            Frame.Handle_03_E = 1;
            reg_backh[0] = 0x03; 
            REG03_Back(data, len, reg_backh, reg_backl, regist_count);
        }
    }
    else
    {
        printf("Address ERROR!!!,异常码：0x02\r\n");
        Frame.Handle_03_E = 1;
        reg_backh[0] = 0x02; 
        REG03_Back(data, len, reg_backh, reg_backl, regist_count);
        
    }

}
//回显
void REG03_Back(uint8_t *data, uint16_t len,uint16_t *value_h,uint16_t *value_l,uint8_t regist_count)
{
    
    Modbus_arr = (Frame.Handle_03_E == 0) ? (regist_count * 2 + 5) : 5;
    uint8_t frame_back[Modbus_arr];
    int i = 3;

    frame_back[0] = data[0];//起始地址
    frame_back[1] = data[1];
    if(Frame.Handle_03_E == 0)
    {
        frame_back[2] = regist_count * 2;
        for(int j = 0; j < regist_count ; j++)
        {
            frame_back[i++] = value_h[j];
            frame_back[i++] = value_l[j];
            
        }
        uint16_t crc = CRC_CheckL(frame_back,Modbus_arr - 2);
        frame_back[i++] = crc & 0xFF;    // 低字节在前
        frame_back[i++] = crc >> 8;      // 高字节在后
    }
    else
    {
        frame_back[1] = data[1] | 0x80;
        frame_back[2] = value_h[0];
        uint16_t crc = CRC_CheckL(frame_back, 3);
        frame_back[3] = crc & 0xFF;   // CRC低字节
        frame_back[4] = crc >> 8;     // CRC高字节
    }
    
    printf("0x03功能码 10进制回复帧为:");
    for(int j = 0; j < Modbus_arr; j++)
    printf("%d ",frame_back[j]);
    printf("\r\n");
    printf("0x03功能码 16进制回复帧为:");
    for(int j = 0; j < Modbus_arr; j++)
    printf("%02X ",frame_back[j]);
    printf("\r\n");
    
    RsState.tx_pending = WAIT;
    //HAL_UART_Transmit(&huart1,frame_back,arr, HAL_MAX_DELAY);
    memcpy(Modbus_Tx_data_03,frame_back,Modbus_arr);
    
    
    /*modbus测试帧
    if(Modbus_busy_frame_flag == 0)
    {
        if(HAL_UART_Transmit_IT(&huart1, Tx_data, arr) == HAL_OK)
        {
            Modbus_busy_frame_flag = 1;
        }
    }
    */

}

/*
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        // 在这里写上"发送完毕"后要做的事情
        // 比如：点亮LED、或者重新启动DMA接收
        Modbus_busy_frame_flag = 0; 
    }
}
*/

//功能码0x06
void Handle_06(uint8_t *data, uint16_t len)
{
    Frame.pending_response_type = TEST_06;
    uint8_t error_data;
    uint16_t reg_add;//寄存器地址
    //uint16_t reg_data;//写入寄存器的值
    uint8_t reg_count;//回复帧的长度
    Check_Frame_flag = HAL_OK;

    reg_add  = (data[2] << 8) | data[3];
    Modbus_reg_data = (data[4] << 8) | data[5];
    

    if(reg_add < REG_TOTAL_NUM)
    {
        registers_check(reg_add);
        if(Frame.Handle_06_W)
        {
            switch (reg_add)
            {
                case REG_MOTOR_START:
                    if(Motor_CheckState())
                        Check_Frame_flag = HAL_ERROR;
                    break;
                case REG_MOTOR_DIR:
                    if(Motor_CheckDir())
                        Check_Frame_flag = HAL_ERROR;
                    break;
                case REG_MOTOR_PWM:
                    if(Motor_CheckPWM())
                        Check_Frame_flag = HAL_ERROR;
                    break;
            }
            if (Can_use_flag != Can_Use_Modbus &&
                CanState.Can_Modbus_State != CAN_MODBUS_IDLE &&
                Can_Tx_Busy_Flag != Can_Leisure)
            {
                Check_Frame_flag = HAL_ERROR;
                printf("Modbus部分ERROR!!!!\r\n");
            }

            if(Check_Frame_flag == HAL_ERROR)
            {
                error_data = 0x03;
                printf("ERROR!!!!,异常码:%02X\r\n",(unsigned int)error_data);
                data[1] = data[1] | 0x80;
                data[2] = 0x03;
                uint16_t crc = CRC_CheckL(data, 3);
                data[3] = crc & 0xFF;   // CRC低字节
                data[4] = crc >> 8;     // CRC高字节
                reg_count = 5;
                Check_Frame_flag = HAL_OK;
                
            }
            else
            {
                Frame.USE_FRAMR = TEST_NULL;
                registers[REG_MOTOR_FAULT].value = FAULT_NOERROR;
                registers[reg_add].updated = 1;//修改寄存器
                registers[reg_add].value = Modbus_reg_data;
                
                //registers[reg_add].regist = reg_data;//后期删除 这一部分 放入硬件的修改
                //Motor_Control(reg_add);
                if (Can_use_flag == Can_Use_Modbus &&
                    CanState.Can_Modbus_State == CAN_MODBUS_IDLE &&
                    Can_Tx_Busy_Flag == Can_Leisure)
                {
                    CanState.Can_Modbus_pending = Can_Modbus_request_tx;
                    CanState.Can_Modbus_State = CAN_MODBUS_TX_PENDING;

                    Can_ModbustoMotor_Send();
                }
                Frame.Handle_06_W = 0;
                reg_count = len;
            }
        }
        else
        {
            error_data = 0x02;
            printf("ERROR!!!!,异常码:%02X\r\n",(unsigned int)error_data);
            data[1] = data[1] | 0x80;
            data[2] = 0x02;
            uint16_t crc = CRC_CheckL(data, 3);
            data[3] = crc & 0xFF;   // CRC低字节
            data[4] = crc >> 8;     // CRC高字节
            reg_count = 5;
        }

    }
    else
    {
        error_data = 0x02;
        printf("ERROR!!!,异常码:%02X\r\n",error_data);
        data[1] = data[1] | 0x80;
        data[2] = 0x02;
        uint16_t crc = CRC_CheckL(data, 3);
        data[3] = crc & 0xFF;   // CRC低字节
        data[4] = crc >> 8;     // CRC高字节
        reg_count = 5;

    }
    
    printf("0x06功能码 10进制回复帧为:");
    for(int i = 0;i < reg_count;i++)
    printf("%d ",data[i]);
    printf("\r\n");
    printf("0x06功能码 16进制回复帧为:");
    for(int i = 0;i < reg_count;i++)
    printf("%02X ",data[i]);
    
    printf("\r\n");
    //HAL_UART_Transmit(&huart1,data,reg_count,HAL_MAX_DELAY);
    Modbus_arr = reg_count;
    RsState.tx_pending = WAIT;
    memcpy(Modbus_Tx_data_06,data,Modbus_arr);
    //RS485_SendByte_Length(Tx_data,reg_count);

    /*modbus测试帧
    if(Modbus_busy_frame_flag == 0)
    {
        if(HAL_UART_Transmit_IT(&huart1, Tx_data, reg_count) == HAL_OK)
        {
            Modbus_busy_frame_flag = 1;
        }
    }
    */
    
    Frame.Handle_06_W = TEST_NULL;
    Frame.USE_FRAMR = TEST_NULL;
    Check_Frame_flag = HAL_OK;
    
}


void Handle_10(uint8_t* data,uint16_t len)
{
    Frame.pending_response_type = TEST_10;
    Check_Frame_flag = HAL_OK;
    if(len >= 9)
    {
        uint16_t reg_num;//寄存器数量
        uint16_t reg_add;//起始地址
        //uint16_t reg_data;//寄存器的值
        uint8_t reg_abit;//寄存器总共占的字节数
        uint8_t error_data;//异常码
        uint16_t reg_tadd;//测试地址
        reg_add = (data[2] << 8) | data[3]; 
        reg_num = (data[4] << 8) | data[5];
        reg_abit = data[6];
        reg_tadd = reg_add;//测试地址
    
        if((reg_num > 0 && reg_num <= 8) && ((reg_abit == reg_num * 2) && len == 9 + 2*reg_num))
        {
            if(reg_add < REG_TOTAL_NUM && reg_num <= REG_TOTAL_NUM - reg_add)
            {
                for(int i = 0;i < reg_num;i++)
                {
                    registers_check(reg_tadd++);
                    if(Frame.Handle_10_W == TEST_NULL)
                    {
                        Frame.USE_FRAMR = TEST_ADD_ERROR;
                    }
                }
                if(Frame.USE_FRAMR != TEST_ADD_ERROR)
                {
                    uint32_t reg_Padd = reg_add;
                    for(int m = 0;m < reg_num;m++)
                    {
                        reg_Padd = reg_add + m;
                        Modbus_reg_data = (data[7 + 2*m] << 8) | data[8 + 2*m];
                        
                        
                        switch (reg_Padd)
                        {
                            case REG_MOTOR_START:
                                if(Motor_CheckState())
                                    Check_Frame_flag = HAL_ERROR;
                                break;
                            case REG_MOTOR_DIR:
                                if(Motor_CheckDir())
                                    Check_Frame_flag = HAL_ERROR;
                                break;
                            case REG_MOTOR_PWM:
                                if(Motor_CheckPWM())
                                    Check_Frame_flag = HAL_ERROR;
                                break;
                        }
                        if (Can_use_flag != Can_Use_Modbus ||
                            CanState.Can_Modbus_State != CAN_MODBUS_IDLE ||
                            Can_Tx_Busy_Flag != Can_Leisure)
                        {
                            Check_Frame_flag = HAL_ERROR;
                            printf("Modbus部分ERROR!!!!\r\n");
                        }
                        if(Check_Frame_flag == HAL_ERROR)
                        {
                            Check_Frame_flag = HAL_OK;
                            Frame.USE_FRAMR = TEST_DATA_ERROR;
                        }
                            
                    }
                    
                    if(Frame.USE_FRAMR != TEST_ADD_ERROR && Frame.USE_FRAMR != TEST_DATA_ERROR)
                    {
                        Frame.USE_FRAMR = TEST_NULL;
                        registers[REG_MOTOR_FAULT].value = FAULT_NOERROR;
                        printf("数据全部合法！！！\r\n");
                        int n = 7;
                        for(int j = 0;j < reg_num; j++)
                        {
                            Modbus_reg_data = (data[n] << 8) | data[n+1];
                            n += 2;
                            registers[reg_add].value = Modbus_reg_data;
                            registers[reg_add].updated = 1;
                            //Motor_Control(reg_add++);
                            reg_add++;
                        }
                        if (Can_use_flag == Can_Use_Modbus &&
                            CanState.Can_Modbus_State == CAN_MODBUS_IDLE &&
                            Can_Tx_Busy_Flag == Can_Leisure)
                        {
                            CanState.Can_Modbus_pending = Can_Modbus_request_tx;
                            CanState.Can_Modbus_State = CAN_MODBUS_TX_PENDING;

                            Can_ModbustoMotor_Send();
                        }
                        else
                        {
                            printf("modbus异常！！！！\r\n");
                        }
                                                    

                        REG10_Back(data,len,reg_num);
                    }
                }
                
            }
            else
            {
                Frame.USE_FRAMR = TEST_ADD_ERROR;
            }
        }
        else
        {
            Frame.USE_FRAMR = TEST_DATA_ERROR;
        }

        if(Frame.USE_FRAMR == TEST_ADD_ERROR)
        {
            printf("Address ERROR!!\r\n");
            error_data = 0x02;
            data[1] = 0x90;
            data[2] = error_data;
            printf("ERROR!!!!,非法数据地址,异常码:%02X\r\n",(unsigned int)error_data);
            REG10_Back(data,len,reg_num);
        }
        else if(Frame.USE_FRAMR == TEST_DATA_ERROR && Frame.USE_FRAMR != TEST_ADD_ERROR)
        {
            printf("DATA ERROR!!!!\r\n");
            error_data = 0x03;
            data[1] = 0x90;
            data[2] = error_data;
            printf("ERROR!!!!,非法数据值,异常码:%02X\r\n",(unsigned int)error_data);
            REG10_Back(data,len,reg_num);
        }
    }
    else
    {
        Error_frame_flag = 1;
        printf("是残缺帧！！！\r\n");
        Frame.pending_response_type = TEST_NULL;
    }
    Frame.Handle_10_W = TEST_NULL;
    Frame.USE_FRAMR = TEST_NULL;
}

void REG10_Back(uint8_t* data,uint16_t len,uint16_t reg_num)
{
    uint8_t frame_back[8];
    uint16_t crc;
    uint8_t u_num;


    if(data[1] == 0x90)//说明有错误
    {
        
        frame_back[0] = data[0];
        frame_back[1] = 0x90;
        frame_back[2] = data[2];
        crc = CRC_CheckL(frame_back, 3);
        frame_back[3] = crc & 0xFF;   // CRC低字节
        frame_back[4] = crc >> 8;     // CRC高字节
        frame_back[5] = '\0';
    }
    else
    {
        u_num = 8;
        for(int i = 0;i < u_num - 2;i++)
        {
            frame_back[i] = data[i];
        }
        crc = CRC_CheckL(frame_back,u_num-2);
        frame_back[u_num - 2] = crc & 0xFF;   // CRC低字节
        frame_back[u_num - 1] = crc >> 8;     // CRC高字节
    }

    Modbus_arr = (data[1] == 0x90) ? 5 : u_num;

    printf("0x10功能码 10进制回复帧为:");
    for(int j = 0; j < Modbus_arr; j++)
    printf("%d ",frame_back[j]);
    printf("\r\n");
    printf("0x10功能码 16进制回复帧为:");
    for(int j = 0; j < Modbus_arr; j++)
    printf("%02X ",frame_back[j]);
    printf("\r\n");

    
    //HAL_UART_Transmit(&huart1,Tx_data_10,arr,HAL_MAX_DELAY);
    
    RsState.tx_pending = WAIT;
    memcpy(Modbus_Tx_data_10,frame_back,Modbus_arr);
    //RS485_SendByte_Length(Tx_data,arr);

    /* modbus测试
    if(Modbus_busy_frame_flag == 0)
    {
        if(HAL_UART_Transmit_IT(&huart1, Tx_data_10, arr) == HAL_OK)
        {
            Modbus_busy_frame_flag = 1;
        }
    }
    */

}

void Handle_ErrorFrame(uint8_t* data,uint16_t len)
{
    Frame.pending_response_type = TEST_ERROR_Frame;
    Modbus_Error_Data[0] = data[0];
    Modbus_Error_Data[1] = data[1] | 0x80;
    Modbus_Error_Data[2] = 0x01;
    uint16_t crc = CRC_CheckL(Modbus_Error_Data, 3);
    Modbus_Error_Data[3] = crc & 0xFF;   // CRC低字节
    Modbus_Error_Data[4] = crc >> 8;     // CRC高字节

    Modbus_arr = 5;
    printf("未知功能码 10进制回复帧为:");
    for(int j = 0; j < Modbus_arr; j++)
    printf("%d ",Modbus_Error_Data[j]);
    printf("\r\n");
    printf("未知功能码功能码 16进制回复帧为:");
    for(int j = 0; j < Modbus_arr; j++)
    printf("%02X ",Modbus_Error_Data[j]);
    printf("\r\n");

    //HAL_UART_Transmit(&huart1,Tx_data_10,arr,HAL_MAX_DELAY);
    
    RsState.tx_pending = WAIT;
    memcpy(Modbus_Tx_data,Modbus_Error_Data,Modbus_arr);

    
    /* modbus测试
    if(Modbus_busy_frame_flag == 0)
    {
        if(HAL_UART_Transmit_IT(&huart1, Tx_data_10, arr) == HAL_OK)
        {
            Modbus_busy_frame_flag = 1;
        }
    }
    */

}





