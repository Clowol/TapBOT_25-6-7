/******************** (C) COPYRIGHT 2026 *****************************************
 * @author      Clomol
 * @date        2025-2026
 * @file        app_config.h
 * @brief       Project-level configuration values.
*********************************************************************************/
#ifndef __APP_CONFIG_H
#define __APP_CONFIG_H

/* UART baud rates */
/* UART波特率设置   */
#define APP_USART1_BAUD                 4000000U
#define APP_USART2_BAUD                 115200U
#define APP_USART3_BAUD                 115200U
#define APP_UART4_BAUD                  9600U
#define APP_UART5_BAUD                  115200U

/* UART receive and transmit buffer sizes */
/* UART接收和发送缓冲区大小设置*/
#define APP_USART1_MAX_RECV_LEN         100U
#define APP_USART2_MAX_RECV_LEN         100U
#define APP_USART3_MAX_RECV_LEN         100U
#define APP_USART4_MAX_RECV_LEN         100U
#define APP_USART5_MAX_RECV_LEN         100U

#define APP_USART2_RX_RING_BUF_SIZE     512U
#define APP_USART3_RX_RING_BUF_SIZE     512U

#define APP_USART1_DMA_TX_BSIZE         100U
#define APP_USART2_DMA_TX_BSIZE         100U
#define APP_USART3_DMA_TX_BSIZE         100U
#define APP_UART4_DMA_TX_BSIZE          100U

/* Remote controller thresholds */
/* 遥控器开关和模拟量阈值设置 */
#define RMT_SWITCH_LOW_THRESHOLD        1300U
#define RMT_SWITCH_HIGH_THRESHOLD       1700U
#define RMT_ANALOG_DEADBAND             1.0f

/* Remote controller calibration */
/* 遥控器输入输出映射设置 */
#define RMT_RUD_IN_LEFT                 283.0f
#define RMT_RUD_IN_MID                  1002.0f
#define RMT_RUD_IN_RIGHT                1722.0f
#define RMT_RUD_OUT_LEFT                -30.0f
#define RMT_RUD_OUT_MID                 0.0f
#define RMT_RUD_OUT_RIGHT               30.0f

#define RMT_GEAR_IN_BACK                1950.0f
#define RMT_GEAR_IN_MID                 1500.0f
#define RMT_GEAR_IN_FORWARD             1050.0f
#define RMT_GEAR_OUT_BACK               -50.0f
#define RMT_GEAR_OUT_MID                0.0f
#define RMT_GEAR_OUT_FORWARD            50.0f

#define RMT_Y1_IN_BACK                  1950.0f
#define RMT_Y1_IN_MID                   1500.0f
#define RMT_Y1_IN_FORWARD               1050.0f
#define RMT_Y1_OUT_BACK                 -50.0f
#define RMT_Y1_OUT_MID                  0.0f
#define RMT_Y1_OUT_FORWARD              50.0f

/* Actuator mappings */
/* 执行器映射设置 */
#define YUSHU_GEAR_IN_BACK              -50.0f
#define YUSHU_GEAR_IN_MID               0.0f
#define YUSHU_GEAR_IN_FORWARD           50.0f
#define YUSHU_SPEED_OUT_BACK            -20.0f
#define YUSHU_SPEED_OUT_MID             0.0f
#define YUSHU_SPEED_OUT_FORWARD         60.0f

#define STEER_Y1_IN_LEFT                -50.0f
#define STEER_Y1_IN_MID                 0.0f
#define STEER_Y1_IN_RIGHT               50.0f
#define STEER_SPEED_OUT_LEFT            -1000.0f
#define STEER_SPEED_OUT_MID             0.0f
#define STEER_SPEED_OUT_RIGHT           1000.0f

#define PTZ_UP_DOWN_SPEED_DEFAULT       0x20U
#define PTZ_LEFT_RIGHT_SPEED_DEFAULT    0x10U

#endif /* __APP_CONFIG_H */

/******************* (C) COPYRIGHT 2026 END OF FILE *****************************/
