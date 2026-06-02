/******************** (C) COPYRIGHT 2026 *****************************************
   * @author      Clomol
   * @date        2026-2027
   * @brief       选择控制源并发布统一命令
   * @license     [z]本代码仅用于教学与科研目的，未经作者书面许可，不得用于商业用途
   *              This project is released under the MIT License.
   * @note        
   * @warning     
*********************************************************************************/
#ifndef __CONTROL_DISPATCHER_H
#define __CONTROL_DISPATCHER_H

#include "stm32f10x.h"
#include "function.h"
#include "app_config.h"

/*********************************** 控制参数宏定义 *********************************/
#define CTRL_STEER_NUM                 4U           // 系统中舵机的数量
#define CTRL_UPPER_TIMEOUT_TICKS_10MS  50U          // 上层命令超时阈值（10ms为单位），超过该值认为上层失联

#define UPPER_CMD_SET_SOURCE           0x01U        // 设置控制源
#define UPPER_CMD_SET_ALL              0x02U        // 设置所有控制参数
#define UPPER_CMD_YUSHU_SPEED          0x10U        // 设置宇树电机速度
#define UPPER_CMD_EXEC_YUSHU_CAN       0x11U        // 宇树电机CAN命令
#define UPPER_CMD_STEER_SYNC           0x20U        // 舵机同步
#define UPPER_CMD_EXEC_STEER_SPEED     0x21U        // 舵机速度命令
#define UPPER_CMD_EXEC_STEER_POSITION  0x22U        // 舵机位置命令
#define UPPER_CMD_EXEC_STEER_MODE      0x23U        // 舵机模式命令
#define UPPER_CMD_PTZ                  0x30U        // 云台控制
#define UPPER_CMD_EXEC_PTZ             0x31U        // 执行云台命令
#define UPPER_CMD_HEARTBEAT            0x7FU        // 心跳命令

/**
 * @brief 控制源选择枚举
 */
typedef enum
{
	CTRL_SRC_RMT = 0,
	CTRL_SRC_UPPER
} ctrl_source_t;


/**
 * @brief 控制命令结构体，用于传递所有控制指令
 */
typedef struct
{
	float yushu_gear_cmd;
	float steer_axis_cmd;
	u8 steer_enable[CTRL_STEER_NUM];
	u8 ptz_up_down_cmd;
	u8 ptz_left_right_cmd;
	u8 ptz_up_down_speed;
	u8 ptz_left_right_speed;
	u8 clutch_cmd;
	u8 valid;                           // 命令有效标志，1表示命令有效，0表示无效或未更新
} ctrl_cmd_t;


/************************************外部全局变量声明 ****************************/
extern volatile ctrl_source_t g_ctrl_source_req;        
extern volatile ctrl_source_t g_ctrl_source_active;
extern ctrl_cmd_t g_ctrl_cmd;


/*==================== 函数接口声明 ====================*/

void ControlDispatcher_Init(void);

void ControlDispatcher_Update(void);

void ControlDispatcher_SetUpperCmd(const ctrl_cmd_t *cmd);

void ControlDispatcher_OnUpperPacketReceived(u8 cmd_id, const u8 *payload, u8 len);

void ControlDispatcher_NotifyUpperAlive(void);

u8 ControlDispatcher_IsUpperAlive(void);

const ctrl_cmd_t *ControlDispatcher_GetCmd(void);


#endif /* __CONTROL_DISPATCHER_H */

/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/
