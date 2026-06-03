/******************** (C) COPYRIGHT 2026 *****************************************
   * @author      Clomol
   * @date        2026-2027
   * @brief       应锟矫诧拷锟斤拷锟斤拷锟斤拷锟酵凤拷募锟斤拷锟斤拷锟斤拷邪锟斤拷锟较低筹拷锟斤拷锟侥ｏ拷锟酵凤拷募锟斤拷锟?   *              锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟今、匡拷锟斤拷模式锟斤拷锟斤拷锟酵碉拷锟斤拷锟斤拷凇锟?   * @license     [z]锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷诮锟窖э拷锟斤拷锟斤拷目锟侥ｏ拷未锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟缴ｏ拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷业锟斤拷途
   *              This project is released under the MIT License.
   * @note        锟斤拷锟侥硷拷锟斤拷应锟矫层公锟斤拷锟斤拷冢锟斤拷锟斤拷锟侥ｏ拷锟酵凤拷募锟绞憋拷锟斤拷锟斤拷循锟斤拷锟斤拷锟斤拷锟斤拷
   * @warning     全锟街匡拷锟狡憋拷锟斤拷锟结被遥锟截猴拷锟斤拷位锟斤拷锟斤拷锟教癸拷同锟斤拷锟绞ｏ拷锟睫革拷时锟斤拷注锟斤拷锟斤拷锟皆达拷谢锟斤拷呒锟斤拷锟?*********************************************************************************/
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
#include "upper_feedback.h"
#include "subboard_link.h"
#include "subboard_protocol.h"
#include "string.h"

/***************************************************************************************************
 * @name   app_control_mode_t
 * @brief  系统锟斤拷锟斤拷模式枚锟斤拷
 * @param  MAN_MODE  锟街讹拷/锟斤拷锟斤拷模式预锟斤拷
 * @param  RMT_MODE  遥锟斤拷锟斤拷锟斤拷锟斤拷模式
 * @param  ROCK_MODE 摇锟斤拷/锟斤拷展锟斤拷锟斤拷模式预锟斤拷
 * @param  UP_MODE   锟斤拷位锟斤拷锟斤拷锟斤拷模式
 * @note   g_RmtUpManCtrlMode 使锟矫革拷枚锟斤拷值锟斤拷锟斤拷锟斤拷前锟斤拷锟斤拷锟斤拷锟皆达拷锟? ***************************************************************************************************/
typedef enum
{
    MAN_MODE  = 1,
    RMT_MODE  = 2,
    ROCK_MODE = 3,
    UP_MODE   = 4
} app_control_mode_t;

/********************************** 全锟街匡拷锟斤拷状态锟斤拷锟斤拷 ********************************************/
extern u8 g_RmtUpManCtrlMode;
extern u8 g_RmtUpManRealMode;
extern MOTOR_send cmd;
extern MOTOR_recv data;

/********************************** 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷 ********************************************/
void FunctionProce(void);
void Function_10ms(void);
void Function_30ms(void);
void Function_50ms(void);
void Function_100ms(void);
void Function_200ms(void);
void Function_300ms(void);
void Function_500ms(void);
void Function_1s(void);

/********************************** 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷 ********************************************/
void LED_WorkCtrlFun(void);
void USART1_Proc(void);
void USART2_Proc(void);
void USART4_Proc(void);
void USART5_Proc(void);
void function_prtf(void);

#endif /* __FUNCTION_H */


/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/



