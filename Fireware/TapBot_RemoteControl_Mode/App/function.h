/******************** (C) COPYRIGHT 2026 *****************************************
 * @file        function.h
 * @brief       Application-layer shared declarations, periodic task prototypes, and helpers.
 * 
 * @note        
 * @warning     
 * @license     This project is released under the MIT License.
 *********************************************************************************/
#ifndef __FUNCTION_H
#define __FUNCTION_H

#include "stm32f10x.h"

#include "app_config.h"
#include "sys.h"
#include "led.h"
#include "timer.h"
#include "usart.h"
#include "adc.h"
#include "switch.h"
#include "encoder.h"
#include "wit_imu.h"
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
#include "arm_auto_task.h"

#include "upper_feedback.h"
#include "subboard_link.h"
#include "subboard_protocol.h"

#include "string.h"

/***************************************************************************************************
 * @name   app_control_mode_t
 * @brief  System control mode enumeration.
 * @param  MAN_MODE   Manual / direct-control mode (reserved).
 * @param  RMT_MODE   Remote-controller mode (default).
 * @param  ROCK_MODE  Rocker / extended control mode (reserved).
 * @param  UP_MODE    Upper-computer (ROS2) control mode.
 * @note   g_RmtUpManCtrlMode uses this enum to switch the active control source.
 ***************************************************************************************************/
typedef enum
{
    MAN_MODE  = 1,
    RMT_MODE  = 2,
    ROCK_MODE = 3,
    UP_MODE   = 4
} app_control_mode_t;

/********************************** Global State Variables ********************************************/
extern u8 g_RmtUpManCtrlMode;           // the desired control mode
extern u8 g_RmtUpManRealMode;           // The actually effective mode
extern MOTOR_send cmd;
extern MOTOR_recv data;

/********************************** Periodic Task Functions ********************************************/
void FunctionProce(void);
void Function_10ms(void);
void Function_30ms(void);
void Function_50ms(void);
void Function_100ms(void);
void Function_200ms(void);
void Function_300ms(void);
void Function_500ms(void);
void Function_1s(void);

/********************************** Helper / Utility Functions ********************************************/
void LED_WorkCtrlFun(void);
void USART1_Proc(void);
void USART2_Proc(void);
void USART4_Proc(void);
void USART5_Proc(void);
void function_prtf(void);               // Debugging and printing related


#endif /* __FUNCTION_H */


/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/


