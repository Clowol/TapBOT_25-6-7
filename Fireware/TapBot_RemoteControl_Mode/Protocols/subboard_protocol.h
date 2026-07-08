/******************** (C) COPYRIGHT 2026 *****************************************
 * @file    subboard_protocol.h
 * @brief   Command definitions between F107 main board and F103 end-effector board.
 * @note        
 * @warning     
 * @license     This project is released under the MIT License.
 *********************************************************************************/
#ifndef __SUBBOARD_PROTOCOL_H
#define __SUBBOARD_PROTOCOL_H

#include "stm32f10x.h"


/*=================================== Enumeration definition =========================*/
/* 0x01-0x7f (main to sub); 0x80-0xff(sub to main) */
typedef enum
{
    SUBBOARD_CMD_HEARTBEAT          = 0x01,
    SUBBOARD_CMD_ESTOP              = 0x02,
    SUBBOARD_CMD_STOP               = 0x03,
    SUBBOARD_CMD_SET_MODE           = 0x04,
    SUBBOARD_CMD_SERVO123_SPEED     = 0x20,
    SUBBOARD_CMD_SERVO123_POSITION  = 0x21,
    SUBBOARD_CMD_AUTO_START         = 0x30,
    SUBBOARD_CMD_AUTO_STOP          = 0x31,
    SUBBOARD_CMD_PUSHROD            = 0x40,

    SUBBOARD_FB_ACK                 = 0x80,
    SUBBOARD_FB_STATE               = 0x81,
    SUBBOARD_FB_SERVO               = 0x82,
    SUBBOARD_FB_ERROR               = 0x83
} subboard_cmd_id_t;



typedef enum
{
    SUBBOARD_MODE_REMOTE_SPEED = 0,
    SUBBOARD_MODE_UPPER_POSITION = 1,
    SUBBOARD_MODE_AUTO = 2,
    SUBBOARD_MODE_ESTOP = 3
} subboard_mode_t;



typedef enum
{
    SUBBOARD_STATE_UNKNOWN = 0xFF,
    SUBBOARD_STATE_IDLE = 0,
    SUBBOARD_STATE_REMOTE = 1,
    SUBBOARD_STATE_UPPER = 2,
    SUBBOARD_STATE_WAIT_OBJECT = 3,
    SUBBOARD_STATE_PUSHROD_EXTEND = 4,
    SUBBOARD_STATE_PRESS_HOLD = 5,
    SUBBOARD_STATE_SERVO_FORWARD = 6,
    SUBBOARD_STATE_SERVO_REVERSE = 7,
    SUBBOARD_STATE_WAIT_OBJECT_CLEAR = 8,
    SUBBOARD_STATE_PUSHROD_RETRACT = 9,
    SUBBOARD_STATE_DONE = 10,
    SUBBOARD_STATE_ERROR = 11,
    SUBBOARD_STATE_ESTOP = 12
} subboard_state_t;

typedef enum
{
    SUBBOARD_ERR_NONE = 0,
    SUBBOARD_ERR_CRC = 1,
    SUBBOARD_ERR_LEN = 2,
    SUBBOARD_ERR_TIMEOUT = 3,
    SUBBOARD_ERR_SENSOR = 4,
    SUBBOARD_ERR_PUSHROD = 5,
    SUBBOARD_ERR_SERVO = 6
} subboard_error_t;



/*================================ Function prototype =======================================*/
/* Receive callback function */
void SubBoardProtocol_OnFrame(u8 cmd_id, const u8 *payload, u8 len);

void SubBoardProtocol_SendHeartbeat(void);
void SubBoardProtocol_SendSetMode(u8 mode);
void SubBoardProtocol_SendServo123Speed(s16 speed0, s16 speed1, s16 speed2);
void SubBoardProtocol_SendServo123Position(s16 pos0, s16 pos1, s16 pos2, u16 run_time, u16 speed);

void SubBoardProtocol_SendAutoStart(u16 forward_ms, u16 reverse_ms, s16 speed);
void SubBoardProtocol_SendAutoStop(void);
void SubBoardProtocol_SendEStop(void);

#endif /* __SUBBOARD_PROTOCOL_H */

/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/
