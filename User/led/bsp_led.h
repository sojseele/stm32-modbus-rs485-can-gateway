
#ifndef __BSP_LED_H
#define __BSP_LED_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"


void LED_GPIO_Config(void);


#define LED_G_Pin         GPIO_PIN_0
#define LED_G_GPIO_Port   GPIOB


#define LED_R_Pin         GPIO_PIN_5
#define LED_R_GPIO_Port   GPIOB

#define LED_B_Pin         GPIO_PIN_1
#define LED_B_GPIO_Port   GPIOB


//LED���ƺ�  �ߵ�ƽ��
//���ƺ��
#define R_LED_ON()              HAL_GPIO_WritePin(LED_R_GPIO_Port,LED_R_Pin,GPIO_PIN_RESET)  //�������
#define R_LED_OFF()             HAL_GPIO_WritePin(LED_R_GPIO_Port,LED_R_Pin,GPIO_PIN_SET)    //Ϩ����
#define R_LED_TOGGLE()          HAL_GPIO_TogglePin(LED_R_GPIO_Port,LED_R_Pin)                //��ת���״̬

//�����̵�
#define G_LED_ON()              HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_RESET)         //�����̵�
#define G_LED_OFF()             HAL_GPIO_WritePin(LED_G_GPIO_Port,LED_G_Pin,GPIO_PIN_SET)    //Ϩ���̵�
#define G_LED_TOGGLE()          HAL_GPIO_TogglePin(LED_G_GPIO_Port,LED_G_Pin)                //��ת�̵�״̬

#define B_LED_ON()              HAL_GPIO_WritePin(LED_B_GPIO_Port, LED_B_Pin, GPIO_PIN_RESET)         //�����̵�
#define B_LED_OFF()             HAL_GPIO_WritePin(LED_B_GPIO_Port,LED_B_Pin,GPIO_PIN_SET)    //Ϩ���̵�
#define B_LED_TOGGLE()          HAL_GPIO_TogglePin(LED_B_GPIO_Port,LED_B_Pin)      

/*--------------------------------��Ͽ��ƺ�----------------------------*/
//����ȫ��
#define RGB_ALL_ON()            R_LED_ON();G_LED_ON();B_LED_ON()

//����ȫ��
#define RGB_ALL_OFF()           R_LED_OFF();G_LED_OFF();B_LED_OFF()

//�������
#define R_LED_ON_ONLY()         R_LED_ON();G_LED_OFF();B_LED_OFF()

//�����̵� 
#define G_LED_ON_ONLY()         R_LED_OFF();G_LED_ON();B_LED_OFF()

#define B_LED_ON_ONLY()         R_LED_OFF();G_LED_OFF();B_LED_ON()






#endif /*__BSP_LED_H */

