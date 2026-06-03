#ifndef __RING_BUF_H
#define __RING_BUF_H
#include "stm32f10x.h"
typedef struct
{
    u8 *buffer;
    u32 size;
    volatile u32 head;
    volatile u32 tail;
} ring_buf_t;
void ring_buf_init(ring_buf_t *rb, u8 *buf, u32 size);
u32 ring_buf_put(ring_buf_t *rb, const u8 *data, u32 len);
u32 ring_buf_get(ring_buf_t *rb, u8 *data, u32 len);
u32 ring_buf_available(ring_buf_t *rb);
#endif
