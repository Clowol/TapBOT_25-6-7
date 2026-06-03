/******************** (C) COPYRIGHT 2026 *****************************************
 * @file        upper_feedback.c
 * @brief       Packs STM32 feedback frames for the upper computer on USART2.
 *
 * Frame:
 *   0xA5 cmd_id payload_len payload crc_low crc_high
 *
 * CRC16-CCITT:
 *   init 0xFFFF, poly 0x1021, over cmd_id + payload_len + payload.
 *********************************************************************************/
#include "upper_feedback.h"
#include "usart.h"
#include "control_dispatcher.h"
#include "encoder.h"
#include "steer_data.h"
#include "ptz_data.h"
#include "yushu_motor_data.h"
#include "switch.h"
#include "timer.h"
#include "app_config.h"
#include "wit_imu.h"

#define UPPER_FB_PAYLOAD_MAX_LEN        32U
#define UPPER_FB_FRAME_MAX_LEN          (UPPER_FB_PAYLOAD_MAX_LEN + 5U)
#define UPPER_FB_SYSTEM_STATE_OK        0U

static u16 s_upper_fb_seq = 0U;
static u32 s_upper_fb_time_ms = 0U;

static u16 UpperFeedback_Crc16Ccitt(const u8 *data, u16 len)
{
    u16 crc = 0xFFFFU;
    u16 i;
    u8 bit;

    for(i = 0U; i < len; i++)
    {
        crc ^= ((u16)data[i] << 8);
        for(bit = 0U; bit < 8U; bit++)
        {
            if((crc & 0x8000U) != 0U)
            {
                crc = (u16)((crc << 1) ^ 0x1021U);
            }
            else
            {
                crc <<= 1;
            }
        }
    }

    return crc;
}

static void UpperFeedback_PutU16LE(u8 *buf, u8 *idx, u16 value)
{
    buf[*idx] = (u8)(value & 0xFFU);
    (*idx)++;
    buf[*idx] = (u8)((value >> 8) & 0xFFU);
    (*idx)++;
}

static void UpperFeedback_PutS16LE(u8 *buf, u8 *idx, s16 value)
{
    UpperFeedback_PutU16LE(buf, idx, (u16)value);
}

static void UpperFeedback_PutU32LE(u8 *buf, u8 *idx, u32 value)
{
    buf[*idx] = (u8)(value & 0xFFU);
    (*idx)++;
    buf[*idx] = (u8)((value >> 8) & 0xFFU);
    (*idx)++;
    buf[*idx] = (u8)((value >> 16) & 0xFFU);
    (*idx)++;
    buf[*idx] = (u8)((value >> 24) & 0xFFU);
    (*idx)++;
}

static void UpperFeedback_PutS32LE(u8 *buf, u8 *idx, s32 value)
{
    UpperFeedback_PutU32LE(buf, idx, (u32)value);
}

static s16 UpperFeedback_ScaleFloatToS16(float value, float scale)
{
    float scaled = value * scale;

    if(scaled >= 0.0f)
    {
        scaled += 0.5f;
    }
    else
    {
        scaled -= 0.5f;
    }

    if(scaled > 32767.0f)
    {
        return 32767;
    }
    if(scaled < -32768.0f)
    {
        return (s16)-32768;
    }

    return (s16)scaled;
}

static u8 UpperFeedback_GetClutchState(void)
{
    return (u8)GPIO_ReadOutputDataBit(GPIOD, GPIO_Pin_12);
}

static u16 UpperFeedback_GetFaultFlags(void)
{
    u16 flags = 0U;

    if(ControlDispatcher_IsUpperAlive() == 0U)
    {
        flags |= (1U << 0);
    }
    if(RmtPwrOffFlg != 0U)
    {
        flags |= (1U << 1);
    }
    if(CAN1_OutCommunFlg != 0U)
    {
        flags |= (1U << 2);
    }
    if(Ptz_WorkStatus >= PTZ_STATUS_ERR_ADDR)
    {
        flags |= (1U << 3);
    }
    if((CAN2_OutCommunFlg != 0U) || (WitImu_GetFeedback()->valid == 0U))
    {
        flags |= (1U << 4);
    }

    return flags;
}

static void UpperFeedback_SendFrame(u8 cmd_id, const u8 *payload, u8 payload_len)
{
    u8 frame[UPPER_FB_FRAME_MAX_LEN];
    u16 crc;
    u8 i;

    if(payload_len > UPPER_FB_PAYLOAD_MAX_LEN)
    {
        return;
    }

    frame[0] = UPPER_FB_FRAME_HEAD;
    frame[1] = cmd_id;
    frame[2] = payload_len;

    for(i = 0U; i < payload_len; i++)
    {
        frame[3U + i] = payload[i];
    }

    crc = UpperFeedback_Crc16Ccitt(&frame[1], (u16)(payload_len + 2U));
    frame[3U + payload_len] = (u8)(crc & 0xFFU);
    frame[4U + payload_len] = (u8)((crc >> 8) & 0xFFU);

    USART2_DMA_send(frame, (u8)(payload_len + 5U));
}

void UpperFeedback_SendArmState(void)
{
    u8 payload[31];
    u8 idx = 0U;
    const encoder_feedback_t *encoder = Encoder_GetFeedback();
    const wit_imu_feedback_t *imu = WitImu_GetFeedback();
    u8 i;

    s_upper_fb_time_ms += 50U;

    UpperFeedback_PutU16LE(payload, &idx, s_upper_fb_seq++);
    UpperFeedback_PutU32LE(payload, &idx, s_upper_fb_time_ms);
    payload[idx++] = (u8)g_ctrl_source_active;
    payload[idx++] = UPPER_FB_SYSTEM_STATE_OK;
    UpperFeedback_PutU16LE(payload, &idx, UpperFeedback_GetFaultFlags());
    UpperFeedback_PutS32LE(payload, &idx, encoder->length_mm);
    UpperFeedback_PutS16LE(payload, &idx, encoder->line_speed_mm_s);
    payload[idx++] = (u8)LIMIT_SWITCH_IN;
    payload[idx++] = UpperFeedback_GetClutchState();

    for(i = 0U; i < STEER_POSITION_SERVO_NUM; i++)
    {
        UpperFeedback_PutS16LE(payload, &idx, SteerSendMsgArr[i].PosData);
    }
    UpperFeedback_PutS16LE(payload, &idx, SteerSendMsgArr[STEER_ROTATE_SERVO_INDEX].SpeedData);

    UpperFeedback_PutS16LE(payload, &idx, imu->yaw_deg_x100);
    UpperFeedback_PutS16LE(payload, &idx, imu->pitch_deg_x100);
    payload[idx++] = imu->valid;

    UpperFeedback_SendFrame(UPPER_FB_CMD_ARM_STATE, payload, idx);
}

void UpperFeedback_SendActuatorEcho(void)
{
    u8 payload[24];
    u8 idx = 0U;
    u8 i;

    UpperFeedback_PutS16LE(payload, &idx, UpperFeedback_ScaleFloatToS16(YushuSpeed, 10.0f));
    UpperFeedback_PutS16LE(payload, &idx, UpperFeedback_ScaleFloatToS16(cmd.W, 10.0f));
    UpperFeedback_PutS16LE(payload, &idx, UpperFeedback_ScaleFloatToS16(cmd.T, 100.0f));
    UpperFeedback_PutS16LE(payload, &idx, UpperFeedback_ScaleFloatToS16(cmd.K_W, 1000.0f));
    payload[idx++] = (u8)cmd.mode;

    for(i = 0U; i < CTRL_STEER_NUM; i++)
    {
        UpperFeedback_PutS16LE(payload, &idx, SteerSendMsgArr[i].SpeedData);
    }

    payload[idx++] = g_ctrl_cmd.ptz_up_down_cmd;
    payload[idx++] = g_ctrl_cmd.ptz_left_right_cmd;
    payload[idx++] = g_ctrl_cmd.ptz_up_down_speed;
    payload[idx++] = g_ctrl_cmd.ptz_left_right_speed;
    payload[idx++] = Encoder_GetFeedback()->valid;
    payload[idx++] = (u8)SteerActualMode;
    payload[idx++] = (u8)Ptz_WorkStatus;

    UpperFeedback_SendFrame(UPPER_FB_CMD_ACTUATOR_ECHO, payload, idx);
}

void UpperFeedback_SendDiag(void)
{
    u8 payload[9];
    u8 idx = 0U;

    payload[idx++] = (RmtPwrOffFlg == 0U) ? 1U : 0U;
    payload[idx++] = ControlDispatcher_IsUpperAlive();
    payload[idx++] = Encoder_GetFeedback()->valid;
    payload[idx++] = WitImu_GetFeedback()->valid;
    payload[idx++] = 0U;
    UpperFeedback_PutU16LE(payload, &idx, (u16)LIMIT_SWITCH_IN);
    UpperFeedback_PutU16LE(payload, &idx, UpperFeedback_GetFaultFlags());

    UpperFeedback_SendFrame(UPPER_FB_CMD_DIAG, payload, idx);
}

/******************* (C) COPYRIGHT 2026 END OF FILE *****************************/
