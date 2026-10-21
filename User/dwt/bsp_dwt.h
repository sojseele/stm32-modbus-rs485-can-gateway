

#ifndef __BSP_DWT_H
#define __BSP_DWT_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"

#define DEMCR               *(uint32_t *)(0xE000EDFC)
#define DWT_CTRL            *(uint32_t *)(0xE0001000)
#define DWT_CYCCNT          *(uint32_t *)(0xE0001004)

#define DEMCR_TRCENA            (1<<24)
#define DWT_CTRL_CYCCNTENA      (1<<0)

void DWT_Init(void);
uint32_t DWT_GetTick(void);
uint32_t DWT_TickToMicrosecond(uint32_t tick,uint32_t frequency);
void DWT_DealyUs(uint32_t time);
void DWT_DealyMs(uint32_t time);
void DWT_DealyS(uint32_t time);



#endif /*  __BSP_DWT_H  */

