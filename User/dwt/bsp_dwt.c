

#include "DWT/bsp_dwt.h"


//初始化DWT计数器

void DWT_Init(void)
{
	//使能DWT外设
	DEMCR |= (uint32_t)DEMCR_TRCENA;
	
	//DWT寄存器清零
	DWT_CYCCNT = (uint32_t)0U;    //使能寄存器前先清零
	
	//使能cortex_M DWT CYCCNT寄存器
	DWT_CTRL |= (uint32_t)DWT_CTRL_CYCCNTENA;
	
}

//读取当前时间戳
//当前时间戳即DWT_CYCCNT寄存器的值
uint32_t DWT_GetTick(void)
{
	
	return ((uint32_t)DWT_CYCCNT);
	
}


//节拍数转换为时间间隔
//tick:需要转换的节拍数
//frequency:内核时钟频率
uint32_t DWT_TickToMicrosecond(uint32_t tick,uint32_t frequency)
{
	
	return (uint32_t)(1000000.0/frequency*tick);
  
  
}

/** 
  * @brief  DWT计数实现精确延时，32位计数器
  * @param  time:延迟长度 单位 1us
  * @retval 无
  */
void DWT_DealyUs(uint32_t time)
{
	//将微秒转换为相应的时钟计数值
	uint32_t tick_duration = time * (SystemCoreClock / 1000000);
	uint32_t tick_start = DWT_GetTick();         //刚进入时的计数器值
	
	while(DWT_GetTick() - tick_start < tick_duration);
	
	
}

/** 
  * @brief  DWT计数实现精确延时，32位计数器
  * @param  time:延迟长度 单位 1ms
  * @retval 无
  */
void DWT_DealyMs(uint32_t time)
{
	for(uint32_t i = 0;i<time;i++)
	{
		DWT_DealyUs(1000);
	}
	
}
/** 
  * @brief  DWT计数实现精确延时，32位计数器
  * @param  time:延迟长度 单位 1s
  * @retval 无
  */
void DWT_DealyS(uint32_t time)
{
	for(uint32_t i = 0;i<time;i++)
	{
		DWT_DealyMs(1000);
	}
	
}

