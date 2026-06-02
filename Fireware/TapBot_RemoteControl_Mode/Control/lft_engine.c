/******************** (C) COPYRIGHT 2018 *****************************************
 * 文件名  ：lft_engine.c
 * 描述    ：左  主机启动、停止控制
 * 硬件配置：         
 * 版本    ：
 * 修改日期： 
 * 作者    ：
 * 修改日志：
*********************************************************************************/
#include "lft_engine.h"


u8 g_LftEngSttStpCtrl = 0xFF;													//启动左主机命令 		0-停止  1-启动  0xFF-无效  点触型
u8 g_LftEngSttStpState = 0x00;												//左主机启动状态 		0-停止  1-启动  0xFF-无效  
u16 g_LftSpeedRealVal = 0;														//左主机转速值			[0       3500]

u8 LftEngSttStpKeepFlg = 0xFF;												//发动机启停控制  保持性  0-停止  1-启动  0xFF-无效  



//以下为发动机相关控制
/*
 * 函数名：LftEngCtrlFun
 * 描述  ：主机启动控制周期函数
 * 输入  : 无
 * 输出  ：无
 */
void LftEngCtrlFun(void)
{	
	LftEngRmtUpCtrlFun();				//上层 遥控方式赋值
	
	LftEngSttStpCtrlFun();			//发动机上层 遥控器控制
	
	LftEngStartCtrlFun();				//发动机启动控制
	
	LftEngStopCtrlFun();				//发动机停止控制

//	LftEngParReadFun();					//发动机参数读取  辅助板直接传输
	
	LftEngWorkCheckFun();				//发动机工作检测
	
//	LftEngStartProtectFun();		//发动机启动保护
}



/*
 * 函数名：LftEngRmtUpCtrlFun
 * 描述  ：周期判断，接受的控制空调是上层控制，还是遥控器控制
 * 输入  ：需处理的接收字符串
 * 输出  ：无	
 */
void LftEngRmtUpCtrlFun(void)
{
	//判断控制方式
	if(RmtHalt_Stat != 1)									//如果不是急停下
	{
		switch(g_RmtUpManRealMode)
		{
			case RMT_MODE:										//遥控器模式
				g_LftEngSttStpCtrl = RmtLftEngStartStop;	
				break;		
			case UP_MODE:											//上层模式
				g_LftEngSttStpCtrl = UpLftEngStartStop;
				break;		
      case MAN_MODE:										//有人驾驶模式
        g_LftEngSttStpCtrl = 0xFF;	
				break;
			default:	
				break;
		}
	}
	else if(RmtHalt_Stat == 1)						//如果急停按下
	{
		g_LftEngSttStpCtrl = 0xFF;	
	}
	
	RmtLftEngStartStop = 0xFF;
}



/*
 * 函数名：LftEngSttStpCtrlFun
 * 描述  ：主机启动控制周期函数  触发量变成保持量
 * 输入  : 无
 * 输出  ：无
 */
void LftEngSttStpCtrlFun(void)
{
	switch(g_LftEngSttStpCtrl)
	{
		case 0:
			LftEngSttStpKeepFlg = 0;
			break;
		case 1:
			if(g_LftEngSttStpState == 0)
			{	
				LftEngSttStpKeepFlg = 1;
			}
			break;
		default: 
			;
	}
}



/*
 * 函数名：LftEngStopCtrlFun
 * 描述  ：主机停止控制  周期：10ms   
 *         降低油门-3s-继电器断开
 * 输入  : 无
 * 输出  ：无
 */
void LftEngStopCtrlFun(void)
{
	static u8 st = 0;
	static u16 cnt10ms = 0;
	
	//主机启动  清状态机
	if(LftEngSttStpKeepFlg == 1)																   	//0-停止
	{
		//主机停止需要时间，防止特别快的拨动开关，需把状态机复位
		st =0;
		cnt10ms = 0;
	}	

	//主机停止  降低油门
	if(LftEngSttStpKeepFlg == 0)																//0-停止	
	{
		if(g_fLftThroCtrlVal > 0)			//如果油门不为0
			g_fLftThroCtrlVal = 0;
	}
	
	//随时可以停止
	if(LftEngSttStpKeepFlg == 0)
	{
		switch(st)
		{
			case 0:
				SW_LFT_ENG_START(OFF);
        SW_LFT_ENG_POWER(OFF);
				
				swgPrt("LftEngStop !\r\n");
			
				g_LftSpeedRealVal = 0;						//主机关闭后将转速置为0  但是还可能会被B板转速数据赋值
			
				st++;
				break;
			case 1:
				if(++cnt10ms > 200)						//延时2s
				{
					cnt10ms = 0;
					st++;
				}
				break;
			case 2:
				SW_LFT_ENG_START(OFF);
        SW_LFT_ENG_POWER(OFF);

				LftEngSttStpKeepFlg = 0xFF;					//0xFF-无效
				g_LftSpeedRealVal = 0;						//主机关闭后将转速置为0
			
//				//*** 测试代码 测完删掉 *** LftEngWorkCheckFun 里面还有
//				g_LftEngSttStpState=0;
//				//*** ***

				st = 0;
				cnt10ms = 0;
				break;
		}
	}
}


/*
 * 函数名：LftEngStartCtrlFun
 * 描述  ：主机启动控制  周期：10ms   
 *         降低油门-上电继电器闭合-10s-启动继电器闭合-3s-启动继电器断开
 * 输入  : 无
 * 输出  ：无
 */
void LftEngStartCtrlFun(void)
{
	static u8 st = 0;
	static u16 cnt10ms = 0;

	//主机停止
	if(LftEngSttStpKeepFlg == 0)																   	//0-停止
	{
		//如果主机启动过程中可以停止中断启动   需把状态机复位
		SW_LFT_ENG_START(OFF);
		SW_LFT_ENG_POWER(OFF);
		st = 0;
		cnt10ms = 0;
	}	
	
	//主机启动  油门降至最低
	if(LftEngSttStpKeepFlg == 1)
	{
		if(g_fLftThroCtrlVal > 0)		//如果启动时候油门不为0
			g_fLftThroCtrlVal = 0;
	}

	//主机启动   启动按下&主机未工作&油门为0&转速为0
	if(LftEngSttStpKeepFlg == 1/*&& g_fLftThroCtrlVal == 0*/)	
	{
		switch(st)
		{
			case 0:	
				 SW_LFT_ENG_START(OFF);
				 SW_LFT_ENG_POWER(ON);
				st++;
				swgPrt("LftEngStart !\r\n");
				break;
			case 1:
				if(++cnt10ms > 1100)						//延时8s  
				{
					cnt10ms = 0;
					st++;
				}
				break;
			case 2:
				 SW_LFT_ENG_POWER(ON);
				 SW_LFT_ENG_START(ON);
				st++;
				break;
			case 3:
				if(++cnt10ms > 250)						//延时15s //或者是检测到转速信号
				{
					cnt10ms = 0;
					st++;
				}	
				break;
			case 4:
			  SW_LFT_ENG_START(OFF);
				SW_LFT_ENG_POWER(ON);
				LftEngSttStpKeepFlg = 0xFF;				//0xFF-无效;
				st = 0;
				cnt10ms = 0;
				swgPtn("here 1\r\n");
			
//				//*** 测试代码 测完删掉 *** LftEngWorkCheckFun 里面还有
//					g_LftEngSttStpState=1;
//				//*** ***

				break;
		}
	}
}



/*
 * 函数名：LftEngWorkCheckFun
 * 描述  ：检查当前工作状态函数  根据主机转速判断
 * 输入  : 无
 * 输出  ：无
 */
void LftEngWorkCheckFun(void)
{
	static char g_LftEngSttStpStateOld = 0;
	
	//*** 测试代码 测完取消注释 *** LftEngStartCtrlFun 里面还有
	//根据主机转速判断主机运行
	if(g_LftSpeedRealVal>= IDLE_SPEED)
	{
		g_LftEngSttStpState=1;
	}	
	else
	{
		g_LftEngSttStpState=0;
	}
	//*** ***
		
	//提供打印作用  打印一次
	if(g_LftEngSttStpState != g_LftEngSttStpStateOld)
	{
		switch(g_LftEngSttStpState)
		{
			case 0:
				swgPrt("LftEngStop !! \r\n");
				break;
			case 1:
				swgPrt("LftEngWork !! \r\n");
				break;
			default: ;
		}
	}
	g_LftEngSttStpStateOld = g_LftEngSttStpState;
}

/*
 * 函数名：LftEngParReadFun
 * 描述  ：读取主机参数，包括水温，油压，油量，转速
 * 输入  : 无
 * 输出  ：无
 */
void LftEngParReadFun(void)
{
//	g_fLftEngWaterTemp = GetVolt(ADC_FilteredValue[5]);                      //主机水温
//	g_fLftEngOilPress = GetVolt(ADC_FilteredValue[3]); 											//主机油压
}

/******************* (C) COPYRIGHT 2018 END OF FILE *****************************/










