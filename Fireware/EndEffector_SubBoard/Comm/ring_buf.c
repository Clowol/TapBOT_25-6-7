#include "ring_buf.h"

void ring_buf_init(ring_buf_t *rb, u8 *buf, u32 size)
{
    rb->buffer = buf;
    rb->size = size;
    rb->head = 0U;
    rb->tail = 0U;
}

u32 ring_buf_available(ring_buf_t *rb)
{
    return rb->head - rb->tail;
}

u32 ring_buf_put(ring_buf_t *rb, const u8 *data, u32 len)
{
    u32 written = 0U;
    while((written < len) && (ring_buf_available(rb) < rb->size))
    {
        rb->buffer[rb->head & (rb->size - 1U)] = data[written++];
        rb->head++;
    }
    return written;
}

u32 ring_buf_get(ring_buf_t *rb, u8 *data, u32 len)
{
    u32 read = 0U;
    while((read < len) && (ring_buf_available(rb) > 0U))
    {
        data[read++] = rb->buffer[rb->tail & (rb->size - 1U)];
        rb->tail++;
    }
    return read;
}
