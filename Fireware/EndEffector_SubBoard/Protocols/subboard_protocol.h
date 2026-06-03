#ifndef __SUBBOARD_PROTOCOL_H
#define __SUBBOARD_PROTOCOL_H
#include "stm32f10x.h"

typedef enum
{
    SUB_CMD_HEARTBEAT = 0x01,
    SUB_CMD_ESTOP = 0x02,
    SUB_CMD_STOP = 0x03,
    SUB_CMD_SET_MODE = 0x04,
    SUB_CMD_SERVO123_SPEED = 0x20,
    SUB_CMD_SERVO123_POSITION = 0x21,
    SUB_CMD_AUTO_START = 0x30,
    SUB_CMD_AUTO_STOP = 0x31,
    SUB_CMD_PUSHROD = 0x40,
    SUB_FB_ACK = 0x80,
    SUB_FB_STATE = 0x81,
    SUB_FB_SERVO = 0x82,
    SUB_FB_ERROR = 0x83
} sub_cmd_t;

void SubProtocol_OnFrame(u8 cmd_id, const u8 *payload, u8 len);
void SubProtocol_SendAck(u8 cmd_id, u8 status);
void SubProtocol_SendState(u8 state, u8 error);

#endif
