/******************** (C) COPYRIGHT 2026 *****************************************
 * @author      Clomol
 * @date        2025-2026
 * @file        function.h
 * @brief       Task Scheduling Center.
*********************************************************************************/
#ifndef __FUNCTION_H
#define __FUNCTION_H

#include "stm32f10x.h"
#include "app_config.h"
// #include "debug.h"
#include "sys.h"
#include "led.h"
#include "timer.h"
#include "usart.h"
#include "adc.h"
#include "switch.h"
#include "can1.h"
#include "can2.h"
#include "rmt_data.h"
#include "motor_control.h"
#include "steer_data.h"
#include "ptz_data.h"
#include "yushu_motor_data.h"
#include "ros2_comm.h"
#include "rmt_comm.h"
#include "control_dispatcher.h"

#include "string.h"


enum 
{
	MAN_MODE = 1,			// 有人驾驶模式
	RMT_MODE = 2,			// 遥控器模式
	ROCK_MODE= 3,           // 手动摇杆驾驶模式
	UP_MODE  = 4,			// 上层模式
};



/************************************************ 全局变量定义 ****************************************************/
/**
 * @brief 系统期望工作模式（执行值）
 * @param 底层工作模式标志;
 * 		  1-遥控器模式  2-上层模式   3-有人驾驶模式
 * @note  由遥控器/上位机设置，用于切换系统工作模式，需与g_SystemRealWorkMode区分
 */
extern u8 g_RmtUpManCtrlMode;	


/**
 * @brief 系统实际工作模式（真实值）
 * @param 底层工作模式标志
 * 		  1-遥控器模式  2-上层模式   3-有人驾驶模式
 * @note  反映系统当前实际运行的模式，由模式切换逻辑更新，禁止外部直接修改
 */
extern u8 g_RmtUpManRealMode;		

/**
 * @brief 电机控制命令结构体
 */
extern MOTOR_send cmd;   


/*********************************** 函数声明 ***************************************/
void LED_WorkCtrlFun(void);

void FunctionProce(void);    
void Function_10ms(void);
void Function_30ms(void);
void Function_50ms(void);
void Function_100ms(void);
void Function_200ms(void);
void Function_300ms(void);
void Function_500ms(void);
void Function_1s(void);


void USART1_Proc(void);			// 宇树电机 	485串口->接收数据
void USART2_Proc(void);			// 上层通信 	232串口->接收数据
void USART4_Proc(void);			// 云台			485串口->接收数据
void USART5_Proc(void);			// 舵机			485串口->接收数据

/**
 * @brief 系统状态打印函数
 * @note  用于调试，打印系统状态到串口	（USART2输出）
 */
void function_prtf(void);


#endif /* __FUNCTION_H */

/******************* (C) COPYRIGHT 2026 END OF FILE *****************************/

