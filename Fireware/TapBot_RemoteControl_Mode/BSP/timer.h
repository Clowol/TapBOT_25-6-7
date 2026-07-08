/******************** (C) COPYRIGHT 2026 *****************************************
 * @file        timer.h
 * @brief       Timer and global volatile flag declarations shared between ISR and main loop.
 * @note        
 * @warning     
 * @license     This project is released under the MIT License.
 *********************************************************************************/
#ifndef __TIMER_H
#define __TIMER_H

#include "sys.h"
// #include "debug.h"
#include "usart.h"


/**************************************************************************
  * @brief  全局变量定义
  * @param	时间片标志和接收计数器	 （中断中修改，主程序读取）
  * @param	通信超时计数器			（中断中递减，主程序重置）
  * @param	通信状态标志			（中断中修改，主程序判断）	
  * @note   
***********************************************************************/
extern volatile u16 flag10ms;
extern volatile u16 CntRx1;
extern volatile u16 CntRx2;
extern volatile u16 CntRx4;
extern volatile u16 CntRx5;

extern volatile u16 CntRxUp;							// 上层通信接收帧计数
extern volatile u16 CntRxRmt;							// 遥控器通信接收帧计数
extern volatile u16 CntRxCAN1;						// CAN1通信接收帧计数
extern volatile u16 CntRxCAN2;						// CAN2通信接收帧计数

extern volatile u8 UpOutCommunFlg;					// 上层通信失联标志位   	0-正常  1-失联
extern volatile u8 RmtOutCommunFlg;					// 遥控器通信失联标志位	0-正常  1-失联
extern volatile u8 CAN1_OutCommunFlg;				// CAN1通信失联标志位		0-正常  1-失联
extern volatile u8 CAN2_OutCommunFlg;				// CAN2通信失联标志位		0-正常  1-失联
extern volatile u8 RmtPwrOffFlg;						// 遥控器掉电标志位			0-正常  1-失联

/**************************************************************************
  * @name   Timer3_Init(u16 arr, u16 psc);
  * @brief  定时器3初始化
  * @param	arr 自动重装载值  psc 预分频值
  * @retval none
***********************************************************************/
void Timer3_Init(u16 arr,u16 psc);

/**************************************************************************
  * @name   Timer4_Init(u16 arr, u16 psc);
  * @brief  定时器4初始化
  * @param	arr 自动重装载值  psc 预分频值	
  * @retval none 
***********************************************************************/
void Timer4_Init(u16 arr,u16 psc);



#endif /* __TIMER_H */

/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/
