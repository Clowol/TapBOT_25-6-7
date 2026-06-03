/******************** (C) COPYRIGHT 2026 *****************************************
 * @file    subboard_protocol.c
 * @brief   High-level command packing for the F103 end-effector sub-board.
 *********************************************************************************/
#include "subboard_protocol.h"
#include "subboard_link.h"

static void SubBoard_WriteS16LE(u8 *buf, u8 offset, s16 value)
{
    buf[offset] = (u8)((u16)value & 0xFFU);
    buf[offset + 1U] = (u8)(((u16)value >> 8) & 0xFFU);
}

static void SubBoard_WriteU16LE(u8 *buf, u8 offset, u16 value)
{
    buf[offset] = (u8)(value & 0xFFU);
    buf[offset + 1U] = (u8)((value >> 8) & 0xFFU);
}

void SubBoardProtocol_OnFrame(u8 cmd_id, const u8 *payload, u8 len)
{
    switch(cmd_id)
    {
        case SUBBOARD_FB_ACK:
            if((payload != 0) && (len >= 2U))
            {
                SubBoard_LinkSetStatus(payload[0], payload[1]);
            }
            break;

        case SUBBOARD_FB_STATE:
            if((payload != 0) && (len >= 2U))
            {
                SubBoard_LinkSetStatus(payload[0], payload[1]);
            }
            break;

        case SUBBOARD_FB_ERROR:
            if((payload != 0) && (len >= 1U))
            {
                SubBoard_LinkSetStatus(SUBBOARD_STATE_ERROR, payload[0]);
            }
            break;

        default:
            break;
    }
}

void SubBoardProtocol_SendHeartbeat(void)
{
    (void)SubBoard_LinkSendFrame(SUBBOARD_CMD_HEARTBEAT, 0, 0U);
}

void SubBoardProtocol_SendSetMode(u8 mode)
{
    u8 payload[1];

    payload[0] = mode;
    (void)SubBoard_LinkSendFrame(SUBBOARD_CMD_SET_MODE, payload, 1U);
}

void SubBoardProtocol_SendServo123Speed(s16 speed0, s16 speed1, s16 speed2)
{
    u8 payload[6];

    SubBoard_WriteS16LE(payload, 0U, speed0);
    SubBoard_WriteS16LE(payload, 2U, speed1);
    SubBoard_WriteS16LE(payload, 4U, speed2);
    (void)SubBoard_LinkSendFrame(SUBBOARD_CMD_SERVO123_SPEED, payload, 6U);
}

void SubBoardProtocol_SendServo123Position(s16 pos0, s16 pos1, s16 pos2, u16 run_time, u16 speed)
{
    u8 payload[10];

    SubBoard_WriteS16LE(payload, 0U, pos0);
    SubBoard_WriteS16LE(payload, 2U, pos1);
    SubBoard_WriteS16LE(payload, 4U, pos2);
    SubBoard_WriteU16LE(payload, 6U, run_time);
    SubBoard_WriteU16LE(payload, 8U, speed);
    (void)SubBoard_LinkSendFrame(SUBBOARD_CMD_SERVO123_POSITION, payload, 10U);
}

void SubBoardProtocol_SendAutoStart(u16 forward_ms, u16 reverse_ms, s16 speed)
{
    u8 payload[6];

    SubBoard_WriteU16LE(payload, 0U, forward_ms);
    SubBoard_WriteU16LE(payload, 2U, reverse_ms);
    SubBoard_WriteS16LE(payload, 4U, speed);
    (void)SubBoard_LinkSendFrame(SUBBOARD_CMD_AUTO_START, payload, 6U);
}

void SubBoardProtocol_SendAutoStop(void)
{
    (void)SubBoard_LinkSendFrame(SUBBOARD_CMD_AUTO_STOP, 0, 0U);
}

void SubBoardProtocol_SendEStop(void)
{
    (void)SubBoard_LinkSendFrame(SUBBOARD_CMD_ESTOP, 0, 0U);
}

/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/
