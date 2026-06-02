/******************** (C) COPYRIGHT 2026 *****************************************
   * @author      Clomol
   * @date        2026-2027
   * @brief       串口函数头文件
   * @license     [z]本代码仅用于教学与科研目的，未经作者书面许可，不得用于商业用途
   *              This project is released under the MIT License.
   * @note        
   * @warning     
*********************************************************************************/
#ifndef __USART_H
#define	__USART_H

#include "stm32f10x.h"

#include "app_config.h"
#include "timer.h"
#include "sys.h"
#include "string.h"
#include "uart_rx_driver.h"

#include <stdio.h>
#include <stdarg.h>


#define USART1_MAX_RECV_LEN   APP_USART1_MAX_RECV_LEN
#define USART2_MAX_RECV_LEN   APP_USART2_MAX_RECV_LEN
#define USART3_MAX_RECV_LEN   APP_USART3_MAX_RECV_LEN
#define USART4_MAX_RECV_LEN   APP_USART4_MAX_RECV_LEN
#define USART5_MAX_RECV_LEN   APP_USART5_MAX_RECV_LEN

#define USART2_RX_RING_BUF_SIZE  APP_USART2_RX_RING_BUF_SIZE
#define USART3_RX_RING_BUF_SIZE  APP_USART3_RX_RING_BUF_SIZE

#define USART1_DMA_TX_BSIZE   APP_USART1_DMA_TX_BSIZE
#define USART2_DMA_TX_BSIZE   APP_USART2_DMA_TX_BSIZE
#define USART3_DMA_TX_BSIZE   APP_USART3_DMA_TX_BSIZE
#define UART4_DMA_TX_BSIZE    APP_UART4_DMA_TX_BSIZE

/***************************************************************************************************
 * @name    USARTn_MAX_RECV_LEN；   SART1_DMA_TX_BSIZE
 * @brief   接收缓冲区最大长度定义；  DMA发送缓冲区大小定义
 * @param   -len：长度;  
 * @note    RX_STA：状态标志位; RX_BUF：接收缓冲区
 ***************************************************************************************************/
extern int LEN1;
extern char USART1_RX_BUF[USART1_MAX_RECV_LEN];
extern u16 USART1_RX_STA;
extern INT8U USART1_DMA_Tx_Buf[USART1_DMA_TX_BSIZE];
extern char USART1_DMA_Rece_Buf[USART1_MAX_RECV_LEN];

extern int LEN2;
extern char USART2_RX_BUF[USART2_MAX_RECV_LEN];
extern char USART2_DMA_Rece_Buf[USART2_MAX_RECV_LEN];
extern u16 USART2_RX_STA;
extern INT8U USART2_DMA_Tx_Buf[USART2_DMA_TX_BSIZE];
extern char USART2_DMA_Rece_Buf[USART2_MAX_RECV_LEN];

extern int LEN3;
extern char USART3_RX_BUF[USART3_MAX_RECV_LEN];
extern u16 USART3_RX_STA;
extern INT8U USART3_DMA_Tx_Buf[USART3_DMA_TX_BSIZE];
extern char USART3_DMA_Rece_Buf[USART3_MAX_RECV_LEN];

extern int LEN4;
extern char USART4_RX_BUF[USART4_MAX_RECV_LEN];
extern u16 USART4_RX_STA;
extern INT8U UART4_DMA_Tx_Buf[UART4_DMA_TX_BSIZE];
extern char UART4_DMA_Rece_Buf[UART4_MAX_RECV_LEN];

extern int LEN5;
extern char USART5_RX_BUF[USART5_MAX_RECV_LEN];
extern u16 USART5_RX_STA;



/********************************** 串口1 *****************************************/
void USART1_Init(u32 bound);
void USART1_Rcc_Config(void);
void USART1_Gpio_Config(void);
void USART1_Nvic_Config(void);

int fputc(int ch, FILE *f);

void USART1_DMA_send(INT8U* buffer, u8 size);

/********************************** 串口2 *****************************************/
void USART2_Init(u32 bound);
void USART2_Rcc_Config(void);
void USART2_Gpio_Config(void);
void USART2_Nvic_Config(void);

void USART2_DMA_send(INT8U* buffer, u8 size);


/********************************** 串口3 *****************************************/
void USART3_Init(u32 bound);
void USART3_Rcc_Config(void);
void USART3_Gpio_Config(void);
void USART3_Nvic_Config(void);

void USART3_DMA_send(INT8U* buffer, u8 size);

uint32_t USART3_RxAvailable(void);
uint32_t USART3_RxRead(uint8_t *data, uint32_t len);


/********************************** 串口4 *****************************************/
void USART4_Init(u32 bound);
void USART4_Rcc_Config(void);
void USART4_Gpio_Config(void);
void USART4_Nvic_Config(void);

void UART4_DMA_send(INT8U* buffer, u8 size);


/********************************** 串口5 *****************************************/
void USART5_Init(u32 bound);
void USART5_Rcc_Config(void);
void USART5_Gpio_Config(void);
void USART5_Nvic_Config(void); 




/********************************** 串口打印函数 *********************************************/
/***************************************************************************************************
 * @name   USART_printf(USART_TypeDef* USARTx, uint8_t *Data,...)
 * @brief  格式化打印到指定串口
 ***************************************************************************************************/ 
void USART_printf(USART_TypeDef* USARTx, uint8_t *Data,...);


/***************************************************************************************************
 * @name   prtData(char *ary, INT8U len)
 * @brief  发送字节数组
 ***************************************************************************************************/ 
void prtData(char *ary, INT8U len);



#endif /* __USART_H */

/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/
