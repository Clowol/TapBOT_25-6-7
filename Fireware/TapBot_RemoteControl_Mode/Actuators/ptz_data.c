/******************** (C) COPYRIGHT 2024 *****************************************
 * File Name  : ptz_data.c
 * Description: PTZ command driver for the new gimbal protocol.
*********************************************************************************/
/******************** (C) COPYRIGHT 2026 *****************************************
   * @author      Clomol
   * @date        2026-2027
   * @brief       
   * @license     [z]本代码仅用于教学与科研目的，未经作者书面许可，不得用于商业用途
   *              This project is released under the MIT License.
   * @note        
   * @warning     
*********************************************************************************/
#include "ptz_data.h"
#include "control_dispatcher.h"
#include "wit_imu.h"
#include <string.h>

#if PTZ_DEBUG_EN == 1U
#define PTZ_DEBUG_PRINT(...)    swgPrtUx(USART2, __VA_ARGS__)
#else
#define PTZ_DEBUG_PRINT(...)
#endif

u8 PTZ_UpDownMoveFlg = PTZ_STOP;
u8 PTZ_LftRgtMoveFlg = PTZ_STOP;

PTZ_SendFrame PTZ_SendFrame_Cmd =
{
    PTZ_START_BYTE,
    PTZ_DEVICE_ADDR,
    PTZ_CTRL_CMD_FIXED,
    PTZ_CMD_STOP,
    PTZ_LR_SPEED_DEF,
    PTZ_UD_SPEED_DEF,
    0x00U
};

PTZ_RecvFrame PTZ_RecvFrame_Cmd = {0};
PTZ_Status_E Ptz_WorkStatus = PTZ_STATUS_IDLE;
static u8 PtzAngleCtrlEnable = 0U;
static s16 PtzTargetYawX100 = 0;
static s16 PtzTargetPitchX100 = 0;
static u16 PtzTargetToleranceX100 = PTZ_ANGLE_DEFAULT_TOL_X100;
static u8 PtzTargetSpeed = PTZ_ANGLE_DEFAULT_SPEED;

static u8 PTZ_CheckSum(const u8 *pBuff, u16 len)
{
    u16 i;
    u8 sum = 0U;

    if((pBuff == 0) || (len == 0U))
    {
        Ptz_WorkStatus = PTZ_STATUS_ERR_FRAME;
        return 0U;
    }

    for(i = 0U; i < len; i++)
    {
        sum = (u8)(sum + pBuff[i]);
    }

    return sum;
}

static u8 PTZ_LimitSpeed(u8 speed, u8 default_speed)
{
    if(speed > PTZ_SPEED_MAX)
    {
        return default_speed;
    }

    return speed;
}

static PTZ_MoveCmd_E PTZ_DirectionToCmd(u8 up_down, u8 left_right)
{
    if(left_right == PTZ_LEFT)
    {
        return PTZ_CMD_LEFT;
    }
    if(left_right == PTZ_RIGHT)
    {
        return PTZ_CMD_RIGHT;
    }
    if(up_down == PTZ_UP)
    {
        return PTZ_CMD_UP;
    }
    if(up_down == PTZ_DOWN)
    {
        return PTZ_CMD_DOWN;
    }

    return PTZ_CMD_STOP;
}

static void PTZ_UpdateFrameFromFlags(void)
{
    PTZ_SendFrame_Cmd.StartByte = PTZ_START_BYTE;
    PTZ_SendFrame_Cmd.DevAddr = PTZ_DEVICE_ADDR;
    PTZ_SendFrame_Cmd.CtrlCmd = PTZ_CTRL_CMD_FIXED;
    PTZ_SendFrame_Cmd.MoveCmd = (u8)PTZ_DirectionToCmd(PTZ_UpDownMoveFlg, PTZ_LftRgtMoveFlg);

    if((PTZ_UpDownMoveFlg == PTZ_STOP) && (PTZ_LftRgtMoveFlg == PTZ_STOP))
    {
        PTZ_SendFrame_Cmd.LRSpeed = 0U;
        PTZ_SendFrame_Cmd.UDSpeed = 0U;
        Ptz_WorkStatus = PTZ_STATUS_IDLE;
    }
    else
    {
        if(PTZ_LftRgtMoveFlg == PTZ_STOP)
        {
            PTZ_SendFrame_Cmd.LRSpeed = 0U;
        }
        if(PTZ_UpDownMoveFlg == PTZ_STOP)
        {
            PTZ_SendFrame_Cmd.UDSpeed = 0U;
        }
        Ptz_WorkStatus = PTZ_STATUS_MOVING;
    }

    PTZ_SendFrame_Cmd.CheckSum = PTZ_CheckSum(&PTZ_SendFrame_Cmd.DevAddr, 5U);
}

void Send_PTZ_Data(void)
{
    u8 i;
    u32 timeout;
    const u8 *frame;

    PTZ_UpdateFrameFromFlags();
    frame = (const u8 *)&PTZ_SendFrame_Cmd;

    for(i = 0U; i < PTZ_FRAME_LEN; i++)
    {
        timeout = 0U;
        USART_SendData(PTZ_UART_PORT, frame[i]);
        while(((PTZ_UART_PORT->SR & 0x40U) == 0U) && (timeout < 0xFFFFU))
        {
            timeout++;
        }
        if(timeout >= 0xFFFFU)
        {
            Ptz_WorkStatus = PTZ_STATUS_ERR_FRAME;
            PTZ_DEBUG_PRINT("PTZ UART4 send timeout\r\n");
            break;
        }
    }

#if PTZ_DEBUG_EN == 1U
    PTZ_DEBUG_PRINT("PTZ Send: ");
    for(i = 0U; i < PTZ_FRAME_LEN; i++)
    {
        PTZ_DEBUG_PRINT("%02X ", frame[i]);
    }
    PTZ_DEBUG_PRINT("\r\n");
#endif
}

void PTZ_SetMoveCmd(PTZ_MoveCmd_E cmd)
{
    PTZ_UpDownMoveFlg = PTZ_STOP;
    PTZ_LftRgtMoveFlg = PTZ_STOP;

    switch(cmd)
    {
        case PTZ_CMD_UP:
            PTZ_UpDownMoveFlg = PTZ_UP;
            PTZ_SendFrame_Cmd.UDSpeed = PTZ_SPEED_NORMAL;
            break;

        case PTZ_CMD_DOWN:
            PTZ_UpDownMoveFlg = PTZ_DOWN;
            PTZ_SendFrame_Cmd.UDSpeed = PTZ_SPEED_NORMAL;
            break;

        case PTZ_CMD_LEFT:
            PTZ_LftRgtMoveFlg = PTZ_LEFT;
            PTZ_SendFrame_Cmd.LRSpeed = PTZ_SPEED_NORMAL;
            break;

        case PTZ_CMD_RIGHT:
            PTZ_LftRgtMoveFlg = PTZ_RIGHT;
            PTZ_SendFrame_Cmd.LRSpeed = PTZ_SPEED_NORMAL;
            break;

        case PTZ_CMD_STOP:
        default:
            PTZ_SendFrame_Cmd.LRSpeed = 0U;
            PTZ_SendFrame_Cmd.UDSpeed = 0U;
            break;
    }

    PTZ_UpdateFrameFromFlags();
}

void PTZ_SetLRSpeed(u8 speed)
{
    PTZ_SendFrame_Cmd.LRSpeed = PTZ_LimitSpeed(speed, PTZ_LR_SPEED_DEF);
    PTZ_UpdateFrameFromFlags();
}

void PTZ_SetUDSpeed(u8 speed)
{
    PTZ_SendFrame_Cmd.UDSpeed = PTZ_LimitSpeed(speed, PTZ_UD_SPEED_DEF);
    PTZ_UpdateFrameFromFlags();
}

void PTZ_Stop(void)
{
    PTZ_UpDownMoveFlg = PTZ_STOP;
    PTZ_LftRgtMoveFlg = PTZ_STOP;
    PTZ_SendFrame_Cmd.LRSpeed = 0U;
    PTZ_SendFrame_Cmd.UDSpeed = 0U;
    Send_PTZ_Data();
}

static s16 PTZ_AngleDiffX100(s16 target, s16 current)
{
    s32 diff = (s32)target - (s32)current;

    while(diff > 18000L)
    {
        diff -= 36000L;
    }
    while(diff < -18000L)
    {
        diff += 36000L;
    }

    return (s16)diff;
}

void PTZ_SetAngleTarget(s16 yaw_deg_x100, s16 pitch_deg_x100, u16 tolerance_x100, u8 speed)
{
    PtzTargetYawX100 = yaw_deg_x100;
    PtzTargetPitchX100 = pitch_deg_x100;
    PtzTargetToleranceX100 = (tolerance_x100 == 0U) ? PTZ_ANGLE_DEFAULT_TOL_X100 : tolerance_x100;
    PtzTargetSpeed = (speed == 0U) ? PTZ_ANGLE_DEFAULT_SPEED : speed;
    PtzAngleCtrlEnable = 1U;
}

void PTZ_DisableAngleCtrl(void)
{
    PtzAngleCtrlEnable = 0U;
}

static void PTZ_AngleControlProc(void)
{
    const wit_imu_feedback_t *imu = WitImu_GetFeedback();
    s16 yaw_error;
    s16 pitch_error;

    if((PtzAngleCtrlEnable == 0U) || (imu->valid == 0U))
    {
        return;
    }

    yaw_error = PTZ_AngleDiffX100(PtzTargetYawX100, imu->yaw_deg_x100);
    pitch_error = PTZ_AngleDiffX100(PtzTargetPitchX100, imu->pitch_deg_x100);

    PTZ_LftRgtMoveFlg = PTZ_STOP;
    PTZ_UpDownMoveFlg = PTZ_STOP;

    if(yaw_error > (s16)PtzTargetToleranceX100)
    {
        PTZ_LftRgtMoveFlg = PTZ_RIGHT;
    }
    else if(yaw_error < -((s16)PtzTargetToleranceX100))
    {
        PTZ_LftRgtMoveFlg = PTZ_LEFT;
    }

    if(pitch_error > (s16)PtzTargetToleranceX100)
    {
        PTZ_UpDownMoveFlg = PTZ_UP;
    }
    else if(pitch_error < -((s16)PtzTargetToleranceX100))
    {
        PTZ_UpDownMoveFlg = PTZ_DOWN;
    }

    PTZ_SetLRSpeed((PTZ_LftRgtMoveFlg == PTZ_STOP) ? 0U : PtzTargetSpeed);
    PTZ_SetUDSpeed((PTZ_UpDownMoveFlg == PTZ_STOP) ? 0U : PtzTargetSpeed);
    Send_PTZ_Data();

    if((PTZ_LftRgtMoveFlg == PTZ_STOP) && (PTZ_UpDownMoveFlg == PTZ_STOP))
    {
        PtzAngleCtrlEnable = 0U;
    }
}

void PTZ_RecvDataProc(u8 *pBuf, u16 len)
{
    u8 check_sum;

    if((pBuf == 0) || (len != PTZ_FRAME_LEN))
    {
        Ptz_WorkStatus = PTZ_STATUS_ERR_FRAME;
        return;
    }

    memcpy(&PTZ_RecvFrame_Cmd, pBuf, PTZ_FRAME_LEN);

    if((PTZ_RecvFrame_Cmd.StartByte != PTZ_START_BYTE) || (PTZ_RecvFrame_Cmd.DevAddr != PTZ_DEVICE_ADDR))
    {
        Ptz_WorkStatus = PTZ_STATUS_ERR_ADDR;
        return;
    }

    check_sum = PTZ_CheckSum(&pBuf[1], 5U);
    if(PTZ_RecvFrame_Cmd.CheckSum != check_sum)
    {
        Ptz_WorkStatus = PTZ_STATUS_ERR_CHECKSUM;
        return;
    }

    Ptz_WorkStatus = (PTZ_Status_E)PTZ_RecvFrame_Cmd.Status;
}

void PTZ_ControlProc(void)
{
    if(g_ctrl_source_active != CTRL_SRC_UPPER)
    {
        return;
    }

    if(PtzAngleCtrlEnable != 0U)
    {
        PTZ_AngleControlProc();
        return;
    }

    PTZ_UpDownMoveFlg = g_ctrl_cmd.ptz_up_down_cmd;
    PTZ_LftRgtMoveFlg = g_ctrl_cmd.ptz_left_right_cmd;
    PTZ_SetUDSpeed(g_ctrl_cmd.ptz_up_down_speed);
    PTZ_SetLRSpeed(g_ctrl_cmd.ptz_left_right_speed);
    Send_PTZ_Data();
}

void PTZ_ApplyControlCmd(void)
{
    PTZ_ControlProc();
}


/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/

