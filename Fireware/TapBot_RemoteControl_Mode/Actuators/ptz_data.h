/******************** (C) COPYRIGHT 2024 *****************************************
 * File Name  : ptz_data.h
 * Description: PTZ command driver for the new gimbal protocol.
 *********************************************************************************/
#ifndef __PTZ_DATA_H
#define __PTZ_DATA_H

#include "function.h"

#define PTZ_DEVICE_ADDR         0x01U
#define PTZ_UART_PORT           UART4
#define PTZ_UART_BAUDRATE       9600U
#define PTZ_FRAME_LEN           7U

#define PTZ_START_BYTE          0xFFU
#define PTZ_CTRL_CMD_FIXED      0x00U

#define PTZ_LR_SPEED_DEF        0x20U
#define PTZ_UD_SPEED_DEF        0x20U
#define PTZ_SPEED_SLOW          0x10U
#define PTZ_SPEED_NORMAL        0x20U
#define PTZ_SPEED_FAST          0x30U
#define PTZ_SPEED_MAX           0x40U

#define PTZ_CHANNEL_OFFSET      100
#define PTZ_CHANNEL_MID         1500

#define PTZ_DEBUG_EN            0U

/* Internal direction flags used by remote-control and dispatcher layers. */
typedef enum
{
    PTZ_STOP  = 0U,
    PTZ_UP    = 1U,
    PTZ_DOWN  = 2U,
    PTZ_LEFT  = 3U,
    PTZ_RIGHT = 4U
} PTZ_Direction_E;

/*
 * New gimbal command byte values.
 *
 * The verified USER/rmt_data.c sends the values defined in USER/rmt_data.h:
 *   PTZ_UP=0x08, PTZ_DOWN=0x10, PTZ_LEFT=0x02, PTZ_RIGHT=0x04.
 * USER/ptz_data.h contains an opposite comment-level enum, but the actual
 * successful runtime path writes the USER/rmt_data.h values into MoveCmd.
 */
typedef enum
{
    PTZ_CMD_STOP  = 0x00U,
    PTZ_CMD_UP    = 0x08U,
    PTZ_CMD_DOWN  = 0x10U,
    PTZ_CMD_LEFT  = 0x02U,
    PTZ_CMD_RIGHT = 0x04U
} PTZ_MoveCmd_E;

typedef enum
{
    PTZ_STATUS_IDLE         = 0x00U,
    PTZ_STATUS_MOVING       = 0x01U,
    PTZ_STATUS_ERR_ADDR     = 0x02U,
    PTZ_STATUS_ERR_FRAME    = 0x03U,
    PTZ_STATUS_ERR_CHECKSUM = 0x04U
} PTZ_Status_E;

typedef struct
{
    u8 StartByte;
    u8 DevAddr;
    u8 CtrlCmd;
    u8 MoveCmd;
    u8 LRSpeed;
    u8 UDSpeed;
    u8 CheckSum;
} PTZ_SendFrame;

typedef struct
{
    u8 StartByte;
    u8 DevAddr;
    u8 Status;
    u8 Reserve[3];
    u8 CheckSum;
} PTZ_RecvFrame;

extern u8 PTZ_UpDownMoveFlg;
extern u8 PTZ_LftRgtMoveFlg;
extern PTZ_SendFrame PTZ_SendFrame_Cmd;
extern PTZ_RecvFrame PTZ_RecvFrame_Cmd;
extern PTZ_Status_E Ptz_WorkStatus;

void Send_PTZ_Data(void);
void PTZ_SetMoveCmd(PTZ_MoveCmd_E cmd);
void PTZ_SetLRSpeed(u8 speed);
void PTZ_SetUDSpeed(u8 speed);
void PTZ_Stop(void);
void PTZ_SetAngleTarget(s16 yaw_deg_x100, s16 pitch_deg_x100, u16 tolerance_x100, u8 speed);
void PTZ_DisableAngleCtrl(void);
void PTZ_RecvDataProc(u8 *pBuf, u16 len);
void PTZ_ControlProc(void);
void PTZ_ApplyControlCmd(void);

#endif



/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/
