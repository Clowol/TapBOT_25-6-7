/******************** (C) COPYRIGHT 2026 *****************************************
 * @file    subboard_link.h
 * @brief   UART5/RS485 link layer for the end-effector sub-board.
 * @note        
 * @warning     
 * @license     This project is released under the MIT License.
 *********************************************************************************/
#ifndef __SUBBOARD_LINK_H
#define __SUBBOARD_LINK_H

#include "stm32f10x.h"

/*=========================  Macro definition =================================*/
#define SUBBOARD_RX_RING_SIZE      128U
#define SUBBOARD_FRAME_MAX_PAYLOAD 32U


/*=========================  function  =================================*/
void SubBoard_LinkInit(void);
void SubBoard_LinkOnRxByte(u8 data);
void SubBoard_LinkRs485DirInit(void);
void SubBoard_LinkRs485SetRx(void);
void SubBoard_LinkRs485SetTx(void);

void SubBoard_LinkProc(void);
void SubBoard_LinkTick10ms(void);

u8 SubBoard_LinkSendFrame(u8 cmd_id, const u8 *payload, u8 len);

u8 SubBoard_LinkIsOnline(void);

u8 SubBoard_LinkGetLastState(void);
u8 SubBoard_LinkGetLastError(void);
u8 SubBoard_LinkGetLastAckCmd(void);

void SubBoard_LinkSetStatus(u8 state, u8 error);
void SubBoard_LinkSetAck(u8 state, u8 status, u8 ack_cmd);

#endif /* __SUBBOARD_LINK_H */

/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/
