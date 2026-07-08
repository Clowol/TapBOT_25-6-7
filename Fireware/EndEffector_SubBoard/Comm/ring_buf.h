/******************** (C) COPYRIGHT 2026 *****************************************
 * @file        ring_buf.h
 * @brief       Circular buffer implementation
 * 
 * @note        
 * @warning     
 * @license     This project is released under the MIT License.
 *********************************************************************************/
#ifndef __RING_BUF_H
#define __RING_BUF_H

#include "stm32f10x.h"


typedef struct
{
    u8 *buffer;             // 指向实际存储数据的数组首地址
    u32 size;               // 缓冲区总大小（字节数）
    volatile u32 head;      // 写指针
    volatile u32 tail;      // 读指针
} ring_buf_t;


/*======================= Function declaration =================================*/

void ring_buf_init(ring_buf_t *rb, u8 *buf, u32 size);

u32 ring_buf_put(ring_buf_t *rb, const u8 *data, u32 len);

u32 ring_buf_get(ring_buf_t *rb, u8 *data, u32 len);

u32 ring_buf_available(ring_buf_t *rb);


#endif


/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/
