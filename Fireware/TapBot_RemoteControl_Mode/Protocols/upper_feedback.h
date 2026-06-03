/******************** (C) COPYRIGHT 2026 *****************************************
 * @file        upper_feedback.h
 * @brief       Upper-computer feedback frame packer.
 *********************************************************************************/
#ifndef __UPPER_FEEDBACK_H
#define __UPPER_FEEDBACK_H

#include "stm32f10x.h"

#define UPPER_FB_FRAME_HEAD             0xA5U
#define UPPER_FB_CMD_ARM_STATE          0x80U
#define UPPER_FB_CMD_ACTUATOR_ECHO      0x81U
#define UPPER_FB_CMD_DIAG               0x82U

void UpperFeedback_SendArmState(void);
void UpperFeedback_SendActuatorEcho(void);
void UpperFeedback_SendDiag(void);

#endif /* __UPPER_FEEDBACK_H */

/******************* (C) COPYRIGHT 2026 END OF FILE *****************************/
