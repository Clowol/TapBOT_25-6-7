/******************** (C) COPYRIGHT 2026 *****************************************
 * @file        main.c
 * @author      Clomol
 * @date        2025-2026
 * @brief       main program
 * 
 * @license     [z]本代码仅用于教学与科研目的，未经作者书面许可，不得用于商业用途
 *              This project is released under the MIT License.
 * @note        [z]使用本代码时请保留此版权声明
 *              Please retain this copyright notice when using this code
 * @warning     [z]本人能力有限，这段代码可能存在错误或不完善之处，使用前请仔细检查并测试
 *              I am not responsible for any damage or loss caused by using this code. Please review             
*********************************************************************************/
#include "stm32f10x.h"
#include "function.h"

int main()
{
    static u16 cnt1s = 0;       // 1s计数器
    static u16 cnt30ms = 0;     // 30ms计数器
    static u16 cnt50ms = 0;     // 50ms计数器
    static u16 cnt100ms = 0;    // 100ms计数器
    static u16 cnt200ms = 0;    // 200ms计数器
    static u16 cnt300ms = 0;    // 300ms计数器
    static u16 cnt500ms = 0;    // 500ms计数器

    SystemInit();              // 系统时钟配置
    delay_init(72);			   // 延时函数 初始化

    LED_Init();                // LED 初始化
    NVIC_Configuration();      // NVIC中断优先级分组配置

    USART1_Init(APP_USART1_BAUD);		// 串口1-宇树电机-485
	USART2_Init(APP_USART2_BAUD); 		// 串口2-上位机/调试-232
	USART3_Init(APP_USART3_BAUD); 		// 串口3-遥控器接收器-TTL CBUS
	USART4_Init(APP_UART4_BAUD);		// 串口4-云台-485
	USART5_Init(APP_UART5_BAUD);		// 串口5-舵机-485

    Switch_Init();             // 开关输入初始化
    // ADC1_Init();               // ADC1 初始化

    USER_CAN1_Init();             // CAN1 初始化
    // USER_CAN2_Init();             // CAN2 初始化

    Timer3_Init(10,7199);		  // 1ms 定时
	Timer4_Init(500,7199);		  // 50ms 定时

    SendSteer_SYNC_SetMode(STEER_SPEED_MODE);
	Send_PTZ_Data();
	Encoder_Init();

    swgPrtUx(USART2, "\r\n Init Ok \r\n");

    while(1)
    {
		debug();

		ROS2_CommProc();
		
		FunctionProce();
		
		if(flag10ms)			   										
		{
            /*==========  10ms ==========*/
			flag10ms = 0;												
			Function_10ms();			

			/*==========  30ms ==========*/
			if(++cnt30ms>=3)
			{
				cnt30ms=0;
				Function_30ms();		
			}

            /*==========  50ms ==========*/
			if(++cnt50ms>=5)     
			{
				cnt50ms=0;
				Function_50ms();		
			}
			/*==========  100ms ==========*/
			if(++cnt100ms>=10)     
			{
				cnt100ms=0;
				Function_100ms();		
			}
			/*==========  200ms ==========*/	
			if(++cnt200ms>=20)
			{
				cnt200ms = 0;
				Function_200ms();   
			}
			/*==========  300ms ==========*/
			if(++cnt300ms>=30)
			{
				cnt300ms = 0;
				Function_300ms();  
			}
			/*==========  500ms ==========*/
			if(++cnt500ms>=50)
			{
				cnt500ms = 0;
				Function_500ms();  
			}
            /*==========  1s ==========*/
			if(++cnt1s>=100)	   								
			{
				cnt1s=0;
				Function_1s();				
			}
		}
    }
}

/******************* (C) COPYRIGHT 2026 END OF FILE *****************************/
