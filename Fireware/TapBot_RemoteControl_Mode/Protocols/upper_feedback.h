/******************** (C) COPYRIGHT 2026 *****************************************
 * @file        upper_feedback.h
 * @brief       Upper-computer feedback frame packer.
 * @note        
 * @warning     
 * @license     This project is released under the MIT License.
 *********************************************************************************/
#ifndef __UPPER_FEEDBACK_H
#define __UPPER_FEEDBACK_H

#include "stm32f10x.h"

/*============================ Macro definition ====================================*/
#define UPPER_FB_FRAME_HEAD             0xA5U

#define UPPER_FB_CMD_ARM_STATE          0x80U       // Robotic arm status
#define UPPER_FB_CMD_ACTUATOR_ECHO      0x81U       // Actuator feedback
#define UPPER_FB_CMD_DIAG               0x82U       // Diagnostic information



/*============================ Function prototype ================================*/
/* Send robotic arm status frame */
void UpperFeedback_SendArmState(void);
/* Send actuator echo frame */
void UpperFeedback_SendActuatorEcho(void);
/* Send a diagnostic frame */
void UpperFeedback_SendDiag(void);




#endif /* __UPPER_FEEDBACK_H */

/******************* (C) COPYRIGHT 2026 END OF FILE *****************************/
