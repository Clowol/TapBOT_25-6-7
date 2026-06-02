/******************** (C) COPYRIGHT 2026 *****************************************
   * @author      Clomol
   * @date        2026-2027
   * @brief       can2 接口的初始化
   *                -----------------
   *              |  PB5-CAN2-RX     |
   *              |  PB6-CAN2-TX     |
   *               -----------------       
   * @license     [z]本代码仅用于教学与科研目的，未经作者书面许可，不得用于商业用途
   *              This project is released under the MIT License.
   * @note        
   * @warning     
*********************************************************************************/
#ifndef __CAN2_H
#define __CAN2_H


#include "stm32f10x.h"
#include "function.h"

#include "string.h"

/********************************* 宏定义**************************************/
#define     CAN2_MAX_DLC             8U          // CAN2报文最大数据长度（字节）


/******************************* 变量 *********************************************/    
extern u8 CAN2_SendMsg1[CAN2_MAX_DLC];   
//  用于CAN2通行测试

extern u8 canrxbuf2[CAN2_MAX_DLC];


/*******************************  函数声明  *******************************************/
/**
 * @brief  CAN2初始化入口函数（外部调用）
 * @param  无
 * @retval 无
 * @note   依次调用GPIO配置、NVIC配置、CAN模式配置、滤波配置
 */
void USER_CAN2_Init(void);

void CAN2_GPIO_Config(void);
void CAN2_NVIC_Config(void);
void CAN2_Init_Config(void);
void CAN2_Filter_Config(void);

/**
 * @brief  CAN2报文发送函数（外部调用）
 * @param  msg：待发送的数据缓冲区（长度≤8字节）
 *         id： CAN报文ID（标准ID：0~0x7FF，扩展ID：0~0x1FFFFFFF）
 *         dlc：报文数据长度（0~8字节）
 * @retval 发送结果：0-失败，1-成功
 * @note   支持标准/扩展ID，自动判断ID类型，超时时间100ms
 */
u8 Can2_Send_Msg(u8 *msg, u32 id, u8 dlc);

/**
 * @brief  CAN2接收报文处理函数（外部调用）
 * @param  无
 * @retval 无
 * @note   在主循环中调用，处理接收到的报文
 */
void CAN2_RProce(void);



#endif /* __CAN2_H */

/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/
