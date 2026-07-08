/******************** (C) COPYRIGHT 2026 *****************************************
 * @file        can2.h
 * @brief       CAN2 driver interface.
 *********************************************************************************/
#ifndef __CAN2_H
#define __CAN2_H

#include "stm32f10x.h"
#include "app_config.h"

extern u8 CAN2_SendMsg1[8];
extern u8 canrxbuf2[8];

void USER_CAN2_Init(void);
void CAN2_NVIC_Config(void);
void CAN2_NVIC_Configuration(void);
void CAN2_GPIO_Config(void);
void CAN2_Init_Config(void);
void CAN2_Filter_Config(void);
u8 Can2_Send_Msg(u8 *msg, u32 id, u8 dlc);
void CAN2_Rx_Cam_Proce(void);

#endif /* __CAN2_H */


/******************* (C) COPYRIGHT 2026 END OF FILE *****************************/
