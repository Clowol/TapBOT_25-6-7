/******************** (C) COPYRIGHT 2018 *****************************************
 * 文件名  ：generator.c
 * 描述    ：发电机启动、停止控制
 * 硬件配置：         
 * 版本    ：
 * 修改日期： 
 * 作者    ：
 * 修改日志：
*********************************************************************************/
#include "generator.h"

u8 GenSttStpKeepFlg = 0xFF;					// 发电机启停控制  保持型  0-停止  1-启动  0xFF-无效
u8 g_GenSttStpCtrl = 0xFF;													 	//启动发电机命令 		0-停止  1-启动  0xFF-无效  点触型
u8 g_GenSttStpState = 0x00;														//启动发电机状态 		0-停止  1-启动  0xFF-无效  

////////////////////////////////////////////////////////////////////////////////////////////////////
//											   RPC-ID   param #1  param #2  param #3
u8 CAN2_GEN_SendMsg1[8]={0x08,0x09,0x00,0x01,0x00,0x00,0x00,0x00};   /* Start */  
u8 CAN2_GEN_SendMsg2[8]={0x08,0x09,0x00,0x00,0x00,0x00,0x00,0x00};   /* Stop  */  


//以下为发电机相关控制
/*
 * 函数名：GenStartCtrlFun
 * 描述  ：发电机启动控制周期函数
 * 输入  : 无
 * 输出  ：无
 */
void GenCtrlFun(void)
{	
	GenRmtUpCtrlFun();				//上层 遥控方式赋值
		
	GenSttStpCtrlFun();				//发电机上层 遥控器控制
	
	GenStartCtrlFun();				//发电机启动控制
	
	GenStopCtrlFun();					//发电机停止控制

//	GenReadValueFun();				//发电机读取参数值
	
	GenWorkCheckFun();				//发电机工作检测
	
//	GenStartProtectFun();		//发电机启动保护
}



/*
 * 函数名：GenRmtUpCtrlFun
 * 描述  ：周期判断，接受的控制空调是上层控制，还是遥控器控制
 * 输入  ：需处理的接收字符串
 * 输出  ：无	
 */
void GenRmtUpCtrlFun(void)
{
	//判断控制方式
	switch(g_RmtUpManRealMode)
	{
		case RMT_MODE:	//遥控器模式														
			g_GenSttStpCtrl = RmtGenStartStop;	
			break;
		case UP_MODE:		//上层模式
			g_GenSttStpCtrl = UpGenStartStop;
			break;		
		case MAN_MODE:	//有人驾驶模式
			g_GenSttStpCtrl = 0xFF;
			break;		
		default:	
			break;
	}
	
	RmtGenStartStop = 0xFF;
}



/*
 * 函数名：GenSttStpCtrlFun
 * 描述  ：发电机启动控制周期函数  触发量变成保持量
 * 输入  : 无
 * 输出  ：无
 */
void GenSttStpCtrlFun(void)
{
	switch(g_GenSttStpCtrl)
	{
		case 0:
			GenSttStpKeepFlg = 0;             //发电机停止
			break;
		case 1:
			if(g_GenSttStpState == 0)
				GenSttStpKeepFlg = 1;						//发电机启动
			break;
		default: 
			;
	}
}




/*
 * 函数名：GenStopCtrlFun
 * 描述  ：发电机停止控制  周期：10ms   
 *         停止继电器断开3秒  再闭合
 * 输入  : 无
 * 输出  ：无
 */
void GenStopCtrlFun(void)
{
	static u8 st = 0;
	static u16 cnt10ms = 0;
	
	//发电机启动  清状态机
	if(GenSttStpKeepFlg == 1)																   	//0-停止
	{
		//发电机停止需要时间，防止特别快的拨动开关，需把状态机复位
		st =0;
		cnt10ms = 0;
	}	
	
	//随时可以停止
	if(GenSttStpKeepFlg == 0)
	{
		switch(st)
		{
			case 0:
				SW_GEN_START(OFF);						
				SW_GEN_STOP(ON);
				st++;
				swgPrt("GenStop !\r\n");
				break;
			case 1:
				if(++cnt10ms > 200)						   //延时
				{
					cnt10ms = 0;
					st++;
				}
				break;
			case 2:				
				SW_GEN_START(OFF);						
				SW_GEN_STOP(OFF);
				GenSttStpKeepFlg = 0xFF;			   //0xFF-无效
				st = 0;
				cnt10ms = 0;
			
//			g_GenSttStpState = 0;
			
				break;
		}
	}
}


/*
 * 函数名：GenStartCtrlFun
 * 描述  ：发电机启动控制  周期：10ms   
 * 输入  : 无
 * 输出  ：无
 */
void GenStartCtrlFun(void)
{
	static u8 st = 0;
	static u16 cnt10ms = 0;

	//发电机停止
	if(GenSttStpKeepFlg == 0)																   	//0-停止
	{
		//如果发电机启动过程中可以停止中断启动   需把状态机复位
		st = 0;
		cnt10ms = 0;
	}	
	
	//发电机启动   启动按下&发电机未工作&油门为0&转速为0
	if(GenSttStpKeepFlg == 1)	
	{
		switch(st)
		{
			case 0:
				SW_GEN_START(ON);								
				SW_GEN_STOP(OFF);
				st++;
				swgPrt("GenStart !\r\n");
				break;
	    case 1:
				if(++cnt10ms > 1200)              //延时13s
				{
					cnt10ms = 0;
				  st++;
				}
				break;
			case 2:
				SW_GEN_START(OFF);
				SW_GEN_STOP(OFF);
				GenSttStpKeepFlg = 0xFF;				//0xFF-无效;
				st = 0;
				cnt10ms = 0;
				swgPrt("here 7\r\n");
			
//			//测试代码  测完删掉
//			g_GenSttStpState = 1;
//			//*** ***
				
				break;
		}
	}
}




///*
// * 函数名：GenReadValueFun
// * 描述  ：发电机读取水温和油压
// * 输入  ：
// * 输出  ：无	
// */
//void GenReadValueFun(void)
//{
////	g_fGenOilPress = ADC_FilteredValue[4];
//	
//	//暂时  发电机启动即赋值
//	if(g_GenSttStpState == 1)
//	{
////		g_fGenOilPress = 40;
////		g_fGenWaterTemp = 85;
//	}
//	else if(g_GenSttStpState == 0)
//	{
////		g_fGenOilPress = 0;
////		g_fGenWaterTemp = 0;
//	}
//}



/*
 * 函数名：GenWorkCheckFun
 * 描述  ：检查当前工作状态函数  根据发电机油温判断
 * 输入  : 无
 * 输出  ：无
 */
void GenWorkCheckFun(void)
{
	static u8 g_GenSttStpStateOld = 0; 
	
	if(GEN_STATE_IN_220V == 0)
	{
		g_GenSttStpState = 1;	
	}
	else if(GEN_STATE_IN_220V == 1)
	{
		g_GenSttStpState = 0;	
	}

	//提供打印作用  打印一次
	if(g_GenSttStpState != g_GenSttStpStateOld)
	{
		switch(g_GenSttStpState)
		{
			case 0:
				swgPrt("GenStop !! \r\n");
				break;
			case 1:
				swgPrt("GenWork !! \r\n");
				break;
			default: ;
		}
	}
	g_GenSttStpStateOld = g_GenSttStpState;
}



/******************* (C) COPYRIGHT 2018 END OF FILE *****************************/








