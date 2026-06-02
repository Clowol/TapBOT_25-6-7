/******************** (C) COPYRIGHT 2018 *****************************************
 * 文件名  ：rgt_engine.c
 * 描述    ：右 主机启动、停止控制
 * 硬件配置：         
 * 版本    ：
 * 修改日期： 
 * 作者    ：
 * 修改日志：
*********************************************************************************/
#include "rgt_engine.h"


u8 g_RgtEngSttStpCtrl = 0xFF;													//启动右主机命令 		0-停止  1-启动  0xFF-无效  点触型
u8 g_RgtEngSttStpState = 0x00;												//右主机启动状态 		0-停止  1-启动  0xFF-无效  
u16 g_RgtSpeedRealVal = 0;														//右主机转速值			[0       3500]

u8 RgtEngSttStpKeepFlg = 0xFF;												//发动机启停控制  保持性  0-停止  1-启动  0xFF-无效  



//以下为发动机相关控制
/*
 * 函数名：RgtEngCtrlFun
 * 描述  ：主机启动控制周期函数
 * 输入  : 无
 * 输出  ：无
 */
void RgtEngCtrlFun(void)
{	
	RgtEngRmtUpCtrlFun();					//上层 遥控方式赋值
	
	RgtEngSttStpCtrlFun();				//发动机上层 遥控器控制
	
	RgtEngStartCtrlFun();					//发动机启动控制
	
	RgtEngStopCtrlFun();					//发动机停止控制

//	RgtEngParReadFun();						//发动机参数读取
	
	RgtEngWorkCheckFun();					//发动机工作检测
	
//	RgtEngStartProtectFun();		//发动机启动保护
}



/*
 * 函数名：RgtEngRmtUpCtrlFun
 * 描述  ：周期判断，接受的控制空调是上层控制，还是遥控器控制
 * 输入  ：需处理的接收字符串
 * 输出  ：无	
 */
void RgtEngRmtUpCtrlFun(void)
{
	//判断控制方式
	if(RmtHalt_Stat != 1)									//如果不是急停下
	{
		switch(g_RmtUpManRealMode)
		{
			case RMT_MODE:										//遥控器模式
				g_RgtEngSttStpCtrl = RmtRgtEngStartStop;	
				break;		
			case UP_MODE:											//上层模式
				g_RgtEngSttStpCtrl = UpRgtEngStartStop;
				break;		
      case MAN_MODE:										//有人驾驶模式
        g_RgtEngSttStpCtrl = 0xFF;
				break;
			default:	
				break;
		}
	}
	else if(RmtHalt_Stat == 1)						//如果急停按下
	{
		g_RgtEngSttStpCtrl = 0xFF;	
	}
	
	RmtRgtEngStartStop = 0xFF;
}



/*
 * 函数名：RgtEngSttStpCtrlFun
 * 描述  ：主机启动控制周期函数  触发量变成保持量
 * 输入  : 无
 * 输出  ：无
 */
void RgtEngSttStpCtrlFun(void)
{
	switch(g_RgtEngSttStpCtrl)
	{
		case 0:
			RgtEngSttStpKeepFlg = 0;
			break;
		case 1:
			if(g_RgtEngSttStpState == 0)
			{
				RgtEngSttStpKeepFlg = 1;
			}
			break;
		default: 
			;
	}
}



/*
 * 函数名：RgtEngStopCtrlFun
 * 描述  ：主机停止控制  周期：10ms   
 *         降低油门-3s-继电器断开
 * 输入  : 无
 * 输出  ：无
 */
void RgtEngStopCtrlFun(void)
{
	static u8 st = 0;
	static u16 cnt10ms = 0;
	
	//主机启动  清状态机
	if(RgtEngSttStpKeepFlg == 1)																   	//0-停止
	{
		//主机停止需要时间，防止特别快的拨动开关，需把状态机复位
		st =0;
		cnt10ms = 0;
	}	

	//主机停止  降低油门
	if(RgtEngSttStpKeepFlg == 0)																//0-停止	
	{
		if(g_fLftThroCtrlVal > 0)			//如果油门不为0
			g_fLftThroCtrlVal = 0;
	}
	
	//随时可以停止
	if(RgtEngSttStpKeepFlg == 0)
	{
		switch(st)
		{
			case 0:
				SW_RGT_ENG_START(OFF);
        SW_RGT_ENG_POWER(OFF);
				
				swgPrt("RgtEngStop !\r\n");
			
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
				SW_RGT_ENG_START(OFF);
        SW_RGT_ENG_POWER(OFF);

				RgtEngSttStpKeepFlg = 0xFF;					//0xFF-无效
				g_LftSpeedRealVal = 0;						//主机关闭后将转速置为0
			
//				//*** 测试代码 测完删掉 *** RgtEngWorkCheckFun 里面还有
//				g_RgtEngSttStpState=0;
//				//*** ***

				st = 0;
				cnt10ms = 0;
				break;
		}
	}
}


/*
 * 函数名：RgtEngStartCtrlFun
 * 描述  ：主机启动控制  周期：10ms   
 *         降低油门-上电继电器闭合-10s-启动继电器闭合-3s-启动继电器断开
 * 输入  : 无
 * 输出  ：无
 */
void RgtEngStartCtrlFun(void)
{
	static u8 st = 0;
	static u16 cnt10ms = 0;

	//主机停止
	if(RgtEngSttStpKeepFlg == 0)																   	//0-停止
	{
		//如果主机启动过程中可以停止中断启动   需把状态机复位
		SW_RGT_ENG_START(OFF);
		SW_RGT_ENG_POWER(OFF);
		st = 0;
		cnt10ms = 0;
	}	
	
	//主机启动  油门降至最低
	if(RgtEngSttStpKeepFlg == 1)
	{
		if(g_fLftThroCtrlVal > 0)		//如果启动时候油门不为0
			g_fLftThroCtrlVal = 0;
	}

	//主机启动   启动按下&主机未工作&油门为0&转速为0
	if(RgtEngSttStpKeepFlg == 1 /* && g_fLftThroCtrlVal == 0*/)	
	{
		switch(st)
		{
			case 0:	
				 SW_RGT_ENG_START(OFF);
				 SW_RGT_ENG_POWER(ON);
				st++;
				swgPrt("RgtEngStart !\r\n");
				break;
			case 1:
				if(++cnt10ms > 1100)						//延时8s  
				{
					cnt10ms = 0;
					st++;
				}
				break;
			case 2:
				 SW_RGT_ENG_POWER(ON);
				 SW_RGT_ENG_START(ON);
				st++;
				break;
			case 3:
				if(++cnt10ms > 250)						//延时2s //或者是检测到转速信号
				{
					cnt10ms = 0;
					st++;
				}	
				break;
			case 4:
			  SW_RGT_ENG_START(OFF);
				SW_RGT_ENG_POWER(ON);
				RgtEngSttStpKeepFlg = 0xFF;				//0xFF-无效;
				st = 0;
				cnt10ms = 0;
				swgPtn("here 2\r\n");
			
//				//*** 测试代码 测完删掉 *** RgtEngWorkCheckFun 里面还有
//					g_RgtEngSttStpState=1;
//				//*** ***

				break;
		}
	}
}



/*
 * 函数名：RgtEngWorkCheckFun
 * 描述  ：检查当前工作状态函数  根据主机转速判断
 * 输入  : 无
 * 输出  ：无
 */
void RgtEngWorkCheckFun(void)
{
	static char g_RgtEngSttStpStateOld = 0;
	
	//*** 测试代码 测完取消注释 *** RgtEngStartCtrlFun 里面还有
	//根据主机转速判断主机运行
	if(g_RgtSpeedRealVal>= IDLE_SPEED)
	{
		g_RgtEngSttStpState=1;
	}	
	else
	{
		g_RgtEngSttStpState=0;
	}
	//*** ***
		
	//提供打印作用  打印一次
	if(g_RgtEngSttStpState != g_RgtEngSttStpStateOld)
	{
		switch(g_RgtEngSttStpState)
		{
			case 0:
				swgPrt("RgtEngStop !! \r\n");
				break;
			case 1:
				swgPrt("RgtEngWork !! \r\n");
				break;
			default: ;
		}
	}
	g_RgtEngSttStpStateOld = g_RgtEngSttStpState;
}

/*
 * 函数名：RgtEngParReadFun
 * 描述  ：读取主机参数，包括水温，油压，油量，转速
 * 输入  : 无
 * 输出  ：无
 */
void RgtEngParReadFun(void)
{
//	g_fRgtEngWaterTemp = GetVolt(ADC_FilteredValue[5]);                      //主机水温
//	g_fRgtEngOilPress = GetVolt(ADC_FilteredValue[3]); 											//主机油压
}

/******************* (C) COPYRIGHT 2018 END OF FILE *****************************/










