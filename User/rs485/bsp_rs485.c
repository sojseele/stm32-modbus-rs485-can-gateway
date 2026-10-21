
#include "rs485/bsp_rs485.h"
#include "usart/bsp_usart.h"
#include "modbus/bsp_modbus.h"
#include "modbus/app_modbus.h"

uint8_t uart_rx_byte;//接收数据存储
uint8_t uart_Buffer[UART_BUFF_SIZE];
//uint16_t Receive_Buffer[UART_BUFF_SIZE];//接收的数据
//uint8_t Receive_Buff[RX_BUFFER_SIZE];
volatile uint16_t Receive_len;
volatile uint16_t uart_p = 0;//接收数据长度 
volatile uint8_t Rs485_Tx_busy_flag = 0;//发送是否忙碌
volatile uint8_t Rs485_Rx_busy_flag = 0;

void RS485_Config(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

}



//-----------------------------  发送数据  -----------------------------------//
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART2)
	{
		Rs485_Tx_busy_flag = 0;
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_2,GPIO_PIN_RESET);
		RsState.tx_busy = FINISH;
		RsState.rx_frame_ready = WAIT;
		RsState.tx_pending = WAIT;
		Receive_Buf();
	}
}

//发送1个字节
void RS485_SendByte(uint8_t ch)
{
	
	if(Rs485_Tx_busy_flag == 0)
	{
		RsState.tx_busy = WAIT;
		RsState.tx_pending = FINISH;
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_2,GPIO_PIN_SET);
		Rs485_Tx_busy_flag = 1;
		
		if(HAL_UART_Transmit_IT(&huart2,&ch,1) != HAL_OK)
		{
			printf("ERROR,数据发送错误！！！\r\n");
			Rs485_Tx_busy_flag = 0;
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_2,GPIO_PIN_RESET);
			RsState.tx_busy = FINISH;
			RsState.rx_frame_ready = WAIT;
			Receive_Buf();
		}
	}
	else
	{
		Rs485_Tx_busy_flag = 0;
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_2,GPIO_PIN_RESET);
		RsState.tx_busy = FINISH;
		RsState.rx_frame_ready = WAIT;
		Receive_Buf();
	}
		//等待发送完成
	//while (__HAL_UART_GET_FLAG (&huart2,UART_FLAG_TC) == RESET);
	
	
}
//发送指定字节
int RS485_SendByte_Length(uint8_t* str,uint32_t len)
{
	
	if(Rs485_Tx_busy_flag == 0)
	{
		RsState.tx_busy = WAIT;
		RsState.tx_pending = FINISH;
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_2,GPIO_PIN_SET);
		Rs485_Tx_busy_flag = 1;
		
		if(HAL_UART_Transmit_IT(&huart2,str,len) != HAL_OK)
		{
			printf("ERROR,数据发送错误！！！\r\n");
			Rs485_Tx_busy_flag = 0;
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_2,GPIO_PIN_RESET);
			RsState.tx_busy = FINISH;
			RsState.rx_frame_ready = WAIT;
			RsState.tx_pending = WAIT;
			Frame.pending_response_type = TEST_NULL;
			Receive_Buf();
			return HAL_ERROR;
		}
		return HAL_OK;
	}
	return HAL_ERROR;
	

	//等待发送完成
	//while (__HAL_UART_GET_FLAG (&huart2,UART_FLAG_TC) == RESET);
	
	//HAL_GPIO_WritePin(GPIOC,GPIO_PIN_2,GPIO_PIN_RESET);
	
}

//发送任意字节
void RS485_SendString(uint8_t* str)
{
	
	int32_t len = strlen((char*)str);
	
	if(Rs485_Tx_busy_flag == 0)
	{
		RsState.tx_busy = WAIT;
		RsState.tx_pending = FINISH;
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_2,GPIO_PIN_SET);
		Rs485_Tx_busy_flag = 1;
		
		if(HAL_UART_Transmit_IT(&huart2,str,len) != HAL_OK)
		{
			printf("ERROR,数据发送错误！！！\r\n");
			Rs485_Tx_busy_flag = 0;
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_2,GPIO_PIN_RESET);
			RsState.tx_busy = FINISH;
			RsState.rx_frame_ready = WAIT;
			Receive_Buf();
		}
	}
	
	// 等待TC（发送完成），比Delay更精确，不浪费时间
    //while (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_TC) == RESET);
	
	//HAL_GPIO_WritePin(GPIOC,GPIO_PIN_2,GPIO_PIN_RESET);
	
}


//-----------------------------  接收数据  -----------------------------------//
////中断缓存串口数据
/*
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	
	if(huart->Instance == USART1)
	{
		if(uart_p < UART_BUFF_SIZE)
		{
			uart_Buffer[uart_p++] = uart_rx_byte;
		}
		else
		{
			clean_ReBuff();
		}
		HAL_UART_Receive_IT(&huart1, &uart_rx_byte, 1); // 重新启动，保持持续接收
	}
}
*/


//获取接收到的数据和长度
char *get_rebuff(uint16_t *len) 
{
	
    *len = uart_p;
    return (char *)&uart_Buffer;
}


void Receive_Buf()
{
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_2,GPIO_PIN_RESET);//更改为 接收模式
	
	if(Rs485_Rx_busy_flag == 0)
	{
		clean_ReBuff();
		if(HAL_UARTEx_ReceiveToIdle_DMA(&huart2,Receive_Buff,RX_BUFFER_SIZE) == HAL_OK)
		{
			Rs485_Rx_busy_flag = 1;
			__HAL_DMA_DISABLE_IT(huart2.hdmarx, DMA_IT_HT);
		}
	}

}
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if(huart->Instance == USART2)
    {
		Receive_flag = 1;
		Rs485_Rx_busy_flag = 0;
	   	Receive_Size = Size;
		//Receive_Buf();
    	RsState.rx_frame_ready = FINISH;//是否收到完整数据帧 1=拿到完整 0= 未拿到

    }

}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART2)
	{
		Rs485_Rx_busy_flag = 0;
		Receive_flag = 0;
		RsState.rx_frame_ready = WAIT;
		Receive_Buf();
	}
	
}

void clean_ReBuff()
{
    //uart_p = 0;
    //memset(uart_Buffer, 0, RX_BUFFER_SIZE);
	Receive_Size = 0;
	Receive_flag = 0;
	memset(Receive_Buff,0,RX_BUFFER_SIZE);

}

void RS485_DEBUG_ARRAY(uint8_t *arr, uint16_t num)
{
    uint16_t i;
    for (i = 0; i < num; i++)
    {
        printf("0x%02X ", arr[i]);
    }
    printf("\r\n");
}

