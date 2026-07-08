/******************** (C) COPYRIGHT 2026 *****************************************
 * @file        delay.c
 * @brief       SysTick-based microsecond and millisecond delay functions.
 * @note        
 * @warning     
 * @license     This project is released under the MIT License.
 *********************************************************************************/
#include "delay.h"

static u8  fac_us = 0;						//us延时倍乘数
static u16 fac_ms = 0;						//ms延时倍乘数


/*******************************************************************
  * @file	    delay_init(uint8_t SYSCLK)
  * @brief  	延迟函数初始化 		
  * @param      -SYSCLK:系统时钟
  *             [SYSTICK的时钟固定为HCLK时钟的1/8]
  * @retval 	无
  ******************************************************************/  
void delay_init(u8 SYSCLK)
{
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);	//选择外部时钟  HCLK/8
	//SysTick->CTRL&=0xfffffffb;		
    //bit2清空,选择外部时钟  HCLK/8[直接操作寄存器的等效写法]
	fac_us = SYSCLK/8;		    
	fac_ms = (u16)fac_us*1000;
}



/*******************************************************************	    								     
  * @file		  delay_us(uint32_t nus)
  * @brief  	微秒级延时 
  * @param     -nus 延时时长 	范围：0~233015    								   
  * @retval 	无
  ******************************************************************/   		    								   
void delay_us(u32 nus)
{		
	u32 temp;	    	 
	SysTick->LOAD = nus * fac_us; 		// 时间加载	  		 
	SysTick->VAL = 0x00;        		// 清空计数器
	SysTick->CTRL = 0x01;      		    // 开始倒数

	do
	{
		temp = SysTick->CTRL;
	}
	while(temp&0x01 && !(temp&(1<<16)));	// 等待时间到达  ( 定时器运行中\时间未到；)

	SysTick->CTRL = 0x00;       		// 关闭计数器
	SysTick->VAL = 0x00;       			// 清空计数器	 
}



/*******************************************************************
  * @file		  delay_ms(u16 nms)
  * @brief  	毫秒级延时 
  * @param      -nms 延时时长 范围：0~1864		
  *             [fac_ms = SYSCLK / 8 / 1000]
  * @note		  (SysTick->LOAD为24位寄存器)
  *             ->(最大延时为:nms<=0xffffff*8*1000/SYSCLK）  
  *              SYSCLK单位为Hz,nms单位为ms, 在72M条件下,nms<=1864
  * @retval 	 无
  ******************************************************************/  								    
void delay_ms(u16 nms)
{	 		  	  
	u32 temp;		   
	SysTick->LOAD = (u32)nms * fac_ms;		    // 时间加载
	SysTick->VAL = 0x00;           				// 清空计数器
	SysTick->CTRL = 0x01 ;          			// 开始倒数  

	do
	{
		temp=SysTick->CTRL;
	}
	while(temp&0x01  &&  !(temp&(1<<16)));		// 等待时间到达 

	SysTick->CTRL = 0x00;       				// 关闭计数器
	SysTick->VAL = 0X00;       					// 清空计数器	  	    
}



/*******************************************************************
  * @file		  delay_ms_extend(uint32_t nms)
  * @brief  	毫秒级延时(extend) 
  * @param    -nms 延时时长 范围：0~1864	
  * @retval 	无
  ******************************************************************/ 
void delay_ms_extend(u32 nms) {
    while (nms > 1864) {
        delay_ms(1864);
        nms -= 1864;
    }
    delay_ms(nms);
}



/*******************************************************************
  * @file		delay_s(u8 ns)
  * @brief  	秒级延时 
  * @param      -ns 延时时长
  * @retval 	无
  ******************************************************************/   		    								   
void delay_s(u8 ns)
{
  while(ns--)
	{
		delay_ms_extend(1000);
	}
}



/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/
