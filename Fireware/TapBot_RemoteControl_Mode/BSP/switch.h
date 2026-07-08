/******************** (C) COPYRIGHT 2026 *****************************************
 * @file        switch.h
 * @brief       Switch pin definitions and input macros.
 * @note        
 * @warning     
 * @license     This project is released under the MIT License.
 *********************************************************************************/
#ifndef __SWITCH_H
#define __SWITCH_H

#include "stm32f10x.h"
#include "delay.h"

/***************************************************************************************************
 * @name	
 * @brief  开关量输入输出用到的I/O口定义
 * @param     
 *		SW1*  SW2*  SW3  SW4  	SW5  SW6  SW7  	SW8	 SW9  	SW10  SW11  SW12  SW13  SW14  	SW15  SW16
 *		PD12 PD13 PD14 PD15 	PC6  PC8  PC9  	PE13 PE14 	PE2   PE3   PE4   PE5   PE6   	PA11  PA12
 * @else	
 *		IN1  	IN2  IN3  IN4  	IN5  	IN6  	IN7  	IN8	 	IN9  IN10  	IN11  IN12*
 *		PE11 	PB3  PB4  PB7  	PA1  	PE9  	PA15 	PC7   	PD10 PD11  	PD7   PD4
 *
 * @note 	SW1~SW16 对应输出引脚; IN1~IN12 对应输入引脚
 ***************************************************************************************************/

#define ON  1
#define OFF 0

#define SW1(a)	if (a)	\
					GPIO_SetBits(GPIOD,GPIO_Pin_12);\
					else		\
					GPIO_ResetBits(GPIOD,GPIO_Pin_12)

#define SW2(a)	if (a)	\
					GPIO_SetBits(GPIOD,GPIO_Pin_13);\
					else		\
					GPIO_ResetBits(GPIOD,GPIO_Pin_13)

#define SW3(a)	if (a)	\
					GPIO_SetBits(GPIOD,GPIO_Pin_14);\
					else		\
					GPIO_ResetBits(GPIOD,GPIO_Pin_14)

#define SW4(a)	if (a)	\
					GPIO_SetBits(GPIOD,GPIO_Pin_15);\
					else		\
					GPIO_ResetBits(GPIOD,GPIO_Pin_15)

#define SW5(a)	if (a)	\
					GPIO_SetBits(GPIOC,GPIO_Pin_6);\
					else		\
					GPIO_ResetBits(GPIOC,GPIO_Pin_6)

#define SW6(a)	if (a)	\
					GPIO_SetBits(GPIOC,GPIO_Pin_8);\
					else		\
					GPIO_ResetBits(GPIOC,GPIO_Pin_8)

#define SW7(a)	if (a)	\
					GPIO_SetBits(GPIOC,GPIO_Pin_9);\
					else		\
					GPIO_ResetBits(GPIOC,GPIO_Pin_9)

#define SW8(a)	if (a)	\
					GPIO_SetBits(GPIOE,GPIO_Pin_13);\
					else		\
					GPIO_ResetBits(GPIOE,GPIO_Pin_13)
					
#define SW9(a)	if (a)	\
					GPIO_SetBits(GPIOE,GPIO_Pin_14);\
					else		\
					GPIO_ResetBits(GPIOE,GPIO_Pin_14)
					
#define SW10(a)	if (a)	\
					GPIO_SetBits(GPIOE,GPIO_Pin_2);\
					else		\
					GPIO_ResetBits(GPIOE,GPIO_Pin_2)
					
#define SW11(a)	if (a)	\
					GPIO_SetBits(GPIOE,GPIO_Pin_3);\
					else		\
					GPIO_ResetBits(GPIOE,GPIO_Pin_3)
					
#define SW12(a)	if (a)	\
					GPIO_SetBits(GPIOE,GPIO_Pin_4);\
					else		\
					GPIO_ResetBits(GPIOE,GPIO_Pin_4)
					
#define SW13(a)	if (a)	\
					GPIO_SetBits(GPIOE,GPIO_Pin_5);\
					else		\
					GPIO_ResetBits(GPIOE,GPIO_Pin_5)
					
#define SW14(a)	if (a)	\
					GPIO_SetBits(GPIOE,GPIO_Pin_6);\
					else		\
					GPIO_ResetBits(GPIOE,GPIO_Pin_6)
					
#define SW15(a)	if (a)	\
					GPIO_SetBits(GPIOA,GPIO_Pin_11);\
					else		\
					GPIO_ResetBits(GPIOA,GPIO_Pin_11)
					
#define SW16(a)	if (a)	\
					GPIO_SetBits(GPIOA,GPIO_Pin_12);\
					else		\
					GPIO_ResetBits(GPIOA,GPIO_Pin_12)
					
// Input switch read macros
#define 	IN1     					GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_11)  
#define 	IN2     					GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_3) 
#define 	IN3      					GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_4)  
#define 	IN4      					GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_7)  
#define 	IN5    						GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1)  
#define 	IN6    						GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_9)  
#define 	IN7    						GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_15) 
#define 	IN8    						GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_7)  
#define 	IN9    						GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_10) 
#define 	IN10    					GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_11)  
#define 	IN11    					GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_7) 
#define 	IN12    					GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_4)  


//  macro redefinition for specific switches
#define   SW_CLUTCH(a)    				SW1(a)        // 离合器 
#define   SW_BOOT_LED(a)    			SW2(a)        // BOOT按钮上的呼吸灯 
					

#define 	LIMIT_SWITCH_IN  			IN12         // 接近开关


void Switch_Init(void);

#endif /* __SWITCH_H */

/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/
