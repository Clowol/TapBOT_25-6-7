/******************** (C) COPYRIGHT 2026 *****************************************
   * @author      Clomol
   * @date        2026-2027
   * @brief       控制调度器实现，负责遥控器/上位机控制源切换、命令解析与执行
   * @license     [z]本代码仅用于教学与科研目的，未经作者书面许可，不得用于商业用途
   *              This project is released under the MIT License.
   * @note        
   * @warning     上位机超时时间需与心跳包发送频率匹配
*********************************************************************************/
#include "control_dispatcher.h"

#include "rmt_data.h"
#include "ptz_data.h"


/************************************** 全局变量定义 ***************************************/
volatile ctrl_source_t g_ctrl_source_req = CTRL_SRC_RMT;
volatile ctrl_source_t g_ctrl_source_active = CTRL_SRC_RMT;
ctrl_cmd_t g_ctrl_cmd;

static ctrl_cmd_t UpperCmd;
static volatile u16 UpperTimeoutTicks = CTRL_UPPER_TIMEOUT_TICKS_10MS;
static volatile u8 UpperCmdValid = 0U;



/**************************************** 函数定义 ****************************************/
/**
 * @brief 从字节数组中读取小端16位有符号整数
 * @param data 指向至少2字节的数组
 * @return 解析出的有符号16位整数值
 */
static s16 ControlDispatcher_ReadS16LE(const u8 *data)
{
	return (s16)((u16)data[0] | ((u16)data[1] << 8));
}


/**
 * @brief 从字节数组中读取小端16位无符号整数
 * @param data 指向至少2字节的数组
 * @return 解析出的无符号16位整数值
 */
static u16 ControlDispatcher_ReadU16LE(const u8 *data)
{
	return (u16)((u16)data[0] | ((u16)data[1] << 8));
}


/**
 * @brief 检查当前是否允许执行上位机命令
 * @return 1:允许，0:不允许
 * @note 条件：遥控器与上位机切换标志 g_RmtUpManCtrlMode 为 UP_MODE（即允许上位机介入）
 */
static u8 ControlDispatcher_CanExecUpper(void)
{
	return (g_RmtUpManCtrlMode == UP_MODE) ? 1U : 0U;
}


/**
 * @brief 清除控制命令结构体中的所有字段
 * @param cmd 指向待清除的命令结构体（不能为NULL）
 * @note 将各个字段设为安全默认值，valid置0
 */
static void ControlDispatcher_ClearCmd(ctrl_cmd_t *cmd)
{
	u8 i;

	if(cmd == 0)
	{
		return;
	}

	cmd->yushu_gear_cmd = 0.0f;
	cmd->steer_axis_cmd = 0.0f;
	for(i = 0U; i < CTRL_STEER_NUM; i++)
	{
		cmd->steer_enable[i] = 0U;
	}
	cmd->ptz_up_down_cmd = PTZ_STOP;
	cmd->ptz_left_right_cmd = PTZ_STOP;
	cmd->ptz_up_down_speed = 0U;
	cmd->ptz_left_right_speed = 0U;
	cmd->clutch_cmd = 0xFFU;
	cmd->valid = 0U;
}


/**
 * @brief 从遥控器全局变量加载控制命令
 * @param cmd 输出参数，填充后的命令结构体
 * @note 读取 RmtGearValue、Rmt_Y1_Value 及遥控器通道状态，并处理遥控器断电/通信异常时的离合器保护
 */
static void ControlDispatcher_LoadRemoteCmd(ctrl_cmd_t *cmd)
{
	ControlDispatcher_ClearCmd(cmd);

    /* 遥控器断电或通信异常时，强制离合器断开 */
	if((RmtPwrOffFlg != 0U) || (RmtOutCommunFlg != 0U))
	{
		cmd->clutch_cmd = 0U;
		cmd->valid = 1U;
		return;
	}

	cmd->yushu_gear_cmd = RmtGearValue;
	cmd->steer_axis_cmd = Rmt_Y1_Value;
	cmd->steer_enable[0] = (Rmtrece_Msg.CH_A >= RMT_SWITCH_HIGH_THRESHOLD) ? 1U : 0U;
	cmd->steer_enable[1] = (Rmtrece_Msg.CH_B >= RMT_SWITCH_HIGH_THRESHOLD) ? 1U : 0U;
	cmd->steer_enable[2] = (Rmtrece_Msg.CH_C >= RMT_SWITCH_HIGH_THRESHOLD) ? 1U : 0U;
	cmd->steer_enable[3] = (Rmtrece_Msg.CH_D >= RMT_SWITCH_HIGH_THRESHOLD) ? 1U : 0U;
	cmd->ptz_up_down_cmd = PTZ_UpDownMoveFlg;
	cmd->ptz_left_right_cmd = PTZ_LftRgtMoveFlg;
	cmd->ptz_up_down_speed = (PTZ_UpDownMoveFlg == PTZ_STOP) ? 0U : PTZ_UP_DOWN_SPEED_DEFAULT;
	cmd->ptz_left_right_speed = (PTZ_LftRgtMoveFlg == PTZ_STOP) ? 0U : PTZ_LEFT_RIGHT_SPEED_DEFAULT;
	cmd->clutch_cmd = RmtClutchStartStop;
	cmd->valid = 1U;
}




/**************************** 上位机命令执行函数 *****************************/
/**
 * @brief 执行玉树CAN控制命令（UPPER_CMD_EXEC_YUSHU_CAN）
 * @param payload 数据负载指针，格式：payload[0]=cmd, [1]=mode, [2-3]=W, [4-5]=T, [6-7]=K_W
 * @param len     负载长度（至少8字节）
 * @note 解析后调用 YushuMotor_SendControl() 发送给玉树电机，并更新心跳
 */
static void ControlDispatcher_ExecYushuCan(const u8 *payload, u8 len)
{
	if((payload == 0) || (len < 8U) || (ControlDispatcher_CanExecUpper() == 0U))
	{
		return;
	}

	cmd.id = payload[0];
	cmd.mode = payload[1];
	cmd.W = ((float)ControlDispatcher_ReadS16LE(&payload[2])) * 0.1f;
	cmd.T = ((float)ControlDispatcher_ReadS16LE(&payload[4])) * 0.01f;
	cmd.K_W = ((float)ControlDispatcher_ReadS16LE(&payload[6])) * 0.001f;
	YushuMotor_SendControl();
	ControlDispatcher_NotifyUpperAlive();
}


/**
 * @brief 执行舵机速度控制命令（UPPER_CMD_EXEC_STEER_SPEED）
 * @param payload 数据负载指针，格式：payload[0]=使能掩码, 之后每2字节一个速度值（按舵机顺序）
 * @param len     负载长度（至少9字节，支持4个舵机）
 * @note 将速度值填入 SteerSendMsgArr，并调用同步速度运行函数
 */
static void ControlDispatcher_ExecSteerSpeed(const u8 *payload, u8 len)
{
	u8 i;
	u8 enable_mask;

	if((payload == 0) || (len < 9U) || (ControlDispatcher_CanExecUpper() == 0U))
	{
		return;
	}

	enable_mask = payload[0];
	if(SteerRunMode != STEER_SPEED_MODE)
	{
		SteerRunMode = STEER_SPEED_MODE;
		SendSteer_SYNC_SetMode(STEER_SPEED_MODE);
	}

	for(i = 0U; i < CTRL_STEER_NUM; i++)
	{
		if((enable_mask & (1U << i)) != 0U)
		{
			SteerSendMsgArr[i].SpeedData = ControlDispatcher_ReadS16LE(&payload[1U + (i * 2U)]);
		}
		else
		{
			SteerSendMsgArr[i].SpeedData = 0;
		}
	}

	SendSteer_SYNC_SpeedRun();
	ControlDispatcher_NotifyUpperAlive();
}


/**
 * @brief 执行舵机位置控制命令（UPPER_CMD_EXEC_STEER_POSITION）
 * @param payload 数据负载指针，格式：payload[0]=使能掩码, 之后每2字节一个位置值（按舵机顺序），
 *                最后4字节：运行时间（2字节）和速度（2字节）
 * @param len     负载长度（至少13字节，支持4个舵机）
 */
static void ControlDispatcher_ExecSteerPosition(const u8 *payload, u8 len)
{
	u8 i;
	u8 enable_mask;
	u16 run_time;
	u16 speed;

	if((payload == 0) || (len < 13U) || (ControlDispatcher_CanExecUpper() == 0U))
	{
		return;
	}

	enable_mask = payload[0];
	run_time = ControlDispatcher_ReadU16LE(&payload[9]);
	speed = ControlDispatcher_ReadU16LE(&payload[11]);

	if(SteerRunMode != STEER_POSITION_MODE)
	{
		SteerRunMode = STEER_POSITION_MODE;
		SendSteer_SYNC_SetMode(STEER_POSITION_MODE);
	}

	for(i = 0U; i < CTRL_STEER_NUM; i++)
	{
		if((enable_mask & (1U << i)) != 0U)
		{
			SteerSendMsgArr[i].PosData = ControlDispatcher_ReadS16LE(&payload[1U + (i * 2U)]);
			SteerSendMsgArr[i].RunTime = run_time;
			SteerSendMsgArr[i].SpeedData = (s16)speed;
		}
		else
		{
			SteerSendMsgArr[i].RunTime = 0U;
			SteerSendMsgArr[i].SpeedData = 0;
		}
	}

	SendSteer_SYNC_DataFun();
	ControlDispatcher_NotifyUpperAlive();
}


/**
 * @brief 执行舵机模式设置命令（UPPER_CMD_EXEC_STEER_MODE）
 * @param payload 数据负载指针，payload[0]=模式（STEER_SPEED_MODE 或 STEER_POSITION_MODE）
 * @param len     负载长度（至少1字节）
 */
static void ControlDispatcher_ExecSteerMode(const u8 *payload, u8 len)
{
	if((payload == 0) || (len < 1U) || (ControlDispatcher_CanExecUpper() == 0U))
	{
		return;
	}

	if((payload[0] == STEER_SPEED_MODE) || (payload[0] == STEER_POSITION_MODE))
	{
		SteerRunMode = payload[0];
		SendSteer_SYNC_SetMode(SteerRunMode);
		ControlDispatcher_NotifyUpperAlive();
	}
}

/**
 * @brief 执行云台控制命令（UPPER_CMD_EXEC_PTZ）
 * @param payload 数据负载指针，格式：payload[0]=上下命令, [1]=左右命令, [2]=上下速度, [3]=左右速度
 * @param len     负载长度（至少4字节）
 * @note 设置全局云台控制变量并发送
 */
static void ControlDispatcher_ExecPtz(const u8 *payload, u8 len)
{
	if((payload == 0) || (len < 4U) || (ControlDispatcher_CanExecUpper() == 0U))
	{
		return;
	}

	PTZ_UpDownMoveFlg = payload[0];
	PTZ_LftRgtMoveFlg = payload[1];
	PTZ_SendMsg_Cmd.Up_Down_Speed = payload[2];
	PTZ_SendMsg_Cmd.Lft_Rgt_Speed = payload[3];
	Send_PTZ_Data();
	ControlDispatcher_NotifyUpperAlive();
}




/*********************************   调度器   *********************************************/ 
/**
 * @brief 控制调度器初始化
 * @note 清除所有命令缓冲区，复位控制源为遥控器，重置上位机超时计数
 */
void ControlDispatcher_Init(void)
{
	ControlDispatcher_ClearCmd(&g_ctrl_cmd);
	ControlDispatcher_ClearCmd(&UpperCmd);
	g_ctrl_source_req = CTRL_SRC_RMT;
	g_ctrl_source_active = CTRL_SRC_RMT;
	UpperTimeoutTicks = CTRL_UPPER_TIMEOUT_TICKS_10MS;
	UpperCmdValid = 0U;
}


/**
 * @brief 控制调度器周期性更新函数（通常放在主循环中）
 * @note 负责上位机超时计数、控制源选择、命令装载
 *       当允许上位机控制、上位机命令有效且上位机在线时，切换到上位机模式；
 *       否则使用遥控器模式
 */
void ControlDispatcher_Update(void)
{
	if(UpperTimeoutTicks < CTRL_UPPER_TIMEOUT_TICKS_10MS)
	{
		UpperTimeoutTicks++;
	}

    /* 判断是否应使用上位机命令 */
	if((g_RmtUpManCtrlMode == UP_MODE) && (UpperCmdValid != 0U) && (ControlDispatcher_IsUpperAlive() != 0U))
	{
		g_ctrl_source_req = CTRL_SRC_UPPER;
		g_ctrl_source_active = CTRL_SRC_UPPER;
		g_RmtUpManRealMode = UP_MODE;
		g_ctrl_cmd = UpperCmd;
		g_ctrl_cmd.valid = 1U;
	}
	else
	{
		g_ctrl_source_req = CTRL_SRC_RMT;
		g_ctrl_source_active = CTRL_SRC_RMT;
		g_RmtUpManRealMode = RMT_MODE;
		ControlDispatcher_LoadRemoteCmd(&g_ctrl_cmd);
	}
}


/**
 * @brief 设置上位机命令（立即生效）
 * @param cmd 指向包含完整控制信息的命令结构体
 * @note 会拷贝命令内容，并更新有效标志和心跳
 */
void ControlDispatcher_SetUpperCmd(const ctrl_cmd_t *cmd)
{
	if(cmd == 0)
	{
		return;
	}

	UpperCmd = *cmd;
	UpperCmd.valid = 1U;
	UpperCmdValid = 1U;
	ControlDispatcher_NotifyUpperAlive();
}

/**
 * @brief 上位机数据包接收回调，根据命令ID分发处理
 * @param cmd_id  命令ID（见 UPPER_CMD_* 宏）
 * @param payload 数据负载指针
 * @param len     数据长度
 * @note 该函数在解析到完整上位机数据包时调用，内部会调用相应的执行函数或更新命令缓存
 */
void ControlDispatcher_OnUpperPacketReceived(u8 cmd_id, const u8 *payload, u8 len)
{
	u8 i;

	if(payload == 0)
	{
		return;
	}

	switch(cmd_id)
	{
        /* 设置控制源（遥控器/上位机） */
		case UPPER_CMD_SET_SOURCE:
			if(len >= 1U)
			{
				g_RmtUpManCtrlMode = (payload[0] == CTRL_SRC_UPPER) ? UP_MODE : RMT_MODE;
				ControlDispatcher_NotifyUpperAlive();
			}
			break;

        /* 设置全部控制参数 */
		case UPPER_CMD_SET_ALL:
			if(len >= 9U)
			{
				UpperCmd.yushu_gear_cmd = ((float)ControlDispatcher_ReadS16LE(&payload[0])) * 0.1f;
				UpperCmd.steer_axis_cmd = ((float)ControlDispatcher_ReadS16LE(&payload[2])) * 0.1f;
				for(i = 0U; i < CTRL_STEER_NUM; i++)
				{
					UpperCmd.steer_enable[i] = ((payload[4] & (1U << i)) != 0U) ? 1U : 0U;
				}
				UpperCmd.ptz_up_down_cmd = payload[5];
				UpperCmd.ptz_left_right_cmd = payload[6];
				UpperCmd.ptz_up_down_speed = payload[7];
				UpperCmd.ptz_left_right_speed = payload[8];
				UpperCmd.clutch_cmd = (len >= 10U) ? payload[9] : 0xFFU;
				ControlDispatcher_SetUpperCmd(&UpperCmd);
			}
			break;

		case UPPER_CMD_YUSHU_SPEED:
			if(len >= 2U)
			{
				UpperCmd.yushu_gear_cmd = ((float)ControlDispatcher_ReadS16LE(&payload[0])) * 0.1f;
				if(len >= 3U)
				{
					UpperCmd.clutch_cmd = payload[2];
				}
				ControlDispatcher_SetUpperCmd(&UpperCmd);
			}
			break;

		case UPPER_CMD_EXEC_YUSHU_CAN:
			ControlDispatcher_ExecYushuCan(payload, len);
			break;

		case UPPER_CMD_STEER_SYNC:
			if(len >= 3U)
			{
				UpperCmd.steer_axis_cmd = ((float)ControlDispatcher_ReadS16LE(&payload[0])) * 0.1f;
				for(i = 0U; i < CTRL_STEER_NUM; i++)
				{
					UpperCmd.steer_enable[i] = ((payload[2] & (1U << i)) != 0U) ? 1U : 0U;
				}
				ControlDispatcher_SetUpperCmd(&UpperCmd);
			}
			break;

		case UPPER_CMD_EXEC_STEER_SPEED:
			ControlDispatcher_ExecSteerSpeed(payload, len);
			break;

		case UPPER_CMD_EXEC_STEER_POSITION:
			ControlDispatcher_ExecSteerPosition(payload, len);
			break;

		case UPPER_CMD_EXEC_STEER_MODE:
			ControlDispatcher_ExecSteerMode(payload, len);
			break;

		case UPPER_CMD_PTZ:
			if(len >= 4U)
			{
				UpperCmd.ptz_up_down_cmd = payload[0];
				UpperCmd.ptz_left_right_cmd = payload[1];
				UpperCmd.ptz_up_down_speed = payload[2];
				UpperCmd.ptz_left_right_speed = payload[3];
				ControlDispatcher_SetUpperCmd(&UpperCmd);
			}
			break;

		case UPPER_CMD_EXEC_PTZ:
			ControlDispatcher_ExecPtz(payload, len);
			break;

		case UPPER_CMD_HEARTBEAT:
			ControlDispatcher_NotifyUpperAlive();
			break;

		default:
			break;
	}
}

/**
 * @brief 通知调度器上位机仍然在线（刷新心跳计时器）
 * @note 通常在上位机发送心跳包或有效命令时调用
 */
void ControlDispatcher_NotifyUpperAlive(void)
{
	UpperTimeoutTicks = 0U;
}

/**
 * @brief 查询上位机是否在线（未超时）
 * @return 1:在线，0:离线
 */
u8 ControlDispatcher_IsUpperAlive(void)
{
	return (UpperTimeoutTicks < CTRL_UPPER_TIMEOUT_TICKS_10MS) ? 1U : 0U;
}

/**
 * @brief 获取当前应执行的控制命令
 * @return 指向最终控制命令的常量指针（只读）
 * @note 外部执行模块应调用此函数获取最新有效命令
 */
const ctrl_cmd_t *ControlDispatcher_GetCmd(void)
{
	return &g_ctrl_cmd;
}



/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/
