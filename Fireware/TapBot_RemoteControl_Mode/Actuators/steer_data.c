/******************** (C) COPYRIGHT 2026 *****************************************
 * @file        steer_data.c
 * @brief       Feetech SMS servo driver: SyncWrite position/speed commands for servos ID0-ID3.
 * @note        
 * @warning     
 * @license     This project is released under the MIT License.
 *********************************************************************************/
#include "steer_data.h"
#include "control_math.h"
#include "control_dispatcher.h"
#include "app_config.h"
#include "subboard_protocol.h"



/*********************************  Global Variables ********************************************/
#define END_AUTO_FORWARD_TIME_MS    3000U
#define END_AUTO_REVERSE_TIME_MS    3000U
#define END_AUTO_ROTATE_SPEED       600


/* Speed mapping table for Y1 (steer) axis: input dead-zone points → output speeds */
float Y1_Speed_Arr[2][3] =
{
    /*   Left dead,    Mid dead,    Right dead  */
    { STEER_Y1_IN_LEFT,     STEER_Y1_IN_MID,     STEER_Y1_IN_RIGHT},
    { STEER_SPEED_OUT_LEFT, STEER_SPEED_OUT_MID, STEER_SPEED_OUT_RIGHT}
};


s16 SteerRunSpeed = 0;            // The currently calculated servo operating speed

u8 SteerRunMode = STEER_SPEED_MODE;
u8 SteerActualMode = STEER_MODE_UNKNOWN;


/*********************************  Global Variables ********************************************/
#define END_AUTO_FORWARD_TIME_MS    3000U
/* Async write buffer for a single servo (position + time + speed) */
u8 SteerDataBuf[13] = \
{
/*   0      1      2      3      4      5      6      7      8      9      10     11     12    */
/* Header  ID   Length  Cmd   RegAdr  PosData(L/H)   TimeData(L/H)   SpeedData(L/H)   Checksum */
    0xFF, 0xFF, 0x01, 0x09, 0x03, 0x2A, 0x00, 0x08, 0x00, 0x00, 0xE8, 0x03, 0x00
};


/* SyncWrite buffer for 4 servos (position + time + speed) */
u8 SteerDataBuf_SYNC[36] = \
{
/*   0      1      2      3      4      5      6    */
/* Header  ID   Length  Cmd   RegAdr  DataLen       */
    0xFF, 0xFF, 0xFE, 0x20, 0x83, 0x2A, 0x06, \

/*   7      8      9      10     11     12     13   */
/* Servo 0: PosData    TimeData      SpeedData      */
    0x00, 0x00, 0x08, 0x00, 0x00, 0xE8, 0x03, \

/*   14     15     16     17     18     19     20   */
/* Servo 1: PosData    TimeData      SpeedData      */
    0x01, 0x00, 0x08, 0x00, 0x00, 0xE8, 0x03, \

/*   21     22     23     24     25     26     27   */
/* Servo 2: PosData    TimeData      SpeedData      */
    0x02, 0x00, 0x08, 0x00, 0x00, 0xE8, 0x03, \

/*   28     29     30     31     32     33     34   */
/* Servo 3: PosData    TimeData      SpeedData      */
    0x03, 0x00, 0x08, 0x00, 0x00, 0xE8, 0x03, \

/*   35     */
/* Checksum */
    0x00
};



/* SyncWrite buffer: set operating mode for all 4 servos */
/* via the daughter board protocol  */
u8 SteerDataBuf_RunMode[16] = \
{
/*   0      1      2      3      4      5      6    */
/* Header  ID   Length  Cmd   RegAdr  DataLen       */
    0xFF, 0xFF, 0xFE, 0x0C, 0x83, 0x21, 0x01, \

/*   7      8      9      10     11     12     13     14   */
/* ID0   Mode    ID1   Mode    ID2   Mode    ID3   Mode    */
    0x00, 0x01, 0x01, 0x01, 0x02, 0x01, 0x03, 0x01, \

/*   15     */
/* Checksum */
    0x00
};


/* SyncWrite buffer: set running speed in speed mode for all 4 servos */
u8 SteerDataBuf_SpeedRun[20] = \
{
/*   0      1      2      3      4      5      6    */
/* Header  ID   Length  Cmd   RegAdr  DataLen       */
    0xFF, 0xFF, 0xFE, 0x10, 0x83, 0x2E, 0x02, \

/*   7      8      9      10     11     12     13     14     15     16     17     18   */
/* ID0   Speed     ID1   Speed     ID2   Speed     ID3   Speed                        */
    0x00, 0xE8, 0x03, 0x01, 0xE8, 0x03, 0x02, 0xE8, 0x03, 0x03, 0xE8, 0x03, \

/*   19     */
/* Checksum */
    0x00
};


/* Single-servo command template (used as scratch before dispatching to array) */
struct SteerSendMsg SteerSendMsg_1 =
{
                 1,     /* u8  SteerID     */
                 0,     /* u8  AccData     */
              2048,     /* s16 PosData     */
                 0,     /* u16 RunTime     */
              1000,     /* s16 SpeedData   */
};


/* Per-servo command array: ID0-ID2 position, ID3 continuous rotation (2048 = 180 deg) */
struct SteerSendMsg SteerSendMsgArr[4] =
{
    /* SteerID,  AccData,  PosData,  RunTime,  SpeedData */
    {    0,        0,       0,      0,        300},
    {    1,        0,       0,      0,        300},
    {    2,        0,       0,      0,        300},
    {    3,        0,    2420,      0,        300}
};


/*********************************  Core functions ********************************************/
/*
 * Copies the single-servo command into the matching slot of the 4-servo array
 * and sends via SyncWrite. Only ID0-ID2 (position servos) are updated.
 */
void SendSteerData(void)
{
    if(SteerSendMsg_1.SteerID < STEER_POSITION_SERVO_NUM)
    {
        SteerSendMsgArr[SteerSendMsg_1.SteerID].PosData = SteerSendMsg_1.PosData;
        SteerSendMsgArr[SteerSendMsg_1.SteerID].RunTime = SteerSendMsg_1.RunTime;
        SteerSendMsgArr[SteerSendMsg_1.SteerID].SpeedData = SteerSendMsg_1.SpeedData;
        SendSteer_Position3_SYNC_DataFun();
    }
}


/** 
 * Sets the operating mode of all 4 steering servos and propagates
 * the matching sub-board mode via UART5.
 *
 * @param SetMode: STEER_POSITION_MODE, STEER_SPEED_MODE, or STEER_MIXED_MODE.
 * @note  No-op if the mode is invalid or already active.
 */
void SendSteer_SYNC_SetMode(u8 SetMode)
{
    u8 sub_mode;

    if((SetMode != STEER_POSITION_MODE) && (SetMode != STEER_SPEED_MODE) && (SetMode != STEER_MIXED_MODE))
    {
        return;
    }

    SteerRunMode = SetMode;
    if(SteerActualMode == SetMode)
    {
        return;
    }

    sub_mode = (SetMode == STEER_SPEED_MODE) ? SUBBOARD_MODE_REMOTE_SPEED : SUBBOARD_MODE_UPPER_POSITION;
    SubBoardProtocol_SendSetMode(sub_mode);
    SteerActualMode = SetMode;
}


/*
 * Public wrapper for SyncWrite position data to servos ID0-ID2.
 */
void SendSteer_SYNC_DataFun(void)
{
    SendSteer_Position3_SYNC_DataFun();
}



/*
 * Sends SyncWrite position commands for servos ID0-ID2 to the sub-board.
 */
void SendSteer_Position3_SYNC_DataFun(void)
{
    SubBoardProtocol_SendServo123Position(SteerSendMsgArr[0].PosData,
                                            SteerSendMsgArr[1].PosData,
                                            SteerSendMsgArr[2].PosData,
                                            SteerSendMsgArr[0].RunTime,
                                            (u16)SteerSendMsgArr[0].SpeedData);
}

void SendSteer_SYNC_SetDefaultMode(void)
{
    SendSteer_SYNC_SetMode(STEER_MIXED_MODE);
}

void SendSteer_Rotate_SpeedRun(void)
{
    /* Servo ID3 is owned by the end-effector automatic task on the sub-board. */
}

void SendSteer_SYNC_SpeedRun(void)
{
    SubBoardProtocol_SendServo123Speed(SteerSendMsgArr[0].SpeedData,
                                        SteerSendMsgArr[1].SpeedData,
                                        SteerSendMsgArr[2].SpeedData);
    }


/*=======================(100m) Core function  =========================*/

/** 
 * @brief Periodic steer control processing (called at 100 ms).
 *
 * Maps the steer axis command through a 3-point curve to produce a speed,
 * applies per-servo enable/disable, sends the speed SyncWrite, and manages
 * the end-effector auto-task (start/stop) on servo ID3 transitions.
 */
void Steer_ControlProc(void)
{
    static u8 auto_req_old = 0U;
    u8 auto_req;
    u8 i;

    SteerRunSpeed = (s16)control_map_3point(g_ctrl_cmd.steer_axis_cmd, Y1_Speed_Arr);
    SteerRunSpeed = -SteerRunSpeed;

    for(i = 0U; i < STEER_POSITION_SERVO_NUM; i++)
    {
        if(g_ctrl_cmd.steer_enable[i] != 0U)
        {
            SteerSpeedValue_SpeedMode(&SteerSendMsgArr[i].SpeedData, &SteerRunSpeed);
        }
        else
        {
            SteerSendMsgArr[i].SpeedData = 0;
        }
    }

    if(SteerRunMode != STEER_SPEED_MODE)
    {
        SteerRunMode = STEER_SPEED_MODE;
        SendSteer_SYNC_SetMode(STEER_SPEED_MODE);
    }

    SendSteer_SYNC_SpeedRun();

    auto_req = (g_ctrl_cmd.steer_enable[STEER_ROTATE_SERVO_INDEX] != 0U) ? 1U : 0U;

    if((auto_req != 0U) && (auto_req_old == 0U))
    {
        SubBoardProtocol_SendAutoStart(END_AUTO_FORWARD_TIME_MS, END_AUTO_REVERSE_TIME_MS, END_AUTO_ROTATE_SPEED);
    }
    else if((auto_req == 0U) && (auto_req_old != 0U))
    {
        SubBoardProtocol_SendAutoStop();
    }

    auto_req_old = auto_req;
}


/** 
 * @brief Converts a signed speed value into Feetech speed-mode format.
 * @note  bit: bit 15 = 0 for CCW / positive, bit 15 = 1 for CW / negative.
 */
void SteerSpeedValue_SpeedMode(s16 * SpeedData, s16 * SteerSpeed)
{
    if(*SteerSpeed >= 0)
    {
        *SpeedData = *SteerSpeed;
        *SpeedData &= (~(1<<15));
    }
    else
    {
        *SpeedData = -*SteerSpeed;
        *SpeedData |= (1<<15);
    }
}


void RmtSteer_SpeedFun(void)
{
    Steer_ControlProc();
}



/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/





