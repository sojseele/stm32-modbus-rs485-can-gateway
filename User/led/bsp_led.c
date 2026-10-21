/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "led/bsp_led.h"


/**
  *  @brief   初始化控制LED的GPIO引脚
  *  @note    配置为推挽输出，默认全灭
	*/
	
void LED_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	//使能时钟端口
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
	
	//设置引脚为推挽输出 绿灯
	GPIO_InitStruct.Pin = LED_G_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;   //推挽输出
	GPIO_InitStruct.Pull = GPIO_NOPULL;           //既不上拉也不下拉
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;  //低速输出
	HAL_GPIO_Init(LED_G_GPIO_Port,&GPIO_InitStruct);
	
	//设置引脚为推挽输出 红灯
	GPIO_InitStruct.Pin = LED_R_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;   //推挽输出
	GPIO_InitStruct.Pull = GPIO_NOPULL;           //既不上拉也不下拉
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;  //低速输出
	HAL_GPIO_Init(LED_R_GPIO_Port,&GPIO_InitStruct);
	
		//设置引脚为推挽输出 红灯
	GPIO_InitStruct.Pin = LED_B_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;   //推挽输出
	GPIO_InitStruct.Pull = GPIO_NOPULL;           //既不上拉也不下拉
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;  //低速输出
	HAL_GPIO_Init(LED_B_GPIO_Port,&GPIO_InitStruct);
	
	
	//初始化关闭所有LED(高电平点亮，默认为低电平)
	HAL_GPIO_WritePin(LED_G_GPIO_Port,LED_G_Pin,GPIO_PIN_SET);
	HAL_GPIO_WritePin(LED_R_GPIO_Port,LED_R_Pin,GPIO_PIN_SET);
	
	HAL_GPIO_WritePin(LED_B_GPIO_Port,LED_B_Pin,GPIO_PIN_SET);
}


















