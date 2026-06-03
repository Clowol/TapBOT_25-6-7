/******************** (C) COPYRIGHT 2026 *****************************************
 * @file    end_effector_task.h
 * @brief   Automatic end-effector task state machine.
 *********************************************************************************/
#ifndef __END_EFFECTOR_TASK_H
#define __END_EFFECTOR_TASK_H

#include "stm32f10x.h"

typedef enum
{
    END_STATE_IDLE = 0,
    END_STATE_REMOTE_SPEED,
    END_STATE_UPPER_POSITION,
    END_STATE_PUSHROD_EXTEND,
    END_STATE_PRESS_HOLD,
    END_STATE_SERVO_FORWARD,
    END_STATE_SERVO_REVERSE,
    END_STATE_WAIT_OBJECT_CLEAR,
    END_STATE_PUSHROD_RETRACT,
    END_STATE_DONE,
    END_STATE_ERROR,
    END_STATE_ESTOP
} end_state_t;

void EndEffectorTask_Init(void);
void EndEffectorTask_Proc(u16 period_ms);
void EndEffectorTask_SetMode(u8 mode);
void EndEffectorTask_SetServo123Speed(s16 speed0, s16 speed1, s16 speed2);
void EndEffectorTask_SetServo123Position(s16 pos0, s16 pos1, s16 pos2, u16 run_time, u16 speed);
void EndEffectorTask_StartAuto(u16 forward_ms, u16 reverse_ms, s16 speed);
void EndEffectorTask_StopAuto(void);
void EndEffectorTask_EStop(void);
u8 EndEffectorTask_GetState(void);
u8 EndEffectorTask_GetError(void);

#endif /* __END_EFFECTOR_TASK_H */
