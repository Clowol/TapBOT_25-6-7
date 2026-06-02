/******************** (C) COPYRIGHT 2026 *****************************************
 * @author      Clomol
 * @date        2025-2026
 * @brief       [z]STM32F107 开发板的引脚定义、外设配置及相关常量
 *              Board configuration
 * 
 * @license     [z]本代码仅用于教学与科研目的，未经作者书面许可，不得用于商业用途
 *              This project is released under the MIT License.
 * @note        [z]使用本代码时请保留此版权声明
 *              Please retain this copyright notice when using this code
 * @warning     [z]本人能力有限，这段代码可能存在错误或不完善之处，使用前请仔细检查并测试
 *              I am not responsible for any damage or loss caused by using this code. Please review             
*********************************************************************************/

#ifndef __BOARD_CONFIG_H
#define __BOARD_CONFIG_H

#include "stm32f10x.h"

/*==============================================================================
 * 1. 位带操作宏（基于 Cortex-M3 位带别名区）
 *    具体实现思想,参考<<CM3权威指南>>第五章(87页~92页).M4同M3类似,只是寄存器地址变了.
 *============================================================================*/
#define BITBAND(addr, bitnum) \
    (((addr) & 0xF0000000) + 0x2000000 + (((addr) & 0xFFFFF) << 5) + ((bitnum) << 2))
#define MEM_ADDR(addr)       (*(volatile unsigned long *)(addr))
#define BIT_ADDR(addr, bitnum) MEM_ADDR(BITBAND(addr, bitnum))



/*==============================================================================
 * 2. GPIO 输出/输入数据寄存器地址（基于标准库 GPIOx_BASE）
 *    注：STM32F107 最多支持 GPIOA~GPIOG，此处保留到 GPIOK 以便代码兼容
 *============================================================================*/
/* 输出寄存器 ODR 地址 */
#define GPIOA_ODR_Addr   (GPIOA_BASE + 0x14) //0x40020014
#define GPIOB_ODR_Addr   (GPIOB_BASE + 0x14) //0x40020414
#define GPIOC_ODR_Addr   (GPIOC_BASE + 0x14) //0x40020814
#define GPIOD_ODR_Addr   (GPIOD_BASE + 0x14) //0x40020C14
#define GPIOE_ODR_Addr   (GPIOE_BASE + 0x14) //0x40021014
#define GPIOF_ODR_Addr   (GPIOF_BASE + 0x14) //0x40021414
#define GPIOG_ODR_Addr   (GPIOG_BASE + 0x14) //0x40021814
#define GPIOH_ODR_Addr   (GPIOH_BASE + 0x14) //0x40021C14
#define GPIOI_ODR_Addr   (GPIOI_BASE + 0x14) //0x40022014
#define GPIOJ_ODR_Addr   (GPIOJ_BASE + 0x14) //0x40022414
#define GPIOK_ODR_Addr   (GPIOK_BASE + 0x14) //0x40022814

/* 输入寄存器 IDR 地址 */
#define GPIOA_IDR_Addr   (GPIOA_BASE + 0x10) //0x40020010
#define GPIOB_IDR_Addr   (GPIOB_BASE + 0x10) //0x40020410
#define GPIOC_IDR_Addr   (GPIOC_BASE + 0x10) //0x40020810
#define GPIOD_IDR_Addr   (GPIOD_BASE + 0x10) //0x40020C10
#define GPIOE_IDR_Addr   (GPIOE_BASE + 0x10) //0x40021010
#define GPIOF_IDR_Addr   (GPIOF_BASE + 0x10) //0x40021410
#define GPIOG_IDR_Addr   (GPIOG_BASE + 0x10) //0x40021810
#define GPIOH_IDR_Addr   (GPIOH_BASE + 0x10) //0x40021C10
#define GPIOI_IDR_Addr   (GPIOI_BASE + 0x10) //0x40022010
#define GPIOJ_IDR_Addr   (GPIOJ_BASE + 0x10) //0x40022410
#define GPIOK_IDR_Addr   (GPIOK_BASE + 0x10) //0x40022810



/*==============================================================================
 * 3. 端口位操作快捷宏（类似 51 单片机的 sbit）
 *    用法：PAout(0) = 1;   value = PAin(1);
 *    注意：n 必须是 0~15 的常量 [ 确保n的值小于16! ]
 *============================================================================*/
#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr, n)
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr, n)

#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr, n)
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr, n)

#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr, n)
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr, n)

#define PDout(n)   BIT_ADDR(GPIOD_ODR_Addr, n)
#define PDin(n)    BIT_ADDR(GPIOD_IDR_Addr, n)

#define PEout(n)   BIT_ADDR(GPIOE_ODR_Addr, n)
#define PEin(n)    BIT_ADDR(GPIOE_IDR_Addr, n)

#define PFout(n)   BIT_ADDR(GPIOF_ODR_Addr, n)
#define PFin(n)    BIT_ADDR(GPIOF_IDR_Addr, n)

#define PGout(n)   BIT_ADDR(GPIOG_ODR_Addr, n)
#define PGin(n)    BIT_ADDR(GPIOG_IDR_Addr, n)

#define PHout(n)   BIT_ADDR(GPIOH_ODR_Addr, n)
#define PHin(n)    BIT_ADDR(GPIOH_IDR_Addr, n)

#define PIout(n)   BIT_ADDR(GPIOI_ODR_Addr, n)
#define PIin(n)    BIT_ADDR(GPIOI_IDR_Addr, n)

#define PJout(n)   BIT_ADDR(GPIOJ_ODR_Addr, n)
#define PJin(n)    BIT_ADDR(GPIOJ_IDR_Addr, n)

#define PKout(n)   BIT_ADDR(GPIOK_ODR_Addr, n)
#define PKin(n)    BIT_ADDR(GPIOK_IDR_Addr, n)



/*==============================================================================
 * 4. 系统级初始化声明（中断优先级配置）
 *============================================================================*/
void NVIC_Configuration(void);

#endif /* __BOARD_CONFIG_H */


/******************* (C) COPYRIGHT 2026 END OF FILE *****************************/
