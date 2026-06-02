/******************** (C) COPYRIGHT 2024 *****************************************
 * File Name  : yushu_motor_data.c
 * Description: Unitree M8010 actuator command generation.
*********************************************************************************/
#include "yushu_motor_data.h"
#include "control_math.h"
#include "control_dispatcher.h"
#include "app_config.h"

float Gear_Speed_Arr[2][3] =
{
    { YUSHU_GEAR_IN_BACK,   YUSHU_GEAR_IN_MID,     YUSHU_GEAR_IN_FORWARD },
    { YUSHU_SPEED_OUT_BACK, YUSHU_SPEED_OUT_MID,   YUSHU_SPEED_OUT_FORWARD }
};

float YushuSpeed = 0.0f;

static s16 Yushu_ScaleFloat(float value, float scale)
{
    if(value >= 0.0f)
    {
        return (s16)(value * scale + 0.5f);
    }
    return (s16)(value * scale - 0.5f);
}

static void YushuMotor_InitCmdOnce(void)
{
    cmd.id = YUSHU_M8010_ID;
    cmd.mode = YUSHU_M8010_MODE_STOP;
    cmd.T = 0.0f;
    cmd.W = 0.0f;
    cmd.Pos = 0.0f;
    cmd.K_P = 0.0f;
    cmd.K_W = 0.0f;
}

void YushuMotor_SendControl(void)
{
#if YUSHU_M8010_USE_CAN
    u8 can_data[8];
    s16 speed_cmd = Yushu_ScaleFloat(cmd.W, 10.0f);
    s16 torque_cmd = Yushu_ScaleFloat(cmd.T, 100.0f);
    s16 kw_cmd = Yushu_ScaleFloat(cmd.K_W, 1000.0f);

    can_data[0] = LBT(speed_cmd);
    can_data[1] = HBT(speed_cmd);
    can_data[2] = LBT(torque_cmd);
    can_data[3] = HBT(torque_cmd);
    can_data[4] = LBT(kw_cmd);
    can_data[5] = HBT(kw_cmd);
    can_data[6] = (u8)cmd.mode;
    can_data[7] = (u8)cmd.id;
    Can1_Send_Msg(can_data, YUSHU_M8010_CAN_TX_ID, 8U);
#else
    USART1_DMA_send((uint8_t *)&cmd, sizeof(cmd.motor_send_data));
#endif
}

void SendYushuMotorDataFun(void)
{
}

void SendYushuMotor_MoveCmd(u8 MoveFlag)
{
    static MOTOR_send *pData;
    static u8 FirstFlag = 1U;

    if(FirstFlag == 1U)
    {
        YushuMotor_InitCmdOnce();
        FirstFlag = 0U;
    }

    switch(MoveFlag)
    {
        case YUSHU_SHORTEN:
            cmd.mode = YUSHU_M8010_MODE_RUN;
            cmd.T = 0.0f;
            cmd.W = 20.0f;
            cmd.K_W = 0.05f;
            break;

        case YUSHU_STRETCH:
            cmd.mode = YUSHU_M8010_MODE_RUN;
            cmd.T = 0.0f;
            cmd.W = -20.0f;
            cmd.K_W = 0.05f;
            break;

        case YUSHU_STOP:
        default:
            cmd.mode = YUSHU_M8010_MODE_STOP;
            cmd.T = 0.0f;
            cmd.W = 0.0f;
            cmd.K_W = 0.0f;
            break;
    }

    pData = &cmd;
    modify_data(pData);
    YushuMotor_SendControl();
}

void RmtYushuMotor_MoveCmd(void)
{
    static s8 Rmt_Y2_FNR = 0;
    static s8 Rmt_Y2_FNR_Old = 0;

    if(RmtGearValue > 10.0f)
    {
        Rmt_Y2_FNR = 1;
    }
    else if(RmtGearValue < -10.0f)
    {
        Rmt_Y2_FNR = -1;
    }
    else
    {
        Rmt_Y2_FNR = 0;
    }

    if(LIMIT_SWITCH_IN != 0)
    {
        if(((Rmt_Y2_FNR_Old == 0) && (Rmt_Y2_FNR == 1)) ||
           ((Rmt_Y2_FNR_Old == -1) && (Rmt_Y2_FNR == 1)))
        {
            SW_CLUTCH(ON);
            SendYushuMotor_MoveCmd(YUSHU_STRETCH);
        }
        else if(((Rmt_Y2_FNR_Old == 0) && (Rmt_Y2_FNR == -1)) ||
                ((Rmt_Y2_FNR_Old == 1) && (Rmt_Y2_FNR == -1)))
        {
            SW_CLUTCH(OFF);
            SendYushuMotor_MoveCmd(YUSHU_SHORTEN);
        }
        else if(((Rmt_Y2_FNR_Old == 1) && (Rmt_Y2_FNR == 0)) ||
                ((Rmt_Y2_FNR_Old == -1) && (Rmt_Y2_FNR == 0)))
        {
            SendYushuMotor_MoveCmd(YUSHU_STOP);
        }
        Rmt_Y2_FNR_Old = Rmt_Y2_FNR;
    }
    else
    {
        SendYushuMotor_MoveCmd(YUSHU_STOP);
    }
}

void YushuMotor_ControlProc(void)
{
    static MOTOR_send *pData;
    static u8 FirstFlag = 1U;
    static u8 LIMIT_SWITCH_IN_Old = 1U;
    static u8 LimitSwitchInFlg = 0U;

    if(FirstFlag == 1U)
    {
        YushuMotor_InitCmdOnce();
        FirstFlag = 0U;
    }

    if(g_ctrl_source_active == CTRL_SRC_UPPER)
    {
        YushuSpeed = g_ctrl_cmd.yushu_gear_cmd;
    }
    else
    {
        if((g_ctrl_cmd.yushu_gear_cmd > -YUSHU_RMT_GEAR_STOP_DEADBAND) &&
           (g_ctrl_cmd.yushu_gear_cmd < YUSHU_RMT_GEAR_STOP_DEADBAND))
        {
            YushuSpeed = 0.0f;
        }
        else
        {
            YushuSpeed = control_map_3point(g_ctrl_cmd.yushu_gear_cmd, Gear_Speed_Arr);
        }
    }

    if(YushuSpeed > YUSHU_SPEED_STOP_EPS)
    {
        SW_CLUTCH(ON);
    }
    else if(YushuSpeed < -YUSHU_SPEED_STOP_EPS)
    {
        SW_CLUTCH(OFF);
    }
    else
    {
        if(g_ctrl_cmd.clutch_cmd == 1U)
        {
            SW_CLUTCH(ON);
        }
        else if(g_ctrl_cmd.clutch_cmd == 0U)
        {
            SW_CLUTCH(OFF);
        }
    }

    if((YushuSpeed > -YUSHU_SPEED_STOP_EPS) && (YushuSpeed < YUSHU_SPEED_STOP_EPS))
    {
        cmd.mode = YUSHU_M8010_MODE_STOP;
        cmd.T = 0.0f;
        cmd.W = 0.0f;
        cmd.K_W = 0.0f;
    }
    else if(YushuSpeed > 0.0f)
    {
        cmd.mode = YUSHU_M8010_MODE_RUN;
        cmd.T = 0.4f;
        cmd.W = -YushuSpeed;
        cmd.K_W = 0.05f;
        LimitSwitchInFlg = 0U;
    }
    else
    {
        if((LIMIT_SWITCH_IN_Old == 1U) && (LIMIT_SWITCH_IN == 0U))
        {
            LimitSwitchInFlg = 1U;
        }

        if(LimitSwitchInFlg == 0U)
        {
            cmd.mode = YUSHU_M8010_MODE_RUN;
            cmd.T = 0.7f;
            cmd.W = -YushuSpeed;
            cmd.K_W = 0.05f;
        }
        else
        {
            cmd.mode = YUSHU_M8010_MODE_STOP;
            cmd.T = 0.0f;
            cmd.W = 0.0f;
            cmd.K_W = 0.0f;
        }
    }

    LIMIT_SWITCH_IN_Old = LIMIT_SWITCH_IN;
    pData = &cmd;
    modify_data(pData);
    YushuMotor_SendControl();
}

void RmtYushuMotor_SpeedFun(void)
{
    YushuMotor_ControlProc();
}

/******************* (C) COPYRIGHT 2024 END OF FILE *****************************/
