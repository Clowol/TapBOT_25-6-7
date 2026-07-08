/******************** (C) COPYRIGHT 2026 *****************************************
  * @file        control_dispatcher.h
  * @brief       Selects active control source and publishes unified commands.
  * @note        控制调度器
  * @warning     
  * @license     This project is released under the MIT License.
 *********************************************************************************/
#ifndef __CONTROL_DISPATCHER_H
#define __CONTROL_DISPATCHER_H

#include "stm32f10x.h"

/*System parameters*/
#define CTRL_STEER_NUM                 4U
#define CTRL_UPPER_TIMEOUT_TICKS_10MS  50U

/*Host computer commands*/
#define UPPER_CMD_SET_SOURCE           0x01U	// Set control source
#define UPPER_CMD_SET_ALL              0x02U

#define UPPER_CMD_YUSHU_SPEED          0x10U
#define UPPER_CMD_EXEC_YUSHU_CAN       0x11U
#define UPPER_CMD_YUSHU_LENGTH         0x12U

#define UPPER_CMD_STEER_SYNC           0x20U	// Synchronise all servos
#define UPPER_CMD_EXEC_STEER_SPEED     0x21U
#define UPPER_CMD_EXEC_STEER_POSITION  0x22U
#define UPPER_CMD_EXEC_STEER_MODE      0x23U	// Set the servo operating mode

#define UPPER_CMD_PTZ                  0x30U
#define UPPER_CMD_EXEC_PTZ             0x31U
#define UPPER_CMD_EXEC_PTZ_ANGLE       0x32U

#define UPPER_CMD_ARM_AUTO_START       0x40U
#define UPPER_CMD_ARM_AUTO_STOP        0x41U

#define UPPER_CMD_HEARTBEAT            0x7FU


typedef enum
{
	CTRL_SRC_RMT = 0,
	CTRL_SRC_UPPER
} ctrl_source_t;

/* Control variable */
typedef struct
{
	float yushu_gear_cmd;
	s32 yushu_target_length_mm;
	float yushu_length_max_speed;
	float steer_axis_cmd;
	u8 steer_enable[CTRL_STEER_NUM];
	u8 ptz_up_down_cmd;
	u8 ptz_left_right_cmd;
	u8 ptz_up_down_speed;
	u8 ptz_left_right_speed;
	u8 clutch_cmd;
	u8 yushu_length_ctrl_enable;
	u8 valid;
} ctrl_cmd_t;


extern volatile ctrl_source_t g_ctrl_source_req;		// Source of the request
extern volatile ctrl_source_t g_ctrl_source_active;		// Effective control sources
extern ctrl_cmd_t g_ctrl_cmd;	// Control orders currently in force


/*=========================  Function ==============================================*/
void ControlDispatcher_Init(void);
/* 10ms update control dispatcher*/
void ControlDispatcher_Update(void);
void ControlDispatcher_SetUpperCmd(const ctrl_cmd_t *cmd);
void ControlDispatcher_OnUpperPacketReceived(u8 cmd_id, const u8 *payload, u8 len);

void ControlDispatcher_NotifyUpperAlive(void);
u8 ControlDispatcher_IsUpperAlive(void);

/* Retrieve the currently active control command */
const ctrl_cmd_t *ControlDispatcher_GetCmd(void);

#endif


/******************* (C) COPYRIGHT 2026 END OF FILE  *********************************************************************************/
