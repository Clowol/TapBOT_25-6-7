/******************** (C) COPYRIGHT 2026 *****************************************
 * @file        steer_data.h
 * @brief       SMS servo driver. ID0-2 position servos, ID3 continuous rotation.
 *********************************************************************************/
#ifndef __STEER_DATA_H
#define __STEER_DATA_H

#include "stm32f10x.h"

#define STEER_POSITION_SERVO_NUM    3U
#define STEER_ROTATE_SERVO_INDEX    3U

typedef enum
{
    STEER_POSITION_MODE = 0,
    STEER_SPEED_MODE = 1,
    STEER_MIXED_MODE = 2,
    STEER_MODE_UNKNOWN = 0xFF
} steer_run_mode_t;

struct SteerSendMsg
{
    u8  SteerID;
    u8  AccData;
    s16 PosData;
    u16 RunTime;
    s16 SpeedData;
};

extern struct SteerSendMsg SteerSendMsg_1;
extern struct SteerSendMsg SteerSendMsgArr[4];
extern u8 SteerDataBuf[13];
extern u8 SteerDataBuf_SYNC[36];
extern u8 SteerRunMode;
extern u8 SteerActualMode;

void SendSteerData(void);
void SendSteer_SYNC_DataFun(void);
void SendSteer_Position3_SYNC_DataFun(void);
void SendSteer_SYNC_SetMode(u8 SetMode);
void SendSteer_SYNC_SetDefaultMode(void);
void SendSteer_SYNC_SpeedRun(void);
void SendSteer_Rotate_SpeedRun(void);
void Steer_ControlProc(void);
void RmtSteer_SpeedFun(void);
void SteerSpeedValue_SpeedMode(s16 *SpeedData, s16 *SteerSpeed);

#endif /* __STEER_DATA_H */



/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/

