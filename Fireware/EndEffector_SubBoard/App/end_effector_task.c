/******************** (C) COPYRIGHT 2026 *****************************************
 * @file    end_effector_task.c
 * @brief   End-effector automatic task and manual command arbitration.
 *********************************************************************************/
#include "end_effector_task.h"
#include "app_config.h"
#include "feetech_servo.h"
#include "pushrod.h"
#include "gpio.h"
#include "subboard_protocol.h"

static end_state_t EndState;
static u8 EndError;
static u16 StateTicks;              // 当前状态已持续的时间
static u16 AutoForwardMs;
static u16 AutoReverseMs;
static s16 AutoRotateSpeed;

static void EndEffectorTask_SetState(end_state_t state)
{
    EndState = state;
    StateTicks = 0U;
    SubProtocol_SendState((u8)EndState, EndError);
}

void EndEffectorTask_Init(void)
{
    EndState = END_STATE_IDLE;
    EndError = 0U;
    StateTicks = 0U;
    AutoForwardMs = 0U;
    AutoReverseMs = 0U;
    AutoRotateSpeed = 0;
    FeetechServo_Init();
    Pushrod_Stop();
}

void EndEffectorTask_SetMode(u8 mode)
{
    if(mode == 0U)
    {
        EndEffectorTask_SetState(END_STATE_REMOTE_SPEED);
    }
    else if(mode == 1U)
    {
        FeetechServo_Stop123();
        EndEffectorTask_SetState(END_STATE_UPPER_POSITION);
    }
    else if(mode == 2U)
    {
        EndEffectorTask_SetState(END_STATE_IDLE);
    }
    else
    {
        EndEffectorTask_EStop();
    }
}

void EndEffectorTask_SetServo123Speed(s16 speed0, s16 speed1, s16 speed2)
{
    if(EndState == END_STATE_ESTOP)
    {
        return;
    }
    FeetechServo_SetModeSpeed123();
    FeetechServo_SetSpeed123(speed0, speed1, speed2);
    EndState = END_STATE_REMOTE_SPEED;
}

void EndEffectorTask_SetServo123Position(s16 pos0, s16 pos1, s16 pos2, u16 run_time, u16 speed)
{
    if(EndState == END_STATE_ESTOP)
    {
        return;
    }
    FeetechServo_SetModePosition123();
    FeetechServo_SetPosition123(pos0, pos1, pos2, run_time, speed);
    EndState = END_STATE_UPPER_POSITION;
}

void EndEffectorTask_StartAuto(u16 forward_ms, u16 reverse_ms, s16 speed)
{
    if(EndState == END_STATE_ESTOP)
    {
        return;
    }
    AutoForwardMs = forward_ms;
    AutoReverseMs = reverse_ms;
    AutoRotateSpeed = speed;
    EndEffectorTask_SetState(END_STATE_WAIT_OBJECT);
}

void EndEffectorTask_StopAuto(void)
{
    FeetechServo_SetRotateSpeed(0);
    Pushrod_Stop();
    EndEffectorTask_SetState(END_STATE_IDLE);
}

void EndEffectorTask_EStop(void)
{
    FeetechServo_StopAll();
    Pushrod_Stop();
    EndEffectorTask_SetState(END_STATE_ESTOP);
}

void EndEffectorTask_Proc(u16 period_ms)
{
    StateTicks += period_ms;

    switch(EndState)
    {
        case END_STATE_WAIT_OBJECT:
            Pushrod_Stop();
            FeetechServo_SetRotateSpeed(0);
            if(EndGpio_ObjectDetected() != 0U)
            {
                EndEffectorTask_SetState(END_STATE_PUSHROD_EXTEND);
            }
            else if(StateTicks >= END_AUTO_OBJECT_TIMEOUT_MS)
            {
                EndError = 4U;
                EndEffectorTask_SetState(END_STATE_ERROR);
            }
            break;

        case END_STATE_PUSHROD_EXTEND:
            Pushrod_Extend();
            if(StateTicks >= APP_CONTROL_PERIOD_MS)
            {
                EndEffectorTask_SetState(END_STATE_PRESS_HOLD);
            }
            break;

        case END_STATE_PRESS_HOLD:
            if(StateTicks >= END_AUTO_PRESS_HOLD_MS)
            {
                FeetechServo_SetRotateSpeed(AutoRotateSpeed);
                EndEffectorTask_SetState(END_STATE_SERVO_FORWARD);
            }
            break;

        case END_STATE_SERVO_FORWARD:
            if(StateTicks >= AutoForwardMs)
            {
                FeetechServo_SetRotateSpeed((s16)-AutoRotateSpeed);
                EndEffectorTask_SetState(END_STATE_SERVO_REVERSE);
            }
            break;

        case END_STATE_SERVO_REVERSE:
            if((EndGpio_ObjectDetected() == 0U) || (StateTicks >= AutoReverseMs))
            {
                FeetechServo_SetRotateSpeed(0);
                EndEffectorTask_SetState(END_STATE_PUSHROD_RETRACT);
            }
            break;

        case END_STATE_PUSHROD_RETRACT:
            Pushrod_Retract();
            if(StateTicks >= END_AUTO_PUSHROD_TIMEOUT_MS)
            {
                Pushrod_Stop();
                EndEffectorTask_SetState(END_STATE_DONE);
            }
            break;

        case END_STATE_DONE:
            EndEffectorTask_SetState(END_STATE_IDLE);
            break;

        default:
            break;
    }
}

u8 EndEffectorTask_GetState(void)
{
    return (u8)EndState;
}

u8 EndEffectorTask_GetError(void)
{
    return EndError;
}
