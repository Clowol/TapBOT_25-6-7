/******************** (C) COPYRIGHT 2026 *****************************************
 * @file        wit_imu.h
 * @brief       WitMotion HWT905CAN IMU interface for PTZ feedback.
 *********************************************************************************/
#ifndef __WIT_IMU_H
#define __WIT_IMU_H

#include "stm32f10x.h"

typedef struct
{
    s16 roll_deg_x100;
    s16 pitch_deg_x100;
    s16 yaw_deg_x100;
    u8 valid;
} wit_imu_feedback_t;

void WitImu_Init(void);
void WitImu_OnCanFrame(u32 std_id, const u8 *data, u8 len);
const wit_imu_feedback_t *WitImu_GetFeedback(void);

#endif /* __WIT_IMU_H */

/******************* (C) COPYRIGHT 2026 END OF FILE *****************************/
