
#include "modbus/app_modbus.h"
#include "modbus/bsp_modbus.h"
#include "usart/bsp_usart.h"
#include "led/bsp_led.h"
#include "rs485/bsp_rs485.h"
#include "protocol/bsp_can_protocol.h"

uint8_t Receive_Buff[RX_BUFFER_SIZE];
volatile uint16_t Receive_Size;
volatile uint8_t Receive_flag;
volatile uint8_t CRC_flag;
volatile uint8_t Error_frame_flag = 0;//0=没有错误 1= 发生错误



void Modbus_Init()
{
    registers_init();
    Receive_flag = 0;
    CRC_flag = 0;
    Receive_Buf();
    //HAL_UARTEx_ReceiveToIdle_DMA(&huart1,Receive_Buff,RX_BUFFER_SIZE);
    printf("初始化modbus slaver\r\n");
}

int modbustest()
{
    if(Error_frame_flag == 1)
    {
        Receive_flag = 0;
        RsState.rx_frame_ready = WAIT;
        Error_frame_flag = 0;
        Receive_Buf();
        return 0;
    }
    if(RsState.tx_busy == FINISH)
    {
        if(Receive_flag == 1 )
        {
            if(Receive_Size >= 4)
            {
                uint16_t count = Receive_Size;
                uint16_t crc_c = CRC_CheckL(Receive_Buff,count-2);
                uint16_t crc_temp = (Receive_Buff[Receive_Size- 1] << 8) | Receive_Buff[Receive_Size - 2];
                (crc_c == crc_temp) ? (CRC_flag = 1) : (CRC_flag = 0);
                if(CRC_flag)
                {
                    AddrResult filration = Add_filtration(Receive_Buff);
                    for(int i = 0; i< count; i++)
                    printf("接收到的数据是：%d\r\n",Receive_Buff[i]);
                    printf("长度：%d\r\n",Receive_Size);
                    switch (filration)
                    {
                    //收到本机地址
                    case ADDR_MATCH:
                        Check_Fuction(Receive_Buff,Receive_Size);
                        
                        break;
                    //收到广播地址 广播功能暂未实现
                    case ADDR_BROADCAST:
                        Error_frame_flag = 1;
                        break;
                    //接收到无用信息
                    case ADDR_IGNORE:
                        Error_frame_flag = 1;
                        break;
                    default:
                        Error_frame_flag = 1;

                    }
                    Receive_flag = 0;
                        
                    //HAL_UARTEx_ReceiveToIle_DMA(&huart1, Receive_Buff, RX_BUFFER_SIZE);
                    CRC_flag = 0;
                }
                else
                {
                    printf("CRC校验失败！！！\r\n");
                    Receive_flag = 0;
                    Error_frame_flag = 1;
                    //HAL_UARTEx_ReceiveToIdle_DMA(&huart1, Receive_Buff, RX_BUFFER_SIZE);
                }
                
            }
            else
            {
                printf("这是一个残缺帧，直接丢弃了!!!!\r\n");
                Error_frame_flag = 1;
                //HAL_UARTEx_ReceiveToIdle_DMA(&huart1, Receive_Buff, RX_BUFFER_SIZE);
            }
        }
    }
    return 0;
}

/*
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if(huart->Instance == USART1)
    {
        Receive_Size = Size;
        Receive_flag = 1;
    }
}
*/

void Check_Fuction(uint8_t *data ,uint16_t len)
{
    uint8_t Funtion = data[1];
    printf("接收到的功能码：%d\r\n",Funtion);
    printf("\r\n");
    Can_use_flag = Can_Use_Modbus;
    switch(Funtion)
    {
        case 0x03:
        {
            if(len != 8)
            {
                printf("错误帧！！！！\r\n");
                Error_frame_flag = 1;
                break;
            }
            handle_03(data, len);
            R_LED_ON();
            //HAL_Delay(1000);
            R_LED_OFF();
            break;
        }
            
        case 0x06:
        {
            if(len != 8)
            {
                printf("错误帧！！！！\r\n");
                Error_frame_flag = 1;
                break;
            }
            Frame.USE_FRAMR = TEST_06;
            Handle_06(data,len);
            G_LED_ON();
            //HAL_Delay(1000);
            G_LED_OFF();
            
            break;
        }
        case 0x10:
        {
            Frame.USE_FRAMR = TEST_10;
            Handle_10(data,len);
            B_LED_ON();
            //HAL_Delay(1000);
            B_LED_OFF();
            
            break;
        }
        default:
        {
            Handle_ErrorFrame(data,len);
            printf("无效功能码！！！\r\n");
            break;
        }
    }

}

