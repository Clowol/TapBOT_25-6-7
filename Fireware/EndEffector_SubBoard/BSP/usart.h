#ifndef __SUB_UART_H
#define __SUB_UART_H
#include "stm32f10x.h"
void SubUart_Init(u32 baud);
void SubUart_SendByte(u8 data);
#endif
