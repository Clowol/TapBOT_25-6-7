/******************** (C) COPYRIGHT 2026 *****************************************
  * @file        control_dispatcher.c
  * @brief       Runtime control-source allocator for remote and upper computer.
  * @note        
  * @warning     
  * @license     This project is released under the MIT License.
 *********************************************************************************/
#include "control_dispatcher.h"
#include "function.h"
#include "rmt_data.h"
#include "ptz_data.h"
#include "app_config.h"
#include "arm_auto_task.h"


/*====================  Static global variables ===========================*/

volatile ctrl_source_t g_ctrl_source_req = CTRL_SRC_RMT;
volatile ctrl_source_t g_ctrl_source_active = CTRL_SRC_RMT;
ctrl_cmd_t g_ctrl_cmd;


static ctrl_cmd_t UpperCmd;
static volatile u16 UpperTimeoutTicks = CTRL_UPPER_TIMEOUT_TICKS_10MS;
static volatile u8 UpperCmdValid = 0U;


/*======================= Auxiliary static functions =======================*/

static s16 ControlDispatcher_ReadS16LE(const u8 *data)
{
	return (s16)((u16)data[0] | ((u16)data[1] << 8));
}

static u16 ControlDispatcher_ReadU16LE(const u8 *data)
{
	return (u16)((u16)data[0] | ((u16)data[1] << 8));
}

static s32 ControlDispatcher_ReadS32LE(const u8 *data)
{
	return (s32)((u32)data[0] |
					((u32)data[1] << 8) |
					((u32)data[2] << 16) |
					((u32)data[3] << 24));
}


/* Determine whether the system permits control from a host computer */
static u8 ControlDispatcher_CanExecUpper(void)
{
	return (g_RmtUpManCtrlMode == UP_MODE) ? 1U : 0U;
}

/* Clear the command structure */
static void ControlDispatcher_ClearCmd(ctrl_cmd_t *cmd)
{
	u8 i;

	if(cmd == 0)
	{
		return;
	}

	cmd->yushu_gear_cmd = 0.0f;
	cmd->yushu_target_length_mm = 0;
	cmd->yushu_length_max_speed = YUSHU_LENGTH_DEFAULT_MAX_SPEED;
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
	cmd->yushu_length_ctrl_enable = 0U;
	cmd->valid = 0U;
}

/* Control commands in remote control mode */
static void ControlDispatcher_LoadRemoteCmd(ctrl_cmd_t *cmd)
{
	static u8 rmt_safe_stop_sent = 0U;

	ControlDispatcher_ClearCmd(cmd);

	if((RmtPwrOffFlg != 0U) || (RmtOutCommunFlg != 0U))
	{
		if(rmt_safe_stop_sent == 0U)
		{
			PTZ_DisableAngleCtrl();
			PTZ_Stop();
			rmt_safe_stop_sent = 1U;
		}
		cmd->clutch_cmd = 0U;
		cmd->valid = 1U;
		return;
	}
	rmt_safe_stop_sent = 0U;

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

// control steer.no3 speed
static void ControlDispatcher_ExecSteerSpeed(const u8 *payload, u8 len)
{
	s16 rotate_speed;

	if((payload == 0) || (ControlDispatcher_CanExecUpper() == 0U))
	{
		return;
	}

	if(len >= 9U)
	{
		if((payload[0] & (1U << STEER_ROTATE_SERVO_INDEX)) != 0U)
		{
			rotate_speed = ControlDispatcher_ReadS16LE(&payload[1U + (STEER_ROTATE_SERVO_INDEX * 2U)]);
		}
		else
		{
			rotate_speed = 0;
		}
	}
	else if(len >= 3U)
	{
		rotate_speed = (payload[0] != 0U) ? ControlDispatcher_ReadS16LE(&payload[1]) : 0;
	}
	else
	{
		return;
	}

	if(SteerRunMode != STEER_MIXED_MODE)
	{
		SteerRunMode = STEER_MIXED_MODE;
		SendSteer_SYNC_SetDefaultMode();
	}

	// send msg
	SteerSendMsgArr[STEER_ROTATE_SERVO_INDEX].SpeedData = rotate_speed;
	SendSteer_Rotate_SpeedRun();
	ControlDispatcher_NotifyUpperAlive();
}


static void ControlDispatcher_ExecSteerPosition(const u8 *payload, u8 len)
{
	u8 i;
	u8 enable_mask;
	u16 run_time;
	u16 speed;
	u8 time_offset;		// offset

	if((payload == 0) || (ControlDispatcher_CanExecUpper() == 0U))
	{
		return;
	}

	// [time_offset,time_offset+1] -> runtime
	// [time_offset+2,time_offset+3] -> speed
	if(len >= 13U)
	{
		time_offset = 9U;
	}
	else if(len >= 11U)
	{
		time_offset = 7U;
	}
	else
	{
		return;
	}

	enable_mask = payload[0];
	run_time = ControlDispatcher_ReadU16LE(&payload[time_offset]);
	speed = ControlDispatcher_ReadU16LE(&payload[time_offset + 2U]);

	if(SteerRunMode != STEER_MIXED_MODE)
	{
		SteerRunMode = STEER_MIXED_MODE;
		SendSteer_SYNC_SetDefaultMode();
	}

	for(i = 0U; i < STEER_POSITION_SERVO_NUM; i++)
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

	SendSteer_Position3_SYNC_DataFun();
	ControlDispatcher_NotifyUpperAlive();
}


// reset steer normal mode
static void ControlDispatcher_ExecSteerMode(const u8 *payload, u8 len)
{
	if((payload == 0) || (len < 1U) || (ControlDispatcher_CanExecUpper() == 0U))
	{
		return;
	}

	SendSteer_SYNC_SetDefaultMode();
	ControlDispatcher_NotifyUpperAlive();
}


static void ControlDispatcher_ExecPtz(const u8 *payload, u8 len)
{
	if((payload == 0) || (len < 4U) || (ControlDispatcher_CanExecUpper() == 0U))
	{
		return;
	}

	PTZ_DisableAngleCtrl();
	PTZ_UpDownMoveFlg = payload[0];
	PTZ_LftRgtMoveFlg = payload[1];
	PTZ_SetUDSpeed(payload[2]);
	PTZ_SetLRSpeed(payload[3]);
	Send_PTZ_Data();
	ControlDispatcher_NotifyUpperAlive();
}

static void ControlDispatcher_ExecPtzAngle(const u8 *payload, u8 len)
{
	s16 yaw_target;
	s16 pitch_target;
	u16 tolerance;
	u8 speed;

	if((payload == 0) || (len < 7U) || (ControlDispatcher_CanExecUpper() == 0U))
	{
		return;
	}

	yaw_target = ControlDispatcher_ReadS16LE(&payload[0]);
	pitch_target = ControlDispatcher_ReadS16LE(&payload[2]);
	tolerance = ControlDispatcher_ReadU16LE(&payload[4]);
	speed = payload[6];
	PTZ_SetAngleTarget(yaw_target, pitch_target, tolerance, speed);
	ControlDispatcher_NotifyUpperAlive();
}

/*========================== External interface functions ============================*/
/* Initialise the scheduler module */
void ControlDispatcher_Init(void)
{
	ControlDispatcher_ClearCmd(&g_ctrl_cmd);
	ControlDispatcher_ClearCmd(&UpperCmd);
	g_ctrl_source_req = CTRL_SRC_RMT;
	g_ctrl_source_active = CTRL_SRC_RMT;
	UpperTimeoutTicks = CTRL_UPPER_TIMEOUT_TICKS_10MS;
	UpperCmdValid = 0U;
}

/* 10ms  ->  Control Source Arbitration and Command Updates */
void ControlDispatcher_Update(void)
{
	ctrl_source_t prev_source = g_ctrl_source_active;

	if(UpperTimeoutTicks < CTRL_UPPER_TIMEOUT_TICKS_10MS)
	{
		UpperTimeoutTicks++;
	}

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

	if((prev_source == CTRL_SRC_UPPER) && (g_ctrl_source_active == CTRL_SRC_RMT))
	{
		PTZ_DisableAngleCtrl();
		PTZ_Stop();
	}
}

/* Host computer settings: complete control commands */
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

/* Processing individual commands sent by the host computer */
void ControlDispatcher_OnUpperPacketReceived(u8 cmd_id, const u8 *payload, u8 len)
{
	u8 i;

	if((payload == 0) && (len > 0U))
	{
		return;
	}

	switch(cmd_id)
	{
		case UPPER_CMD_SET_SOURCE:
			if(len >= 1U)
			{
				g_RmtUpManCtrlMode = (payload[0] == CTRL_SRC_UPPER) ? UP_MODE : RMT_MODE;
				ControlDispatcher_NotifyUpperAlive();
			}
			break;

		case UPPER_CMD_SET_ALL:
			if(len >= 9U)
			{
				UpperCmd.yushu_gear_cmd = ((float)ControlDispatcher_ReadS16LE(&payload[0])) * 0.1f;
				UpperCmd.yushu_length_ctrl_enable = 0U;
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
				UpperCmd.yushu_length_ctrl_enable = 0U;
				if(len >= 3U)
				{
					UpperCmd.clutch_cmd = payload[2];
				}
				ControlDispatcher_SetUpperCmd(&UpperCmd);
			}
			break;

		case UPPER_CMD_YUSHU_LENGTH:
			if(len >= 4U)
			{
				UpperCmd.yushu_target_length_mm = ControlDispatcher_ReadS32LE(&payload[0]);
				
				if(UpperCmd.yushu_target_length_mm < YUSHU_LENGTH_MIN_MM)
				{
					UpperCmd.yushu_target_length_mm = YUSHU_LENGTH_MIN_MM;
				}
				else if(UpperCmd.yushu_target_length_mm > YUSHU_LENGTH_MAX_MM)
				{
					UpperCmd.yushu_target_length_mm = YUSHU_LENGTH_MAX_MM;
				}

				UpperCmd.yushu_length_max_speed = (len >= 6U) ?
					(((float)ControlDispatcher_ReadU16LE(&payload[4])) * 0.1f) :
					YUSHU_LENGTH_DEFAULT_MAX_SPEED;

				if(UpperCmd.yushu_length_max_speed <= 0.0f)
				{
					UpperCmd.yushu_length_max_speed = YUSHU_LENGTH_DEFAULT_MAX_SPEED;
				}

				UpperCmd.yushu_length_ctrl_enable = 1U;
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
				PTZ_DisableAngleCtrl();
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

		case UPPER_CMD_EXEC_PTZ_ANGLE:
			ControlDispatcher_ExecPtzAngle(payload, len);
			break;

		case UPPER_CMD_ARM_AUTO_START:
			if(ControlDispatcher_CanExecUpper() != 0U)
			{
				(void)ArmAutoTask_Start(payload, len);
				ControlDispatcher_NotifyUpperAlive();
			}
			break;

		case UPPER_CMD_ARM_AUTO_STOP:
			if(ControlDispatcher_CanExecUpper() != 0U)
			{
				ArmAutoTask_Stop();
				ControlDispatcher_NotifyUpperAlive();
			}
			break;

		case UPPER_CMD_HEARTBEAT:
			ControlDispatcher_NotifyUpperAlive();
			break;

		default:
			break;
	}
}



void ControlDispatcher_NotifyUpperAlive(void)
{
	UpperTimeoutTicks = 0U;
}



u8 ControlDispatcher_IsUpperAlive(void)
{
	return (UpperTimeoutTicks < CTRL_UPPER_TIMEOUT_TICKS_10MS) ? 1U : 0U;
}

const ctrl_cmd_t *ControlDispatcher_GetCmd(void)
{
	return &g_ctrl_cmd;
}

/******************* (C) COPYRIGHT 2026 END OF FILE  *********************************************************************************/
