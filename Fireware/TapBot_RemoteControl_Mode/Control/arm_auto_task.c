/******************** (C) COPYRIGHT 2026 *****************************************
 * @file        arm_auto_task.c
 * @brief       Automatic arm workflow driven by the upper computer.
 * @license     This project is released under the MIT License.
 *********************************************************************************/
#include "arm_auto_task.h"

#include "app_config.h"
#include "control_dispatcher.h"
#include "encoder.h"
#include "ptz_data.h"
#include "steer_data.h"
#include "subboard_link.h"
#include "subboard_protocol.h"

typedef struct
{
    u8 flags;
    s16 yaw_deg_x100;
    s16 pitch_deg_x100;
    u16 ptz_tolerance_x100;
    u8 ptz_speed;
    s32 yushu_target_length_mm;
    u16 yushu_max_speed_x10;
    s16 servo_pos[STEER_POSITION_SERVO_NUM];
    u16 servo_run_time_ms;
    u16 servo_speed;
    u16 end_forward_ms;
    u16 end_reverse_ms;
    s16 end_rotate_speed;
    u16 retract_length_mm;
} arm_auto_cfg_t;

static arm_auto_cfg_t s_auto_cfg;
static arm_auto_state_t s_auto_state = ARM_AUTO_STATE_IDLE;
static arm_auto_error_t s_auto_error = ARM_AUTO_ERR_NONE;
static u32 s_state_time_ms = 0U;
static u8 s_end_seen_active = 0U;
static u8 s_retract_sent = 0U;

static s16 ArmAutoTask_ReadS16LE(const u8 *data)
{
    return (s16)((u16)data[0] | ((u16)data[1] << 8));
}

static u16 ArmAutoTask_ReadU16LE(const u8 *data)
{
    return (u16)((u16)data[0] | ((u16)data[1] << 8));
}

static s32 ArmAutoTask_ReadS32LE(const u8 *data)
{
    return (s32)((u32)data[0] |
                 ((u32)data[1] << 8) |
                 ((u32)data[2] << 16) |
                 ((u32)data[3] << 24));
}

static s32 ArmAutoTask_AbsS32(s32 value)
{
    return (value < 0) ? -value : value;
}

static void ArmAutoTask_SetState(arm_auto_state_t state)
{
    s_auto_state = state;
    s_state_time_ms = 0U;
    if(state == ARM_AUTO_STATE_RETRACT)
    {
        s_retract_sent = 0U;
    }
}

static void ArmAutoTask_SetError(arm_auto_error_t error)
{
    s_auto_error = error;
    PTZ_DisableAngleCtrl();
    PTZ_Stop();
    SubBoardProtocol_SendAutoStop();
    ArmAutoTask_SetState(ARM_AUTO_STATE_ERROR);
}

static void ArmAutoTask_ApplyYushuTarget(s32 length_mm, u16 max_speed_x10)
{
    ctrl_cmd_t cmd = *ControlDispatcher_GetCmd();

    if(length_mm < YUSHU_LENGTH_MIN_MM)
    {
        length_mm = YUSHU_LENGTH_MIN_MM;
    }
    else if(length_mm > YUSHU_LENGTH_MAX_MM)
    {
        length_mm = YUSHU_LENGTH_MAX_MM;
    }

    cmd.yushu_target_length_mm = length_mm;
    cmd.yushu_length_max_speed = ((float)max_speed_x10) * 0.1f;
    if(cmd.yushu_length_max_speed <= 0.0f)
    {
        cmd.yushu_length_max_speed = YUSHU_LENGTH_DEFAULT_MAX_SPEED;
    }
    cmd.yushu_length_ctrl_enable = 1U;
    cmd.valid = 1U;
    ControlDispatcher_SetUpperCmd(&cmd);
}

static void ArmAutoTask_SendServo123Position(s16 pos0, s16 pos1, s16 pos2, u16 run_time, u16 speed)
{
    if(SteerRunMode != STEER_MIXED_MODE)
    {
        SendSteer_SYNC_SetDefaultMode();
    }

    SteerSendMsgArr[0].PosData = pos0;
    SteerSendMsgArr[1].PosData = pos1;
    SteerSendMsgArr[2].PosData = pos2;
    SteerSendMsgArr[0].RunTime = run_time;
    SteerSendMsgArr[1].RunTime = run_time;
    SteerSendMsgArr[2].RunTime = run_time;
    SteerSendMsgArr[0].SpeedData = (s16)speed;
    SteerSendMsgArr[1].SpeedData = (s16)speed;
    SteerSendMsgArr[2].SpeedData = (s16)speed;
    SendSteer_Position3_SYNC_DataFun();
}

static u8 ArmAutoTask_YushuAtTarget(s32 target_mm)
{
    const encoder_feedback_t *encoder = Encoder_GetFeedback();

    if(encoder->valid == 0U)
    {
        return 0U;
    }

    return (ArmAutoTask_AbsS32(encoder->length_mm - target_mm) <= ARM_AUTO_YUSHU_TOL_MM) ? 1U : 0U;
}

static void ArmAutoTask_SendMoveCommands(void)
{
    if((s_auto_cfg.flags & ARM_AUTO_FLAG_PTZ) != 0U)
    {
        PTZ_SetAngleTarget(s_auto_cfg.yaw_deg_x100,
                           s_auto_cfg.pitch_deg_x100,
                           s_auto_cfg.ptz_tolerance_x100,
                           s_auto_cfg.ptz_speed);
    }

    if((s_auto_cfg.flags & ARM_AUTO_FLAG_YUSHU) != 0U)
    {
        ArmAutoTask_ApplyYushuTarget(s_auto_cfg.yushu_target_length_mm, s_auto_cfg.yushu_max_speed_x10);
    }

    if((s_auto_cfg.flags & ARM_AUTO_FLAG_SERVO123) != 0U)
    {
        ArmAutoTask_SendServo123Position(s_auto_cfg.servo_pos[0],
                                         s_auto_cfg.servo_pos[1],
                                         s_auto_cfg.servo_pos[2],
                                         s_auto_cfg.servo_run_time_ms,
                                         s_auto_cfg.servo_speed);
    }
}

static void ArmAutoTask_SendRetractCommands(void)
{
    PTZ_DisableAngleCtrl();
    PTZ_Stop();
    SubBoardProtocol_SendAutoStop();

    ArmAutoTask_SendServo123Position(ARM_AUTO_RETRACT_SERVO0_POS,
                                     ARM_AUTO_RETRACT_SERVO1_POS,
                                     ARM_AUTO_RETRACT_SERVO2_POS,
                                     ARM_AUTO_RETRACT_SERVO_TIME_MS,
                                     ARM_AUTO_RETRACT_SERVO_SPEED);

    ArmAutoTask_ApplyYushuTarget((s32)s_auto_cfg.retract_length_mm, s_auto_cfg.yushu_max_speed_x10);
}

void ArmAutoTask_Init(void)
{
    s_auto_state = ARM_AUTO_STATE_IDLE;
    s_auto_error = ARM_AUTO_ERR_NONE;
    s_state_time_ms = 0U;
    s_end_seen_active = 0U;
}

u8 ArmAutoTask_Start(const u8 *payload, u8 len)
{
    if((payload == 0) || (len < 32U))
    {
        s_auto_error = ARM_AUTO_ERR_PARAM;
        return 0U;
    }

    s_auto_cfg.flags = payload[0];
    s_auto_cfg.yaw_deg_x100 = ArmAutoTask_ReadS16LE(&payload[1]);
    s_auto_cfg.pitch_deg_x100 = ArmAutoTask_ReadS16LE(&payload[3]);
    s_auto_cfg.ptz_tolerance_x100 = ArmAutoTask_ReadU16LE(&payload[5]);
    s_auto_cfg.ptz_speed = payload[7];
    s_auto_cfg.yushu_target_length_mm = ArmAutoTask_ReadS32LE(&payload[8]);
    s_auto_cfg.yushu_max_speed_x10 = ArmAutoTask_ReadU16LE(&payload[12]);
    s_auto_cfg.servo_pos[0] = ArmAutoTask_ReadS16LE(&payload[14]);
    s_auto_cfg.servo_pos[1] = ArmAutoTask_ReadS16LE(&payload[16]);
    s_auto_cfg.servo_pos[2] = ArmAutoTask_ReadS16LE(&payload[18]);
    s_auto_cfg.servo_run_time_ms = ArmAutoTask_ReadU16LE(&payload[20]);
    s_auto_cfg.servo_speed = ArmAutoTask_ReadU16LE(&payload[22]);
    s_auto_cfg.end_forward_ms = ArmAutoTask_ReadU16LE(&payload[24]);
    s_auto_cfg.end_reverse_ms = ArmAutoTask_ReadU16LE(&payload[26]);
    s_auto_cfg.end_rotate_speed = ArmAutoTask_ReadS16LE(&payload[28]);
    s_auto_cfg.retract_length_mm = ArmAutoTask_ReadU16LE(&payload[30]);

    if(s_auto_cfg.ptz_tolerance_x100 == 0U)
    {
        s_auto_cfg.ptz_tolerance_x100 = PTZ_ANGLE_DEFAULT_TOL_X100;
    }
    if(s_auto_cfg.ptz_speed == 0U)
    {
        s_auto_cfg.ptz_speed = PTZ_ANGLE_DEFAULT_SPEED;
    }
    if(s_auto_cfg.yushu_max_speed_x10 == 0U)
    {
        s_auto_cfg.yushu_max_speed_x10 = (u16)(YUSHU_LENGTH_DEFAULT_MAX_SPEED * 10.0f);
    }
    if(s_auto_cfg.servo_run_time_ms == 0U)
    {
        s_auto_cfg.servo_run_time_ms = ARM_AUTO_SERVO_RUN_TIME_MS;
    }
    if(s_auto_cfg.servo_speed == 0U)
    {
        s_auto_cfg.servo_speed = ARM_AUTO_SERVO_SPEED;
    }
    if(s_auto_cfg.end_forward_ms == 0U)
    {
        s_auto_cfg.end_forward_ms = ARM_AUTO_END_FORWARD_MS;
    }
    if(s_auto_cfg.end_reverse_ms == 0U)
    {
        s_auto_cfg.end_reverse_ms = ARM_AUTO_END_REVERSE_MS;
    }
    if(s_auto_cfg.end_rotate_speed == 0)
    {
        s_auto_cfg.end_rotate_speed = ARM_AUTO_END_ROTATE_SPEED;
    }

    if(s_auto_cfg.retract_length_mm > (u16)YUSHU_LENGTH_MAX_MM)
    {
        s_auto_cfg.retract_length_mm = (u16)YUSHU_LENGTH_MAX_MM;
    }

    s_auto_error = ARM_AUTO_ERR_NONE;
    s_end_seen_active = 0U;
    s_retract_sent = 0U;
    ArmAutoTask_SetState(ARM_AUTO_STATE_MOVE_TO_TARGET);
    return 1U;
}

void ArmAutoTask_Stop(void)
{
    PTZ_DisableAngleCtrl();
    PTZ_Stop();
    SubBoardProtocol_SendAutoStop();
    s_auto_error = ARM_AUTO_ERR_NONE;
    ArmAutoTask_SetState(ARM_AUTO_STATE_ABORT);
}

void ArmAutoTask_Proc10ms(void)
{
    u8 sub_state;

    if(s_auto_state == ARM_AUTO_STATE_IDLE)
    {
        return;
    }

    if(s_state_time_ms < 0xFFFFFFF0UL)
    {
        s_state_time_ms += 10U;
    }

    switch(s_auto_state)
    {
        case ARM_AUTO_STATE_MOVE_TO_TARGET:
            ArmAutoTask_SendMoveCommands();
            ArmAutoTask_SetState(ARM_AUTO_STATE_WAIT_ARM);
            break;

        case ARM_AUTO_STATE_WAIT_ARM:
            if(((s_auto_cfg.flags & ARM_AUTO_FLAG_YUSHU) != 0U) &&
               (Encoder_GetFeedback()->valid == 0U) &&
               (s_state_time_ms >= ARM_AUTO_ENCODER_GRACE_MS))
            {
                ArmAutoTask_SetError(ARM_AUTO_ERR_ENCODER_INVALID);
                break;
            }

            if(s_state_time_ms >= ARM_AUTO_ARM_TIMEOUT_MS)
            {
                ArmAutoTask_SetError(ARM_AUTO_ERR_ARM_TIMEOUT);
                break;
            }

            if(s_state_time_ms >= ARM_AUTO_ARM_MIN_SETTLE_MS)
            {
                if(((s_auto_cfg.flags & ARM_AUTO_FLAG_YUSHU) == 0U) ||
                   (ArmAutoTask_YushuAtTarget(s_auto_cfg.yushu_target_length_mm) != 0U))
                {
                    if((s_auto_cfg.flags & ARM_AUTO_FLAG_END_AUTO) != 0U)
                    {
                        if(SubBoard_LinkIsOnline() == 0U)
                        {
                            ArmAutoTask_SetError(ARM_AUTO_ERR_SUBBOARD_OFFLINE);
                        }
                        else
                        {
                            SubBoard_LinkSetStatus(SUBBOARD_STATE_UNKNOWN, SUBBOARD_ERR_NONE);
                            SubBoardProtocol_SendAutoStart(s_auto_cfg.end_forward_ms,
                                                           s_auto_cfg.end_reverse_ms,
                                                           s_auto_cfg.end_rotate_speed);
                            ArmAutoTask_SetState(ARM_AUTO_STATE_END_AUTO);
                        }
                    }
                    else if((s_auto_cfg.flags & ARM_AUTO_FLAG_RETRACT) != 0U)
                    {
                        ArmAutoTask_SetState(ARM_AUTO_STATE_RETRACT);
                    }
                    else
                    {
                        ArmAutoTask_SetState(ARM_AUTO_STATE_DONE);
                    }
                }
            }
            break;

        case ARM_AUTO_STATE_END_AUTO:
            sub_state = SubBoard_LinkGetLastState();
            if((sub_state >= SUBBOARD_STATE_WAIT_OBJECT) && (sub_state <= SUBBOARD_STATE_DONE))
            {
                s_end_seen_active = 1U;
            }

            if((sub_state == SUBBOARD_STATE_ERROR) || (sub_state == SUBBOARD_STATE_ESTOP))
            {
                ArmAutoTask_SetError(ARM_AUTO_ERR_SUBBOARD_ERROR);
            }
            else if((sub_state == SUBBOARD_STATE_DONE) ||
                    ((s_end_seen_active != 0U) && (sub_state == SUBBOARD_STATE_IDLE)))
            {
                if((s_auto_cfg.flags & ARM_AUTO_FLAG_RETRACT) != 0U)
                {
                    ArmAutoTask_SetState(ARM_AUTO_STATE_RETRACT);
                }
                else
                {
                    ArmAutoTask_SetState(ARM_AUTO_STATE_DONE);
                }
            }
            else if(s_state_time_ms >= ARM_AUTO_END_TIMEOUT_MS)
            {
                ArmAutoTask_SetError(ARM_AUTO_ERR_END_TIMEOUT);
            }
            break;

        case ARM_AUTO_STATE_RETRACT:
            if(s_retract_sent == 0U)
            {
                ArmAutoTask_SendRetractCommands();
                s_retract_sent = 1U;
            }
            else if(s_state_time_ms >= ARM_AUTO_RETRACT_TIMEOUT_MS)
            {
                ArmAutoTask_SetError(ARM_AUTO_ERR_ARM_TIMEOUT);
            }
            else if(s_state_time_ms >= ARM_AUTO_ARM_MIN_SETTLE_MS)
            {
                if(ArmAutoTask_YushuAtTarget((s32)s_auto_cfg.retract_length_mm) != 0U)
                {
                    ArmAutoTask_SetState(ARM_AUTO_STATE_DONE);
                }
            }
            break;

        case ARM_AUTO_STATE_DONE:
        case ARM_AUTO_STATE_ERROR:
        case ARM_AUTO_STATE_ABORT:
        default:
            break;
    }
}

u8 ArmAutoTask_GetState(void)
{
    return (u8)s_auto_state;
}

u8 ArmAutoTask_GetError(void)
{
    return (u8)s_auto_error;
}

u8 ArmAutoTask_IsBusy(void)
{
    return ((s_auto_state != ARM_AUTO_STATE_IDLE) &&
            (s_auto_state != ARM_AUTO_STATE_DONE) &&
            (s_auto_state != ARM_AUTO_STATE_ERROR) &&
            (s_auto_state != ARM_AUTO_STATE_ABORT)) ? 1U : 0U;
}

/******************* (C) COPYRIGHT 2026 END OF FILE *****************************/
