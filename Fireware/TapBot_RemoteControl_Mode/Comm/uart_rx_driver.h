/******************** (C) COPYRIGHT 2026 *****************************************
 * @file        uart_rx_driver.h
 * @brief       UART RX driver data structures and function prototypes.
 * @note        
 * @warning     
 * @license     This project is released under the MIT License.
 *********************************************************************************/
#ifndef __UART_RX_DRIVER_H
#define __UART_RX_DRIVER_H

#include "stm32f10x.h"

/*************************************************************************
 * @brief UART 接收端口枚举
 * 
 * 用于标识不同的 USART 外设，驱动内部使用该枚举索引环形缓冲区数组。
 ***************************************************************************/
typedef enum
{
	UART_RX_PORT_USART2 = 0,
	UART_RX_PORT_USART3,
	UART_RX_PORT_COUNT
} uart_rx_port_t;


/*********************************************************************************
 * @brief   初始化指定 UART 端口的接收环形缓冲区
 * @param   port 要初始化的端口（UART_RX_PORT_USART2 或 UART_RX_PORT_USART3）
 * @note    该函数必须在任何收发操作前调用。每个端口的缓冲区大小在 app_config.h 中定义。
 *          若传入无效端口，函数不做任何处理。
 ********************************************************************************/
void uart_rx_driver_init(uart_rx_port_t port);

/*********************************************************************************
 * @brief   向指定端口的环形缓冲区写入数据（生产者接口）
 * @param   port 目标端口
 * @param   data 待写入数据的源缓冲区指针
 * @param   len  请求写入的字节数
 * @return  实际写入的字节数（可能小于 len，若缓冲区剩余空间不足）
 * @note    该函数通常由 UART 接收中断（或 DMA 传输完成中断）调用。
 *          若端口无效或未初始化，返回 0。
 ********************************************************************************/
uint32_t uart_rx_driver_put(uart_rx_port_t port, const uint8_t *data, uint32_t len);

/*********************************************************************************
 * @brief   查询指定端口环形缓冲区中当前可读的字节数
 * @param   port 目标端口
 * @return  可读字节数（若端口无效或未初始化，返回 0）
 * @note    该函数用于主循环或数据消费任务轮询，判断是否有新数据到达。
 ********************************************************************************/
uint32_t uart_rx_driver_available(uart_rx_port_t port);

/*********************************************************************************
 * @brief   从指定端口的环形缓冲区读取数据（消费者接口）
 * @param   port 源端口
 * @param   data 存储读出数据的目标缓冲区指针
 * @param   len  请求读取的字节数
 * @return  实际读取的字节数（可能小于 len，若缓冲区中有效数据不足）
 * @note    该函数通常在主循环或数据解析任务中调用。读取后数据从缓冲区移除。
 *          若端口无效或未初始化，返回 0。
 ********************************************************************************/
uint32_t uart_rx_driver_read(uart_rx_port_t port, uint8_t *data, uint32_t len);


uint32_t USART2_RxAvailable(void);
uint32_t USART2_RxRead(uint8_t *data, uint32_t len);


uint32_t USART3_RxAvailable(void);
uint32_t USART3_RxRead(uint8_t *data, uint32_t len);


#endif /* __UART_RX_DRIVER_H */

/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/
