/******************** (C) COPYRIGHT 2026 *****************************************
   * @author      Clomol
   * @date        2026-2027
   * @brief       
   * @license     [z]本代码仅用于教学与科研目的，未经作者书面许可，不得用于商业用途
   *              This project is released under the MIT License.
   * @note        
   * @warning     
*********************************************************************************/
#include "function.h"



/*************************** 全局变量定义 *******************************/
u8 g_RmtUpManCtrlMode = RMT_MODE;
u8 g_RmtUpManRealMode = RMT_MODE;

MOTOR_send cmd;
MOTOR_recv data;

/******************** 串口发送缓冲区定义 ****************************/
u8 USART2_SendBuf[] =
{
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, \
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, \
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, \
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, \
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, \
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02
};

u8 USART4_SendBuf[] =
{
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, \
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, \
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, \
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, \
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, \
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04
};



void FunctionProce(void)
{
	USART1_Proc();
	USART2_Proc();
	Rmt_CommProc();
	USART4_Proc();
	USART5_Proc();
}

/********************************* 10ms  ************************************/
/**
 * @brief   10ms周期函数  有顺序要求
 * @note    高频控制任务
 */
void Function_10ms(void)
{
	ControlDispatcher_Update();
}

/********************************* 30ms  ************************************/
/**
 * @brief   30ms周期函数  有顺序要求
 * @note    执行宇树电机速度控制，平衡实时性与资源占用
 */
void Function_30ms(void)
{
	YushuMotor_ControlProc();
}

/********************************* 50ms  ************************************/
/**
 * @brief   50ms周期函数  有顺序要求
 * @note    当前用于云台控制，适合中等频率的控制任务
 */
void Function_50ms(void)
{
	PTZ_ControlProc();
}

/********************************* 100ms  ************************************/
/**
 * @brief   100ms周期函数  有顺序要求
 * @note    当前用于转向控制，适合较低频率的控制任务
 */
void Function_100ms(void)
{
	Steer_ControlProc();
}

/********************************* 200ms  ************************************/
/**
 * @brief   200ms周期函数  有顺序要求
 * @note    当前用于某些中等频率的控制任务
 */
void Function_200ms(void)
{
}

/********************************* 300ms  ************************************/
/**
 * @brief   300ms周期函数  有顺序要求
 * @note    当前用于某些中等频率的控制任务
 */
void Function_300ms(void)
{
}

/********************************* 500ms  ************************************/
/**
 * @brief   500ms周期函数  有顺序要求
 * @note    当前用于某些较低频率的控制任务
 */
void Function_500ms(void)
{
}

/********************************* 1s  ************************************/
/**
 * @brief   1s周期函数  有顺序要求
 * @note    当前用于某些低频率的控制任务:执行LED状态控制、系统状态打印等低频次任务
 */
void Function_1s(void)
{
    // LED指示灯状态控制
	LED_WorkCtrlFun();
    // 系统状态调试打印
	function_prtf();
}




/********************************************************************************
 * @name 	LED_WorkCtrlFun（）
 * @brief   LED指示灯状态控制函数
 * @param   无
 * @retval  无
 * @note    控制电路板上的LED灯和BOOT按钮上的呼吸灯
 ********************************************************************************/
void LED_WorkCtrlFun(void)
{
	static u8 led_toggle = 0;

	if((led_toggle & 0x01U) == 0U)
	{
		LED1(ON);
		SW_BOOT_LED(ON);
	}
	else
	{
		LED1(OFF);
		SW_BOOT_LED(OFF);
	}

	led_toggle++;
}

/**********************************************************************************
 * @name	USART1_Proc()
 * @brief   USART1数据处理函数（宇树电机485）
 * @param   无
 * @retval  无
 * @note    处理宇树电机串口接收数据，调试代码默认关闭
 ***********************************************************************************/
void USART1_Proc(void)
{
	if(USART1_RX_STA)
	{
		USART1_RX_STA = 0;
	}
}

/****************************************************************************
 * @name	USART2_Proc()
 * @brief   USART2数据处理函数（上层通信232）
 * @param   无
 * @retval  无
 * @note    处理上层通信串口接收数据，调试代码默认关闭
 ****************************************************************************/
void USART2_Proc(void)
{
	if(USART2_RX_STA)
	{
		USART2_RX_STA = 0;
	}
}

/***************************************************************************
 * @name	USART4_Proc()
 * @brief   USART4数据处理函数（云台485）
 * @param   无
 * @retval  无
 * @note    处理云台串口接收数据，调试代码默认关闭
 ****************************************************************************/
void USART4_Proc(void)
{
	if(USART4_RX_STA)
	{
		USART4_RX_STA = 0;
	}
}

/*************************************************************************
 * @name	USART5_Proc()
 * @brief   USART5数据处理函数（舵机485）
 * @param   无
 * @retval  无
 * @note    处理舵机串口接收数据，调试代码默认关闭
 ****************************************************************************/
void USART5_Proc(void)
{
	if(USART5_RX_STA & (1U << 15))
	{
		LEN5 = USART5_RX_STA & 0x03FFU;
		USART5_RX_STA = 0;
	}
}

/***************************************************************************************************
 * @name	function_prtf()
 * @brief   1s周期系统状态打印函数
 * @param   无
 * @retval  无
 * @note    打印遥控器通道、IO状态、电机速度等关键调试信息
 ***************************************************************************************************/
void function_prtf(void)
{
	static u8 print_toggle = 0;

	if((print_toggle & 0x01U) == 0U)
	{
		Can1_Send_Msg(CAN1_SendMsg2, 0x01, 2);
	}

#if 1
	swgPrtUx(USART2, "=========================================\r\n");
    swgPrtUx(USART2, "           System Status [1s]            \r\n");
    swgPrtUx(USART2, "=========================================\r\n");

	swgPrtUx(USART2, "CH_X2=%d \r\n", Rmtrece_Msg.CH_X2);
	swgPrtUx(USART2, "CH_Y2=%d \r\n", Rmtrece_Msg.CH_Y2);
	swgPrtUx(USART2, "CH_Y1=%d \r\n", Rmtrece_Msg.CH_Y1);
	swgPrtUx(USART2, "CH_X1=%d \r\n", Rmtrece_Msg.CH_X1);
	swgPrtUx(USART2, "CH_E=%d \r\n", Rmtrece_Msg.CH_E);
	swgPrtUx(USART2, "CH_G=%d \r\n", Rmtrece_Msg.CH_G);
	swgPrtUx(USART2, "CH_H=%d \r\n", Rmtrece_Msg.CH_H);
	swgPrtUx(USART2, "CH_F=%d \r\n", Rmtrece_Msg.CH_F);
	swgPrtUx(USART2, "CH_A=%d \r\n", Rmtrece_Msg.CH_A);
	swgPrtUx(USART2, "CH_B=%d \r\n", Rmtrece_Msg.CH_B);
	swgPrtUx(USART2, "CH_C=%d \r\n", Rmtrece_Msg.CH_C);
	swgPrtUx(USART2, "CH_D=%d \r\n", Rmtrece_Msg.CH_D);
	swgPrtUx(USART2, "CH_RD=%d \r\n", Rmtrece_Msg.CH_RD);
	swgPrtUx(USART2, "CH_RSSI=%d \r\n", Rmtrece_Msg.CH_RSSI);
	swgPrtUx(USART2, "CH_A2=%d \r\n", Rmtrece_Msg.CH_A2);
	swgPrtUx(USART2, "CH_B2=%d \r\n", Rmtrece_Msg.CH_B2);
	swgPrtUx(USART2, "flags=%d \r\n", Rmtrece_Msg.flags);
	swgPrtUx(USART2, "IN12=%d \r\n", IN12);
	swgPrtUx(USART2, "YushuSpeed=%.1f \r\n", YushuSpeed);
	swgPrtUx(USART2, "\r\n");
#endif

	print_toggle++;
}



/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/
