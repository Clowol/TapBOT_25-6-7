/******************** (C) COPYRIGHT 2026 *****************************************
   * @author      Clomol
   * @date        2026-2027
   * @brief       Delay functions using SysTick timer.
   * @license     [z]本代码仅用于教学与科研目的，未经作者书面许可，不得用于商业用�?
   *              This project is released under the MIT License.
   * @note        
   * @warning     
*********************************************************************************/
#ifndef __DELAY_H
#define __DELAY_H

#include "stm32f10x.h"
#include "sys.h"

void delay_init(u8 SYSCLK);

void delay_us(u32 nus);

void delay_ms(u16 nms);
void delay_ms_extend(u32 nms);

void delay_s(u8 ns);

#endif /* __DELAY_H */


/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/
