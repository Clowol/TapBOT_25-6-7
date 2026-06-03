#ifndef __RS485_LINK_H
#define __RS485_LINK_H
#include "stm32f10x.h"
void Rs485Link_Init(void);
void Rs485Link_OnRxByte(u8 data);
void Rs485Link_Proc(void);
u8 Rs485Link_SendFrame(u8 cmd_id, const u8 *payload, u8 len);
#endif
