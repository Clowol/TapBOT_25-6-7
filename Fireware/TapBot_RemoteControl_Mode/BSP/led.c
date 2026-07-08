/******************** (C) COPYRIGHT 2026 *****************************************
 * @file        led.c
 * @brief       LED GPIO initialization and control functions.
 * @note        
 * @warning     
 * @license     This project is released under the MIT License.
 *********************************************************************************/
#include "led.h"


// 全局变量定义
u8 led_debug_flag = 0;


/**************************************************************************
  * @name	  LED_Init(void)
  * @brief  I/O ports used for configuring LEDs 
  * @note    \PE 1
**************************************************************************/

void LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;				    // PE 1 端口配置
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		// 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   // 50MGz速率
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    GPIO_SetBits(GPIOE,GPIO_Pin_1);						   //PE 1 输出高

}


/**************************************************************************
  * @name	  LED_debug(void)
  * @brief    LED调试程序
  * @param  
  * @note     系统调试LED-快速闪烁功能
**************************************************************************/

void LED_debug(void)
{
		LED1( LED_ON );
		delay_ms(50);
		LED1( LED_OFF );
		delay_ms(50);
}



/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/
