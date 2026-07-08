/******************** (C) COPYRIGHT 2026 *****************************************
 * @file        project_macros.h
 * @brief       General-purpose macros, type aliases, and utility functions.
 * @note        
 * @warning     
 * @license     This project is released under the MIT License.
 *********************************************************************************/
#ifndef __PROJECT_MACROS_H
#define __PROJECT_MACROS_H

#include "stm32f10x.h" 
#include "string.h"
#include "stdio.h"

/*==============================================================================
 * 1. 补充标准库未定义的有符号类型别名（标准库已提供 u8/u16/u32 等无符号类型）
 *============================================================================*/
// 带符号整数
typedef int32_t  s32;
typedef int16_t  s16;
typedef int8_t   s8;

typedef const int32_t sc32;
typedef const int16_t sc16;
typedef const int8_t  sc8;

typedef __IO int32_t  vs32;
typedef __IO int16_t  vs16;
typedef __IO int8_t   vs8;

typedef __I int32_t   vsc32;
typedef __I int16_t   vsc16;
typedef __I int8_t    vsc8;

// 无符号整数
typedef const uint32_t uc32;
typedef const uint16_t uc16;
typedef const uint8_t  uc8;

typedef __IO uint32_t	vu32;
typedef __IO uint16_t	vu16;
typedef __IO uint8_t    vu8;

typedef __I uint32_t  vuc32;
typedef __I uint16_t  vuc16;
typedef __I uint8_t   vuc8;

/* 布尔及传统命名（无冲突） */
typedef unsigned char  	        BOOLEAN; 		 /*  布尔变量                    */
typedef unsigned char  	        INT8U;           /*  无符号8位整型变量           */
typedef signed   char  	        INT8S;           /*  有符号8位整型变量           */

typedef unsigned short 	        INT16U;          /*  无符号16位整型变量          */
typedef signed   short 	        INT16S;          /*  有符号16位整型变量          */

typedef unsigned long  	        INT32U;          /*  无符号32位整型变量          */
typedef volatile unsigned int   INT32UV;         /*  无符号32位整型变量          */ 
typedef signed   long  	        INT32S;          /*  有符号32位整型变量          */

typedef          float          FP32;            /*  单精度浮点数（32位长度）    */
typedef          double         FP64;            /*  双精度浮点数（64位长度）    */

typedef unsigned char  	        uchar;           /*  无符号8位整型变量           */
typedef unsigned int  	        uint;            /*  无符号16位整型变量          */
/* 不同平台上的long长度可能不同，用unsigned int可更严格地限定为 32 位*/

/* 布尔值 */
#define false   0
#define true    1



/*==============================================================================
 * 2. 通用位操作宏 
 *============================================================================*/
#define BIT0   (0x01)
#define BIT1   (0x02)
#define BIT2   (0x04)
#define BIT3   (0x08)
#define BIT4   (0x10)
#define BIT5   (0x20)
#define BIT6   (0x40)
#define BIT7   (0x80)



/*==============================================================================
 * 3. 字节/半字提取宏--> 指针操作   [字节拆分宏] 
 *============================================================================*/
#define BYTE0(dwTemp)   (*(uint8_t *)(&(dwTemp)))           /* 取低八位 */
#define BYTE1(dwTemp)   (*((uint8_t *)(&(dwTemp)) + 1))     /* 取高八位 */
#define BYTE2(dwTemp)   (*((uint8_t *)(&(dwTemp)) + 2))     /* 取高十六位 */
#define BYTE3(dwTemp)   (*((uint8_t *)(&(dwTemp)) + 3))     /* 取高三十二位 */



/*==============================================================================
 * 4. 调试打印工具（需要外部提供串口输出函数）
 *============================================================================*/
extern char str[100];                               /* 全局格式化缓冲区，使用前注意重入问题 */

void swgPtn(char *str);                             /* 默认串口输出单个字符（例如 USART1） */
void swgPtnUx(USART_TypeDef* USARTx, char *str);    /* 指定串口输出单个字符 */

/* 带格式化的打印宏 */
#define swgPrt(format, ...) \
    do { \
        sprintf(str, format, ##__VA_ARGS__); \
        swgPtn(str); \
    } while(0)

#define swgPrtUx(USARTx, format, ...) \
    do { \
        sprintf(str, format, ##__VA_ARGS__); \
        swgPtnUx(USARTx, str); \
    } while(0)

/* 原始数据打印（十六进制） */
void swgPrtData(uint8_t *ary, INT8U len);
void swgPrtDataUx(USART_TypeDef* USARTx, uint8_t *ary, INT8U len);



/*==============================================================================
 * 5. 常用辅助函数
 *============================================================================*/
uint8_t  HBT(uint16_t a);                    /* 取16位的高8位 */
uint8_t  LBT(uint16_t b);                    /* 取16位的低8位 */
uint CheckCrc_16bit(uchar *buf, uint len) ;
uint CheckCrc_8bit(uchar *buf, uint len);

#endif /* __PROJECT_MACROS_H */


/******************* (C) COPYRIGHT 2026 END OF FILE *****************************/

