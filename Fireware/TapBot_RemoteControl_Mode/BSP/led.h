/******************** (C) COPYRIGHT 2026 *****************************************
   * @author      Clomol
   * @date        2026-2027
   * @brief       
   * @license     [z]本代码仅用于教学与科研目的，未经作者书面许可，不得用于商业用途
   *              This project is released under the MIT License.
   * @note        
   * @warning     
*********************************************************************************/
#ifndef __LED_H
#define __LED_H

#include "stm32f10x.h"
#include "delay.h"

extern u8 led_debug_flag;


/***************************  宏定义 -> (控制LED开关) *****************************/
/** 
 * @brief  Macro definition to trigger the LED on or off
 * @param  a: LED state (1 - off, 0 - on)
 * @retval None
 */
#define LED_ON  0
#define LED_OFF 1

// 高低电平
#define LED1(a)	if (a)	\
					GPIO_SetBits(GPIOE,GPIO_Pin_1);\
					else		\
					GPIO_ResetBits(GPIOE,GPIO_Pin_1)

                    
//	切换 LED 的状态
#define LED_DEBUG()		led_debug_flag=~led_debug_flag;LED1(led_debug_flag)



/*******************************  函数申明  ************************************/
void LED_Init(void);
void LED_debug(void);

#endif /* __LED_H */

/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/
