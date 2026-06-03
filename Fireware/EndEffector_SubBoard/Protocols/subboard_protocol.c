#include "subboard_protocol.h"
#include "rs485_link.h"
#include "end_effector_task.h"

static s16 ReadS16LE(const u8 *data)
{
    return (s16)((u16)data[0] | ((u16)data[1] << 8));
}

static u16 ReadU16LE(const u8 *data)
{
    return (u16)((u16)data[0] | ((u16)data[1] << 8));
}

void SubProtocol_OnFrame(u8 cmd_id, const u8 *payload, u8 len)
{
    switch(cmd_id)
    {
        case SUB_CMD_HEARTBEAT:
            SubProtocol_SendState(EndEffectorTask_GetState(), EndEffectorTask_GetError());
            break;

        case SUB_CMD_ESTOP:
            EndEffectorTask_EStop();
            SubProtocol_SendAck(cmd_id, 0U);
            break;

        case SUB_CMD_STOP:
        case SUB_CMD_AUTO_STOP:
            EndEffectorTask_StopAuto();
            SubProtocol_SendAck(cmd_id, 0U);
            break;

        case SUB_CMD_SET_MODE:
            if((payload != 0) && (len >= 1U))
            {
                EndEffectorTask_SetMode(payload[0]);
                SubProtocol_SendAck(cmd_id, 0U);
            }
            break;

        case SUB_CMD_SERVO123_SPEED:
            if((payload != 0) && (len >= 6U))
            {
                EndEffectorTask_SetServo123Speed(ReadS16LE(&payload[0]), ReadS16LE(&payload[2]), ReadS16LE(&payload[4]));
                SubProtocol_SendAck(cmd_id, 0U);
            }
            break;

        case SUB_CMD_SERVO123_POSITION:
            if((payload != 0) && (len >= 10U))
            {
                EndEffectorTask_SetServo123Position(ReadS16LE(&payload[0]), ReadS16LE(&payload[2]), ReadS16LE(&payload[4]),
                                                   ReadU16LE(&payload[6]), ReadU16LE(&payload[8]));
                SubProtocol_SendAck(cmd_id, 0U);
            }
            break;

        case SUB_CMD_AUTO_START:
            if((payload != 0) && (len >= 6U))
            {
                EndEffectorTask_StartAuto(ReadU16LE(&payload[0]), ReadU16LE(&payload[2]), ReadS16LE(&payload[4]));
                SubProtocol_SendAck(cmd_id, 0U);
            }
            break;

        default:
            SubProtocol_SendAck(cmd_id, 1U);
            break;
    }
}

void SubProtocol_SendAck(u8 cmd_id, u8 status)
{
    u8 payload[3];
    payload[0] = EndEffectorTask_GetState();
    payload[1] = status;
    payload[2] = cmd_id;
    (void)Rs485Link_SendFrame(SUB_FB_ACK, payload, 3U);
}

void SubProtocol_SendState(u8 state, u8 error)
{
    u8 payload[2];
    payload[0] = state;
    payload[1] = error;
    (void)Rs485Link_SendFrame(SUB_FB_STATE, payload, 2U);
}
