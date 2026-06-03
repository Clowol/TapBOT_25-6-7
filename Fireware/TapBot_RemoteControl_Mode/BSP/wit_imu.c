/******************** (C) COPYRIGHT 2026 *****************************************
 * @file        wit_imu.c
 * @brief       WitMotion HWT905CAN IMU parser and feedback cache.
 *********************************************************************************/
#include "wit_imu.h"
#include "wit_c_sdk.h"
#include "REG.h"
#include "can2.h"
#include "delay.h"
#include "app_config.h"

static wit_imu_feedback_t s_wit_imu_feedback = {0};

static s16 WitImu_RegToDegX100(s16 reg_value)
{
    s32 value = (s32)reg_value * 18000L;
    value /= 32768L;

    if(value > 32767L)
    {
        value = 32767L;
    }
    else if(value < -32768L)
    {
        value = -32768L;
    }

    return (s16)value;
}

static void WitImu_CanWrite(u8 id, u8 *msg, u32 dlc)
{
    if(dlc > 8U)
    {
        dlc = 8U;
    }

    Can2_Send_Msg(msg, id, (u8)dlc);
}

static void WitImu_DelayMs(u16 ms)
{
    delay_ms(ms);
}

static void WitImu_RegUpdate(u32 reg, u32 reg_num)
{
    u32 end_reg = reg + reg_num;

    if((reg <= Roll) && (end_reg > Yaw))
    {
        s_wit_imu_feedback.roll_deg_x100 = WitImu_RegToDegX100(sReg[Roll]);
        s_wit_imu_feedback.pitch_deg_x100 = WitImu_RegToDegX100(sReg[Pitch]);
        s_wit_imu_feedback.yaw_deg_x100 = WitImu_RegToDegX100(sReg[Yaw]);
        s_wit_imu_feedback.valid = 1U;
    }
}

void WitImu_Init(void)
{
    WitInit(WIT_PROTOCOL_CAN, WIT_IMU_CAN_ADDR);
    WitRegisterCallBack(WitImu_RegUpdate);
    WitCanWriteRegister(WitImu_CanWrite);
    WitDelayMsRegister(WitImu_DelayMs);
}

void WitImu_OnCanFrame(u32 std_id, const u8 *data, u8 len)
{
    u8 frame[8];
    u8 i;

    (void)std_id;

    if((data == 0) || (len < 8U))
    {
        return;
    }

    for(i = 0U; i < 8U; i++)
    {
        frame[i] = data[i];
    }

    WitCanDataIn(frame, 8U);
}

const wit_imu_feedback_t *WitImu_GetFeedback(void)
{
    return &s_wit_imu_feedback;
}

/******************* (C) COPYRIGHT 2026 END OF FILE *****************************/
