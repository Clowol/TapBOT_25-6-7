/******************** (C) COPYRIGHT 2026 *****************************************
 * @file        ring_buf.c
 * @brief       Ring buffer implementation for UART DMA/IDLE byte-stream buffering.
 *********************************************************************************/
#include "ring_buf.h"


/*********************************************************************
 * @brief 检查一个数是否为 2 的幂
 * @param value 待检查的值
 * @return 1 是 2 的幂，0 不是（或 value == 0）
 ********************************************************************/
static uint32_t ring_buf_is_power_of_2(uint32_t value)
{
	return ((value != 0U) && ((value & (value - 1U)) == 0U));
}



/**********************************************************************
 * @brief   初始化环形缓冲区
 * @param   rb   指向环形缓冲区结构体的指针
 * @param   buf  指向外部静态分配的数据缓冲区的指针（大小至少为 size 字节）
 * @param   size 缓冲区大小，必须是2的幂
 * @note    如果 rb 或 buf 为空，或者 size 不是 2 的幂，
 *          则 rb->size 被设为 0，表示未就绪。
 *          初始化后 head = tail = 0。
 *************************************************************************/
void ring_buf_init(ring_buf_t *rb, uint8_t *buf, uint32_t size)
{
	if (rb == 0)             // 结构体指针无效
	{
		return;
	}

	rb->buffer = buf;
	rb->head = 0U;
	rb->tail = 0U;

	if ((buf == 0) || (ring_buf_is_power_of_2(size) == 0U))
	{
		rb->size = 0U;
		return;
	}

	rb->size = size;
}


/***********************************************************************
 * @brief   查询环形缓冲区中的有效数据字节数
 * @param   rb 指向环形缓冲区结构体的指针
 * @return  缓冲区中当前可读取的字节数（head - tail 的环形差值）
 * @note    该函数返回的是即时值，在多任务环境中调用后，实际可用数可能因
 *          并发的写入操作而变化。适用于轮询或非临界判断.
 **********************************************************************/
uint32_t ring_buf_available(ring_buf_t *rb)
{
	if ((rb == 0) || (rb->size == 0U))
	{
		return 0U;
	}

	return rb->head - rb->tail;
}



/**********************************************************************
 * @brief   向环形缓冲区写入数据（生产者接口）
 * @param   rb   指向环形缓冲区结构体的指针
 * @param   data 待写入数据的源缓冲区指针
 * @param   len  请求写入的字节数
 * @return  实际写入的字节数（可能小于 len，当缓冲区剩余空间不足时）
 * @note    该函数通常在中断服务程序或数据接收线程中调用。
 *          若剩余空间不足，则会写入部分数据或返回0。调用者应检查返回值。
 *          支持 len=0，此时返回0。
 *********************************************************************/
uint32_t ring_buf_put(ring_buf_t *rb, const uint8_t *data, uint32_t len)
{
	uint32_t head;
	uint32_t tail;
	uint32_t free_len;
	uint32_t write_len;
	uint32_t mask;
	uint32_t i;

	if ((rb == 0) || (data == 0) || (rb->buffer == 0) || (rb->size == 0U))
	{
		return 0U;
	}

    /* 获取当前 head 和 tail 的本地拷贝（避免在循环中反复读取 volatile） */
	head = rb->head;
	tail = rb->tail;

    /* 剩余空闲空间 = 总大小 - (head - tail) */
	free_len = rb->size - (head - tail);

    /* 实际可写入的长度 = min(len, free_len) */
	write_len = (len < free_len) ? len : free_len;

    /* 位掩码 = size - 1，用于快速取模：index & mask 等价于 index % size */
	mask = rb->size - 1U;

    /* 循环写入数据，使用位掩码计算实际存放位置 */
	for (i = 0U; i < write_len; i++)
	{
		rb->buffer[(head + i) & mask] = data[i];
	}

	rb->head = head + write_len;

	return write_len;
}


/***********************************************************************
 * @brief   从环形缓冲区读取数据（消费者接口）
 * @param   rb   指向环形缓冲区结构体的指针
 * @param   data 存储读出数据的目标缓冲区指针
 * @param   len  请求读取的字节数
 * @return  实际读取的字节数（可能小于 len，当缓冲区中有效数据不足时）
 * @note    该函数通常在主循环或数据消费线程中调用。
 *          若有效数据不足，则会读取所有可用数据并返回实际读取字节数。
 *          支持 len=0，此时返回0。
 **********************************************************************/
uint32_t ring_buf_get(ring_buf_t *rb, uint8_t *data, uint32_t len)
{
	uint32_t head;
	uint32_t tail;
	uint32_t available;
	uint32_t read_len;
	uint32_t mask;
	uint32_t i;

	if ((rb == 0) || (data == 0) || (rb->buffer == 0) || (rb->size == 0U))
	{
		return 0U;
	}

	head = rb->head;
	tail = rb->tail;
	available = head - tail;
	read_len = (len < available) ? len : available;
	mask = rb->size - 1U;

	for (i = 0U; i < read_len; i++)
	{
		data[i] = rb->buffer[(tail + i) & mask];
	}

	rb->tail = tail + read_len;

	return read_len;
}



/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/
