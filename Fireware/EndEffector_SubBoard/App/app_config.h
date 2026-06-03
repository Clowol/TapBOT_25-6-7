/******************** (C) COPYRIGHT 2026 *****************************************
 * @file    app_config.h
 * @brief   End-effector sub-board compile-time configuration.
 *********************************************************************************/
#ifndef __APP_CONFIG_H
#define __APP_CONFIG_H

#include "stm32f10x.h"

#define APP_SUBBOARD_UART_BAUD       115200U
#define APP_CONTROL_PERIOD_MS        10U

#define END_SERVO_NUM                4U
#define END_SERVO_POSITION_NUM       3U
#define END_SERVO_ROTATE_INDEX       3U

#define END_AUTO_PRESS_HOLD_MS       300U
#define END_AUTO_OBJECT_TIMEOUT_MS   5000U
#define END_AUTO_PUSHROD_TIMEOUT_MS  5000U

#endif /* __APP_CONFIG_H */
