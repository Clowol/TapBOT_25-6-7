/******************** (C) COPYRIGHT 2026 *****************************************
 * @file        led.h
 * @brief       LED pin definitions and control macros.
 * @note        
 * @warning     
 * @license     This project is released under the MIT License.
 *********************************************************************************/
#ifndef __LED_H
#define __LED_H

#include "stm32f10x.h"
#include "delay.h"

extern u8 led_debug_flag;


/***************************  Macro defination *****************************/
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



/*******************************  function declaration  ************************************/
void LED_Init(void);
void LED_debug(void);

#endif /* __LED_H */

/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/
