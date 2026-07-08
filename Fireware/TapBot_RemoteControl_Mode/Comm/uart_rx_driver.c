/******************** (C) COPYRIGHT 2026 *****************************************
 * @file        uart_rx_driver.c
 * @brief       UART RX byte-stream buffer driver backed by a ring buffer.
 * @note        
 * @warning     
 * @license     This project is released under the MIT License.
 *********************************************************************************/
#include "uart_rx_driver.h"
#include "app_config.h"
#include "ring_buf.h"

/* 静态分配 USART2 和 USART3 的接收环形缓冲区内存 */
static uint8_t USART2_RxRingBuffer[APP_USART2_RX_RING_BUF_SIZE];
static uint8_t USART3_RxRingBuffer[APP_USART3_RX_RING_BUF_SIZE];

/* 环形缓冲区对象数组，索引对应 uart_rx_port_t 枚举值 */
static ring_buf_t UartRxRing[UART_RX_PORT_COUNT];

/*************************************************************************
 * @brief   获取指定端口对应的环形缓冲区指针
 * @param   port UART端口（UART_RX_PORT_USART2 / UART_RX_PORT_USART3）
 * @return  成功返回 ring_buf_t 指针，端口无效返回 NULL
 ************************************************************************/
static ring_buf_t *uart_rx_get_ring(uart_rx_port_t port)
{
	if(port >= UART_RX_PORT_COUNT)
	{
		return 0;
	}
	return &UartRxRing[port];
}


/*********************************************************************************
 * @brief   初始化指定 UART 端口的接收环形缓冲区
 * @param   port 要初始化的端口（UART_RX_PORT_USART2 或 UART_RX_PORT_USART3）
 * @note    该函数必须在任何收发操作前调用。每个端口的缓冲区大小在 app_config.h 中定义。
 *          若端口无效或缓冲区大小不符合2的幂，ring_buf_init 会将 size 置0，后续操作将返回0。
 ********************************************************************************/
void uart_rx_driver_init(uart_rx_port_t port)
{
	switch(port)
	{
		case UART_RX_PORT_USART2:
			ring_buf_init(&UartRxRing[port], USART2_RxRingBuffer, APP_USART2_RX_RING_BUF_SIZE);
			break;
		case UART_RX_PORT_USART3:
			ring_buf_init(&UartRxRing[port], USART3_RxRingBuffer, APP_USART3_RX_RING_BUF_SIZE);
			break;
		default:
			break;
	}
}


/*********************************************************************************
 * @brief   向指定端口的环形缓冲区写入数据（生产者接口）
 * @param   port 目标端口
 * @param   data 待写入数据的源缓冲区指针
 * @param   len  请求写入的字节数
 * @return  实际写入的字节数（可能小于 len，若缓冲区剩余空间不足）
 * @note    该函数通常由 UART 接收中断（或 DMA 传输完成中断）调用。
 *          若端口无效或未初始化，返回 0。
 ********************************************************************************/
uint32_t uart_rx_driver_put(uart_rx_port_t port, const uint8_t *data, uint32_t len)
{
	ring_buf_t *rb = uart_rx_get_ring(port);
	return ring_buf_put(rb, data, len);
}

/*********************************************************************************
 * @brief   查询指定端口环形缓冲区中当前可读的字节数
 * @param   port 目标端口
 * @return  可读字节数（若端口无效或未初始化，返回 0）
 * @note    该函数用于主循环或数据消费任务轮询，判断是否有新数据到达。
 ********************************************************************************/
uint32_t uart_rx_driver_available(uart_rx_port_t port)
{
	ring_buf_t *rb = uart_rx_get_ring(port);
	return ring_buf_available(rb);
}

/*********************************************************************************
 * @brief   从指定端口的环形缓冲区读取数据（消费者接口）
 * @param   port 源端口
 * @param   data 存储读出数据的目标缓冲区指针
 * @param   len  请求读取的字节数
 * @return  实际读取的字节数（可能小于 len，若缓冲区中有效数据不足）
 * @note    该函数通常在主循环或数据解析任务中调用。读取后数据从缓冲区移除。
 *          若端口无效或未初始化，返回 0。
 ********************************************************************************/
uint32_t uart_rx_driver_read(uart_rx_port_t port, uint8_t *data, uint32_t len)
{
	ring_buf_t *rb = uart_rx_get_ring(port);
	return ring_buf_get(rb, data, len);
}



/***********************************************************************/ 
uint32_t USART2_RxAvailable(void)
{
	return uart_rx_driver_available(UART_RX_PORT_USART2);
}

uint32_t USART2_RxRead(uint8_t *data, uint32_t len)
{
	return uart_rx_driver_read(UART_RX_PORT_USART2, data, len);
}

uint32_t USART3_RxAvailable(void)
{
	return uart_rx_driver_available(UART_RX_PORT_USART3);
}

uint32_t USART3_RxRead(uint8_t *data, uint32_t len)
{
	return uart_rx_driver_read(UART_RX_PORT_USART3, data, len);
}



/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/
