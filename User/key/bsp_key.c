
#include "key/bsp_key.h"
#include "usart/bsp_usart.h"

volatile ECU_Dashboard_turn Ecu_Dashboard_TData = {0};
volatile ECU_Dashboard_break Ecu_Dashboard_BData = {0};

void  KEY_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PA1 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

//��ⰴ���Ƿ񱻰���
uint8_t KeyScan(GPIO_TypeDef* GPIOx,uint16_t GPIO_PIN)
{
	if(HAL_GPIO_ReadPin(GPIOx,GPIO_PIN) == SET)
	{
    HAL_Delay(20);
		while(HAL_GPIO_ReadPin(GPIOx,GPIO_PIN) == SET);
		return KEY_ON();
		
	}
	else
		return KEY_OFF();
}



