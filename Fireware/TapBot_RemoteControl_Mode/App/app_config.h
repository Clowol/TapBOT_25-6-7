/******************** (C) COPYRIGHT 2026 *****************************************
 * @file        app_config.h
 * @brief       Project-level configuration constants and feature flags.
 * @note        
 * @warning     
 * @license     This project is released under the MIT License.
 *********************************************************************************/
#ifndef __APP_CONFIG_H
#define __APP_CONFIG_H

/* UART baud rates */
#define APP_USART1_BAUD                 4000000U        // YuShu
#define APP_USART2_BAUD                 115200U         // UpControl
#define APP_USART3_BAUD                 115200U         // remote
#define APP_UART4_BAUD                  9600U           // ptz
#define APP_UART5_BAUD                  115200U         // Sub-board

/* Sub-board UART5 RS485 transceiver */
#define APP_SUBBOARD_RS485_AUTO_DIR     1U              // 1: auto-direction module, 0: GPIO direction control

/* UART receive and transmit buffer sizes */
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

#define CAN1_RX_TEXT_DEBUG              0U          // can1 debug flag  using text
#define COMM_PARSE_BUDGET_BYTES         64U         // Single-session communication analysis bytes

/* CAN and device IDs */
#define CAN2_MAX_DLC                    8U
#define WIT_IMU_CAN_ADDR                0x50U

/* Remote controller thresholds */
#define APP_RMT_TEXT_DEBUG              0U          // remote debug flag  using text
#define RMT_SWITCH_LOW_THRESHOLD        1300U
#define RMT_SWITCH_HIGH_THRESHOLD       1700U
#define RMT_ANALOG_DEADBAND             1.0f        // Analogue deadband

/* Remote controller calibration */
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
#define YUSHU_GEAR_IN_BACK              -50.0f
#define YUSHU_GEAR_IN_MID               0.0f
#define YUSHU_GEAR_IN_FORWARD           50.0f

#define YUSHU_SPEED_OUT_BACK            -20.0f
#define YUSHU_SPEED_OUT_MID             0.0f
#define YUSHU_SPEED_OUT_FORWARD         60.0f

#define YUSHU_M8010_USE_CAN             0U          // flag -> use can1 or not

#define YUSHU_M8010_ID                  1U
#define YUSHU_M8010_CAN_TX_ID           0x01U
#define YUSHU_M8010_MODE_STOP           0U
#define YUSHU_M8010_MODE_RUN            10U

#define YUSHU_RMT_GEAR_STOP_DEADBAND    1.0f
#define YUSHU_SPEED_STOP_EPS            0.01f       // Speed stop threshold
#define YUSHU_LENGTH_DEFAULT_MAX_SPEED  20.0f
#define YUSHU_LENGTH_MIN_MM             0L
#define YUSHU_LENGTH_MAX_MM             2400L
#define YUSHU_LENGTH_TOL_MM             5L
#define YUSHU_LENGTH_PID_DT_S           0.03f
#define YUSHU_LENGTH_PID_KP             0.20f
#define YUSHU_LENGTH_PID_KI             0.00f
#define YUSHU_LENGTH_PID_KD             0.01f
#define YUSHU_LENGTH_PID_I_LIMIT        500.0f

#define STEER_Y1_IN_LEFT                -50.0f
#define STEER_Y1_IN_MID                 0.0f
#define STEER_Y1_IN_RIGHT               50.0f

#define STEER_SPEED_OUT_LEFT            -1000.0f
#define STEER_SPEED_OUT_MID             0.0f
#define STEER_SPEED_OUT_RIGHT           1000.0f


#define PTZ_UP_DOWN_SPEED_DEFAULT       0x20U
#define PTZ_LEFT_RIGHT_SPEED_DEFAULT    0x10U

#define PTZ_ANGLE_DEFAULT_TOL_X100      100U        // Default angular tolerance
#define PTZ_ANGLE_DEFAULT_SPEED         0x20U

#define PTZ_ANGLE_PID_DT_S              0.05f       // Angle PID cycle
#define PTZ_ANGLE_PID_KP                0.020f
#define PTZ_ANGLE_PID_KI                0.000f
#define PTZ_ANGLE_PID_KD                0.002f
#define PTZ_ANGLE_PID_I_LIMIT           2000.0f
#define PTZ_ANGLE_PID_MIN_SPEED         0x08U

/* Upper-computer automatic arm workflow */
#define ARM_AUTO_YUSHU_TOL_MM           20L
#define ARM_AUTO_ENCODER_GRACE_MS       500U
#define ARM_AUTO_ARM_MIN_SETTLE_MS      1000U
#define ARM_AUTO_ARM_TIMEOUT_MS         12000U
#define ARM_AUTO_END_TIMEOUT_MS         20000U
#define ARM_AUTO_RETRACT_TIMEOUT_MS     12000U
#define ARM_AUTO_SERVO_RUN_TIME_MS      1000U
#define ARM_AUTO_SERVO_SPEED            600U
#define ARM_AUTO_END_FORWARD_MS         3000U
#define ARM_AUTO_END_REVERSE_MS         3000U
#define ARM_AUTO_END_ROTATE_SPEED       600
#define ARM_AUTO_RETRACT_SERVO0_POS     0
#define ARM_AUTO_RETRACT_SERVO1_POS     0
#define ARM_AUTO_RETRACT_SERVO2_POS     0
#define ARM_AUTO_RETRACT_SERVO_TIME_MS  1000U
#define ARM_AUTO_RETRACT_SERVO_SPEED    600U

#define SYS_UART_TX_TIMEOUT             0x0000FFFFUL    // Serial transmission timeout loop counter
#define ENCODER_CAN_WAIT_TIMEOUT        0x0000FFFFUL    // Encoder CAN wait timeout counter

#endif /* __APP_CONFIG_H */

/******************* (C) COPYRIGHT 2026 END OF FILE *****************************/
