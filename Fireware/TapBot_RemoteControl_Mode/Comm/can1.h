/******************** (C) COPYRIGHT 2026 *****************************************
 * @file        can1.h
 * @brief       CAN1 pin definitions, baud rate config, and function prototypes.
 *********************************************************************************/
#ifndef __CAN1_H
#define __CAN1_H

#include "stm32f10x.h"
#include "sys.h"
#include "function.h"

#include "string.h"

/********************************* 宏定义**************************************/
#define     CAN1_MAX_DLC             8U          // CAN1报文最大数据长度（字节）
#define     CAN1_MIN_DLC             2U          // CAN1报文最小数据长度（字节）
#define     CAN1_STD_ID_MAX          0x7FFU      // 标准ID最大值（11位）
#define     CAN1_EXT_ID_MAX          0x1FFFFFFFU // 扩展ID最大值（29位）
#define     CAN1_DEFAULT_BAUDRATE    500000U     // 默认波特率：500Kbps
#define     CAN1_APB1_CLK            36000000U   // F107 CAN1挂载APB1总线，时钟频率36MHz

#define     CAN1_STD_FRAME           0U          // CAN2.0A 标准帧（11位ID）
#define     CAN1_EXT_FRAME           1U          // CAN2.0B 扩展帧（29位ID）



/******************************* 变量 *********************************************/    
//  用于CAN1通行测试
extern u8 CAN1_SendMsg1[CAN1_MAX_DLC];   
//  用于CAN1发送“短报文”数据缓存
extern u8 CAN1_SendMsg2[CAN1_MIN_DLC];  

extern u8 canrxbuf1[CAN1_MAX_DLC];


/*******************************  静态函数  *******************************************/
/**
 * @brief  CAN1初始化入口函数（外部调用）
 * @param  无
 * @retval 无
 * @note   依次调用GPIO配置、NVIC配置、CAN模式配置、滤波配置
 */
void USER_CAN1_Init(void);

void CAN1_GPIO_Config(void);
void CAN1_NVIC_Config(void);
void CAN1_Init_Config(void);
void CAN1_Filter_Config(void);

/**
 * @brief  CAN1报文发送函数（外部调用）
 * @param  msg：待发送的数据缓冲区（长度≤8字节）
 *         id： CAN报文ID（标准ID：0~0x7FF，扩展ID：0~0x1FFFFFFF）
 *         dlc：报文数据长度（0~8字节）
 * @retval 发送结果：0-失败，1-成功
 * @note   支持标准/扩展ID，自动判断ID类型，超时时间100ms
 */
u8 CAN1_Send_Msg(u8 *msg, u32 id, u8 dlc);
#define Can1_Send_Msg CAN1_Send_Msg

/**
 * @brief  CAN1接收报文处理函数（内部调用）
 * @param  无
 * @retval 无
 * @note   在CAN1接收中断服务函数中调用，处理接收到的报文
 */
void CAN1_RxProce(void);


#endif /* __CAN1_H */

/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/
