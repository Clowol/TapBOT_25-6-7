/******************** (C) COPYRIGHT 2026 *****************************************
   * @author      Clomol / Xiang
   * @date        2026-2027
   * @brief       timer delay functions.
   * @license     [z]本代码仅用于教学与科研目的，未经作者书面许可，不得用于商业用途
   *              This project is released under the MIT License.
   * @note        
   * @warning     
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
extern u16 flag10ms;
extern u16 CntRx1;
extern u16 CntRx2;
extern u16 CntRx4;
extern u16 CntRx5;

extern u16 CntRxUp;							// 上层通信接收帧计数
extern u16 CntRxRmt;							// 遥控器通信接收帧计数
extern u16 CntRxCAN1;						// CAN1通信接收帧计数
extern u16 CntRxCAN2;						// CAN2通信接收帧计数

extern u8 UpOutCommunFlg;					// 上层通信失联标志位   	0-正常  1-失联
extern u8 RmtOutCommunFlg;					// 遥控器通信失联标志位	0-正常  1-失联
extern u8 CAN1_OutCommunFlg;				// CAN1通信失联标志位		0-正常  1-失联
extern u8 CAN2_OutCommunFlg;				// CAN2通信失联标志位		0-正常  1-失联
extern u8 RmtPwrOffFlg;						// 遥控器掉电标志位			0-正常  1-失联

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
