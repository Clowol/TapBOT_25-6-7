/******************** (C) COPYRIGHT 2026 *****************************************
 * @file        arm_auto_task.h
 * @brief       Upper-computer automatic arm workflow state machine.
 * @license     This project is released under the MIT License.
 *********************************************************************************/
#ifndef __ARM_AUTO_TASK_H
#define __ARM_AUTO_TASK_H

#include "stm32f10x.h"

#define ARM_AUTO_FLAG_PTZ          (1U << 0)
#define ARM_AUTO_FLAG_YUSHU        (1U << 1)
#define ARM_AUTO_FLAG_SERVO123     (1U << 2)
#define ARM_AUTO_FLAG_END_AUTO     (1U << 3)
#define ARM_AUTO_FLAG_RETRACT      (1U << 4)

typedef enum
{
    ARM_AUTO_STATE_IDLE = 0,
    ARM_AUTO_STATE_MOVE_TO_TARGET = 1,
    ARM_AUTO_STATE_WAIT_ARM = 2,
    ARM_AUTO_STATE_END_AUTO = 3,
    ARM_AUTO_STATE_RETRACT = 4,
    ARM_AUTO_STATE_DONE = 5,
    ARM_AUTO_STATE_ERROR = 6,
    ARM_AUTO_STATE_ABORT = 7
} arm_auto_state_t;

typedef enum
{
    ARM_AUTO_ERR_NONE = 0,
    ARM_AUTO_ERR_PARAM = 1,
    ARM_AUTO_ERR_UPPER_MODE = 2,
    ARM_AUTO_ERR_SUBBOARD_OFFLINE = 3,
    ARM_AUTO_ERR_ARM_TIMEOUT = 4,
    ARM_AUTO_ERR_END_TIMEOUT = 5,
    ARM_AUTO_ERR_SUBBOARD_ERROR = 6,
    ARM_AUTO_ERR_ENCODER_INVALID = 7
} arm_auto_error_t;

void ArmAutoTask_Init(void);
u8 ArmAutoTask_Start(const u8 *payload, u8 len);
void ArmAutoTask_Stop(void);
void ArmAutoTask_Proc10ms(void);

u8 ArmAutoTask_GetState(void);
u8 ArmAutoTask_GetError(void);
u8 ArmAutoTask_IsBusy(void);

#endif /* __ARM_AUTO_TASK_H */

/******************* (C) COPYRIGHT 2026 END OF FILE *****************************/
