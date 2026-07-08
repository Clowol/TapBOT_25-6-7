/******************** (C) COPYRIGHT 2026 *****************************************
 * @file        delay.h
 * @brief       SysTick delay function prototypes.
 * @note        
 * @warning     
 * @license     This project is released under the MIT License.
 *********************************************************************************/
#ifndef __DELAY_H
#define __DELAY_H

#include "stm32f10x.h"
#include "sys.h"


/**********************  Task Functions ********************************************/
void delay_init(u8 SYSCLK);

void delay_us(u32 nus);

void delay_ms(u16 nms);
void delay_ms_extend(u32 nms);

void delay_s(u8 ns);

#endif /* __DELAY_H */


/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/
