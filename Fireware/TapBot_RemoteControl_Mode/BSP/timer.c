/******************** (C) COPYRIGHT 2026 *****************************************
 * @file        timer.c
 * @brief       Timer3/Timer4 initialization and interrupt handlers for the periodic task scheduler.
 * @note        
 * @warning     
 * @license     This project is released under the MIT License.
 *********************************************************************************/
#include "timer.h"


/**************************************************************************
  * @brief  全局变量定义
  * @param	-时间片标志和接收计数器	 （中断中修改，主程序读取）
  * @param	-通信超时计数器			（中断中递减，主程序重置）
  * @param	-通信状态标志			（中断中修改，主程序判断）	
  * @note   
***********************************************************************/
volatile u16 flag10ms = 0;				// 10ms 时间片标志
volatile u16 CntRx1 = 0;		  		// 串口1 接收时间标志位
volatile u16 CntRx2 = 0;		  		// 串口2 接收时间标志位
volatile u16 CntRx3 = 0;		  		// 串口3 接收时间标志位
volatile u16 CntRx4 = 0;		  		// 串口4 接收时间标志位
volatile u16 CntRx5 = 0;		  		// 串口5 接收时间标志位

volatile u16 CntRxUp = 60;				// 上层通信 接收帧计数
u16 CntRxAssist = 20;					// 辅助控制板通信 接收帧计数 
volatile u16 CntRxRmt = 80;				// 遥控器通信 接收帧计数
volatile u16 CntRxCAN1 = 80;			// CAN1通信 接收帧计数
volatile u16 CntRxCAN2 = 80;			// CAN2通信 接收帧计数

volatile u8 UpOutCommunFlg = 1;			//上层通信失联	 标志位   	0-正常  1-失联
u8 AssistOutCommunFlg = 1;				//辅助控制板失联 标志位  	0-正常  1-失联
volatile u8 RmtOutCommunFlg = 1;		//遥控器通信失联 标志位		0-正常  1-失联
volatile u8 CAN1_OutCommunFlg = 1;		//CAN1通信失联	 标志位		0-正常  1-失联
volatile u8 CAN2_OutCommunFlg = 1;		//CAN2通信失联	 标志位		0-正常  1-失联
volatile u8 RmtPwrOffFlg = 1;			//遥控器掉电	 标志位		0-正常  1-失联



/**************************************************************************
  * @name   Timer3_Init(u16 arr, u16 psc);
  * @brief  定时器3初始化
  * @param	arr 自动重装载值  （周期=arr+1个计数周期）
  * @param	psc 预分频值	   （时钟频率=72MHz/(psc+1)）
  * @retval none
***********************************************************************/
void Timer3_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef         NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //时钟使能

	TIM_TimeBaseStructure.TIM_Prescaler = psc; 					   // 预分频值:		eg. 7200-1 -> [10Khz 的计数频率]
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;    // 计数模式:		计数模式
	TIM_TimeBaseStructure.TIM_Period = arr; 					   // 重载周期值：	eg.计数到5000为500ms  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; 				   // 分频因子:	    TDTS = Tck_tim
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); 

	// 使能或者失能指定的TIM中断  ->  更新中断和触发中断
	TIM_ITConfig(TIM3, TIM_IT_Update | TIM_IT_Trigger,  ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  			   // TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;      // 先占优先级  2级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  		   // 从优先级	  1级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 			
	NVIC_Init(&NVIC_InitStructure); 

	TIM_Cmd(TIM3, ENABLE);  

   // 初始化接收计数器
	CntRx1=0;
	CntRx2=0;
	CntRx4=0;
	CntRx5=0;								 
}


/**************************************************************************
  * @name   TIM3_IRQHandler(void)
  * @brief  定时器3定时中断函数
  * @param	-None	  
  * @retval -None
***********************************************************************/
void TIM3_IRQHandler(void)
{
	static INT8U cnt1ms = 0;
	
	if (TIM_GetITStatus(TIM3,TIM_IT_Update) != RESET)
	{
		if(++cnt1ms>=10)
		{
			cnt1ms = 0;
			
			flag10ms = 1;
		}

	 	if(CntRx1)
		{
			if(--CntRx1==0)
			{
				USART1_RX_STA|=1<<15;
			}
		}
		if(CntRx2)
		{
			if(--CntRx2==0)
			{
				USART2_RX_STA|=1<<15;
			}
		}
		if(CntRx4)
		{
			if(--CntRx4==0)
			{
				USART4_RX_STA|=1<<15;
			}
		}
		if(CntRx5)				    
		{
			if(--CntRx5 == 0)	    	    	// 计数器减到0 ，10ms定时到
			{
				USART5_RX_STA |= (1 << 15);  	// 标记接收完成（bit15置1）
			}
		}
				
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update );
	}	
}



/**************************************************************************
  * @name   Timer4_Init(u16 arr, u16 psc);
  * @brief  定时器4	初始化
  * @param	arr 自动重装载值  
  * @param	psc 预分频值
  * @retval none
***********************************************************************/
void Timer4_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); 		// 时钟使能

	TIM_TimeBaseStructure.TIM_Prescaler = psc; 
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = arr;			 
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; 
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); 

	TIM_ITConfig(TIM4, TIM_IT_Update | TIM_IT_Trigger, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_Cmd(TIM4, ENABLE);  //使能TIMx外设
}




/**************************************************************************
  * @name   TIM4_IRQHandler(void)
  * @brief  定时器4中断服务函数，处理CAN1、遥控器等通信超时监测
  * @param	None	  
  * @retval -None
***********************************************************************/
void TIM4_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM4,TIM_IT_Update) != RESET)
	{		
		if(CntRxCAN1 && (--CntRxCAN1==0))	    	    // 计数器减到0，定时到
		{
			CAN1_OutCommunFlg = 1;   					// 标志： CAN1通信失联
			//swgPrt(" CAN1OutCom \r\n");
		}
		
		if(CntRxRmt && (--CntRxRmt==0))
		{
			RmtPwrOffFlg = 1;							// 标志： 遥控器接收器损坏
			//swgPrt(" RmtPwrOff \r\n");			
		}

        if(CntRxCAN2 && (--CntRxCAN2==0))
        {
            CAN2_OutCommunFlg = 1;
        }
	}
	TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
}




/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/
