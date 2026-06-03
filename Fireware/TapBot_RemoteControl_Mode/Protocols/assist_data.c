/******************** (C) COPYRIGHT 2018 *****************************************
 * 文件�? ：assist_data.c
 * 描述    ：处理主板与辅助板之间的通信函数
 * 硬件配置:              
 * 版本    �?
 * 修改日期�?
 * 作�?   �?
 * 修改日志�?
*********************************************************************************/
#include "assist_data.h"


//辅助板信�?
u8 g_u8OilTankVolume = 0;															//油箱油量
u8 g_u8BattPercent_1 =80;															//电池电量 01 底层电池 百分�? 0-100  
u8 g_u8BattPercent_2 =80;															//电池电量 02 启动电池 百分�? 0-100 
u8 g_u8RoomTemp = 25;																	//房间温度


u8 AssistDataBUF[100]={0};	      //定义底层通过串口发送给辅助控制板的缓存数组

struct ReceAssist_Msg ReceAssistMsg=	    	//定义接收的数据结构体
{	
			0xA8,						//	u8	RStart1;								// 0  数据帧头 0xA8
			0x99,						//	u8  RStart2;								// 1  数据帧头 0x99
			   0,						//	u8 	MsgNumber;					   	// 2  通信确认 0x00-FF的循环数

				 0,						//	s8 	LftEngWaterTemp;				// 3  左发动机水温�? 1字节�? INT8，单位： ℃，最低有效位�?  1，[-30 - +100] �?
			   0,						//	u16 LftEngSpeedValue;				// 4  左发动机转速�? 2字节，UINT16，单位：RPM，最低有效位�?  1，[  0 - 5000]RPM 	
			   0,						//	u8 LftEngThrotValue;				// 6  左主机油门�?   1字节�?UINT8，单位：  %，最低有效位�?  1，[  0 -  100]  %
			   0,						//	u8 	LftEngOilPress;					// 7  左发动机油压�? 1字节�?UINT8，单位：kPa，最低有效位�?  4，[  0 - 1000]kPa
			   0,						//	u8  LftEngineState;					// 8  左发动机状�?   1字节�?UINT8�?
			   0,						//	u16 LftEngFuelRate;		    	// 9  左发动机油�?   2字节，UINT16，单位：L/h，最低有效位�?.05，[0 - 3212.75]L/h

				 0,						//	s8 	RgtEngWaterTemp;				// 11 右发动机水温�? 1字节�? INT8，单位： ℃，最低有效位�?  1，[-30 - +100] �?
			   0,						//	u16	RgtEngSpeedValue;				// 12 右发动机转速�? 2字节，UINT16，单位：RPM，最低有效位�? 1，[  0 - 5000]RPM 	
			   0,						//	u8  RgtEngThrotValue;				// 14 右主机油门�?   1字节�?UINT8，单位：  %，最低有效位�? 1，[  0 -  100]  %
			   0,						//	u8  RgtEngOilPress;					// 15 右发动机油压�? 1字节�?UINT8，单位：kPa，最低有效位�?  4，[  0 - 1000]kPa
			   0,						//	u8  RgtEngineState;					// 16 右发动机状�?   1字节�?UINT8�?
			   0,						//	u16 RgtEngFuelRate;		    	// 17 右发动机油�?   2字节，UINT16，单位：L/h，最低有效位�?.05，[0 - 3212.75]L/h 

	
			  81,						//	u8	LftOilTankVolume;				// 19 左油量反�?		1字节�?UINT8，单位：  %，最低有效位�?  1，[  0 -  100]  %
			  82,						//	u8	RgtOilTankVolume;				// 20 右油量反�?		1字节�?UINT8，单位：  %，最低有效位�?  1，[  0 -  100]  %

			 121,						//	u8	BatteryVol_1;						// 21 电池1 	1字节�?UINT8，单位：  V，最低有效位�?0.1，[  0 - 14.3]  V
			 122,						//	u8	BatteryVol_2;						// 22 电池2 	1字节�?UINT8，单位：  V，最低有效位�?0.1，[  0 - 14.3]  V
			 123,						//	u8	BatteryVol_3;						// 23 电池3 	1字节�?UINT8，单位：  V，最低有效位�?0.1，[  0 - 14.3]  V
		(12.4-5)*10,			//	u8	BatteryVol_4;						// 24 电池4 	1字节�?UINT8，单位：  V，最低有效位�?0.1，[  5 - 30.3]  V   0V=5V
		(12.5-5)*10,			//	u8	BatteryVol_5;						// 25 电池5 	1字节�?UINT8，单位：  V，最低有效位�?0.1，[  5 - 30.3]  V		0V=5V
		(12.6-5)*10,			//	u8	BatteryVol_6;						// 26 电池6 	1字节�?UINT8，单位：  V，最低有效位�?0.1，[  5 - 30.3]  V		0V=5V

			   0,						//	u16 DeviceState;					  // 27 bit0-15 
											//																		//bit0	机舱风机							
											//																		//bit1	管道风机
											//																		//bit2	浮子开�?
											//																		//bit3	浮子开�?
											//																		//bit4	浮子开�?
											//																		//bit5	浮子开�?
											//																		//bit6	温度感应�?
											//																		//bit7  烟雾感应�?
											//																		//bit8	接近开�?  桅杆倒下
											//																		//bit9	接近开�?  桅杆立起
											//																		//bit10	接近开�?  声呐收起	
											//																		//bit11	接近开�?  声呐下放
											//																		//bit12	预留
											//																		//bit13	预留
											//																		//bit14	预留
											//																		//bit15	预留

			   0,						//	s16	TempData;								// 29 温度�?				2字节�?INT16
			   0,						//	u16	HumiData;								// 31 湿度�?			 	2字节，UINT16
			   0,						//	u8	Reserve1;								// 33 预留�?�?			1字节，UINT8
			   0,						//	u8	Reserve2;								// 34 预留�?�?			1字节，UINT8
			   0,						//	u8	SumCheck;    						// 35 校验�?
};


struct SendToAssist_Msg	SendToAssistMsg=			  //定义发送的数据结构�?
{	
			0xA8,						//	u8 	SStart1;								// 0  数据帧头 0xA8
			0x99,						//	u8 	SStart2;								// 1  数据帧头 0x99	
			   0,						//	u8 	MsgNumber;	           	// 2  通信确认
			   0,						//	u8 	SwitchCmd1;							// 3  开关命�?	
											//																		//bit7-bit6	桅灯				11-开 		00-�?   	01 10-无效
											//																		//bit5-bit4	艉灯   			11-开 		00-�?   	01 10-无效
											//																		//bit3-bit2	左舷  			11-开 		00-�?   	01 10-无效
											//																		//bit1-bit0	右舷    		11-开 		00-�?   	01 10-无效
			   0,						//	u8 	SwitchCmd2;             // 4  开关命�?
											//																		//bit7			预留急停
											//																		//bit6			预留
											//																		//bit5-bit4	预留
											//																		//bit3-bit2	声呐 				01-收起 	10-下放 	00 11-无效
											//																		//bit1-bit0	可倒桅   		01-上升 	10-下降  	00 11-无效
			   0,						//	u8 	SwitchCmd3;							// 5  开关命�?
											//																		//bit7-bit6	预留		
											//																		//bit5-bit4	预留   		
											//																		//bit3-bit2	设备舱风�? 11-开 		00-�?   	01 10-无效
											//																		//bit1-bit0	机舱风机  	11-开 		00-�?   	01 10-无效
			   0,						//	u8 	Reserve;								// 6  预留
											//																		//bit5-bit4	CH_SB  			11-锁住 	00-松开    		01 10-无效
											//																		//bit3-bit2	CH_RD  			11-上升 	00-下降    		01 10-无效
											//																		//bit1-bit0	控制模式  	11-上层 	00-遥控�?   	01 10-无效
			   0,						//	u8 	SumCheck;     					// 7  校验�? 
};
	


/*
 * 函数名：Proce_AssistData
 * 描述  ：解析处理接收到的数�?
 * 输入  ：ReceDat，接收到的数据数�?
 * 输出  ：无	
 */
void Proce_AssistData(char *ReceDat)
{
	u8 Pro_crct=0;  																				//校验�?

	ReceAssistMsg.RStart1=ReceDat[0];	    								//接收数据字节0 帧头 数据帧头 0xAA
	ReceAssistMsg.RStart2=ReceDat[1];	    								//接收数据字节0 帧头 数据帧头 0x06
	ReceAssistMsg.SumCheck=ReceDat[35];	 						   		//接收数据字节作为校验

  Pro_crct = CheckCrc_8bit((u8*)ReceDat, 35);
	
//	swgPrt("Pro_crct = %X \r\n", Pro_crct);
	
	//帧头及校验位同时满足
	if(ReceAssistMsg.RStart1==0xA8 && ReceAssistMsg.RStart2==0x99 && ReceAssistMsg.SumCheck==Pro_crct)   
	{
//		swgPrt( "RAssitOk \r\n "); 

		ReceAssistMsg.MsgNumber=ReceDat[2];	 								//接收数据字节作为高字�?
		
		ReceAssistMsg.LftEngWaterTemp=ReceDat[3];	 					//接收数据字节作为高字�?
		
		ReceAssistMsg.LftEngSpeedValue=ReceDat[5];	 				//接收数据字节作为高字�?
		ReceAssistMsg.LftEngSpeedValue<<=8;			 						//高字节左�?�?
		ReceAssistMsg.LftEngSpeedValue|=ReceDat[4];	 				//接收数据字节作为低字�?
		
		ReceAssistMsg.LftEngThrotValue=ReceDat[6];	 				//接收数据字节作为高字�?
		ReceAssistMsg.LftEngOilPress=ReceDat[7];	 					//接收数据字节作为低字�?
		ReceAssistMsg.LftEngineState=ReceDat[8];	 					//接收数据字节作为高字�?

		ReceAssistMsg.LftEngFuelRate=ReceDat[10];	 					//接收数据字节作为高字�?
		ReceAssistMsg.LftEngFuelRate<<=8;			 							//高字节左�?�?
		ReceAssistMsg.LftEngFuelRate|=ReceDat[9];	 					//接收数据字节作为低字�?

		ReceAssistMsg.RgtEngWaterTemp=ReceDat[11];	 				//接收数据字节作为高字�?

		ReceAssistMsg.RgtEngSpeedValue=ReceDat[13]; 				//接收数据字节作为高字�?
		ReceAssistMsg.RgtEngSpeedValue<<=8;			 						//高字节左�?�?
		ReceAssistMsg.RgtEngSpeedValue|=ReceDat[12];	 			//接收数据字节作为低字�?
		
		ReceAssistMsg.RgtEngThrotValue=ReceDat[14]; 				//接收数据字节作为高字�?
		ReceAssistMsg.RgtEngOilPress=ReceDat[15];	 					//接收数据字节作为低字�?
		ReceAssistMsg.RgtEngineState=ReceDat[16];	 					//接收数据字节作为高字�?

		ReceAssistMsg.RgtEngFuelRate=ReceDat[18]; 					//接收数据字节作为高字�?
		ReceAssistMsg.RgtEngFuelRate<<=8;			 							//高字节左�?�?
		ReceAssistMsg.RgtEngFuelRate|=ReceDat[17];	 				//接收数据字节作为低字�?

		ReceAssistMsg.LftOilTankVolume=ReceDat[19];	 				//接收数据字节
		ReceAssistMsg.RgtOilTankVolume=ReceDat[20];	 				//接收数据字节

		ReceAssistMsg.BatteryVol_1=ReceDat[21];	 				  	//接收数据字节
		ReceAssistMsg.BatteryVol_2=ReceDat[22];	 				  	//接收数据字节

		ReceAssistMsg.BatteryVol_3=ReceDat[23];	 				  	//接收数据字节
		ReceAssistMsg.BatteryVol_4=ReceDat[24];	 				  	//接收数据字节
		ReceAssistMsg.BatteryVol_5=ReceDat[25];	 				  	//接收数据字节
		ReceAssistMsg.BatteryVol_6=ReceDat[26];	 				  	//接收数据字节
		
		ReceAssistMsg.DeviceState=ReceDat[28];	 						//接收数据字节作为高字�?
		ReceAssistMsg.DeviceState<<=8;			 								//高字节左�?�?
		ReceAssistMsg.DeviceState|=ReceDat[27];	 						//接收数据字节作为低字�?
		
		ReceAssistMsg.TempData=ReceDat[30];	 								//接收数据字节作为高字�?
		ReceAssistMsg.TempData<<=8;			 										//高字节左�?�?
		ReceAssistMsg.TempData|=ReceDat[29];	 							//接收数据字节作为低字�?

		ReceAssistMsg.HumiData=ReceDat[32];	 								//接收数据字节作为高字�?
		ReceAssistMsg.HumiData<<=8;			 										//高字节左�?�?
		ReceAssistMsg.HumiData|=ReceDat[31];	 							//接收数据字节作为低字�?

		ReceAssistMsg.Reserve1=ReceDat[33];	 				  			//接收数据字节
		ReceAssistMsg.Reserve2=ReceDat[34];	 				  			//接收数据字节
		
		CntRxAssist = 40;																		//通信失联时间 33ms x 40 
		AssistOutCommunFlg = 0;
		
		AssistDataExchange();
	}			
}



/*
 * 函数名：AssistDataExchange
 * 描述  ：将接收到的数据转换为要使用的数�?
 * 输入  ：无
 * 输出  ：无	
 */
void AssistDataExchange(void)
{
	//和控制相关赋值一下，其他直接上传至上�?
	g_LftSpeedRealVal = ReceAssistMsg.LftEngSpeedValue;
	g_RgtSpeedRealVal = ReceAssistMsg.RgtEngSpeedValue;
	
	//左发动机上电信号
	switch((ReceAssistMsg.Reserve1 >> 0) & 0x01)
	{
		case 0:
			g_Lft_PowerA_Eng_12V_Flg = 0;
			break;
		case 1:
			g_Lft_PowerA_Eng_12V_Flg = 1;
			break;
	}
	
	//右发动机上电信号
	switch((ReceAssistMsg.Reserve1 >> 1) & 0x01)
	{
		case 0:
			g_Rgt_PowerA_Eng_12V_Flg = 0;
			break;
		case 1:
			g_Rgt_PowerA_Eng_12V_Flg = 1;
			break;
	}
	
//	swgPrt( "R%d ", (int32_t)Uprece_Msg.MsgNumber ); 
}


/*
 * 函数名：Send_AssistData
 * 描述  ：将发送的数据压入发送缓存中
 * 输入  ：无
 * 输出  ：无	
 */
void Send_AssistData(void)
{
//	u8 Data_m=0;	   																			//累加计数

	AssistDataBUF[0]=SendToAssistMsg.SStart1;
	AssistDataBUF[1]=SendToAssistMsg.SStart2;
	AssistDataBUF[2]=SendToAssistMsg.MsgNumber;
	SendToAssistMsg.MsgNumber++;
	AssistDataBUF[3]=SendToAssistMsg.SwitchCmd1;
	AssistDataBUF[4]=SendToAssistMsg.SwitchCmd2;
	AssistDataBUF[5]=SendToAssistMsg.SwitchCmd3;
	AssistDataBUF[6]=SendToAssistMsg.Reserve;
	
	SendToAssistMsg.SumCheck = CheckCrc_8bit((u8*)AssistDataBUF, 7);
	AssistDataBUF[7]=SendToAssistMsg.SumCheck;
	
//	for(Data_m=0;Data_m<8;Data_m++)
//	{
//		USART_SendData(USART3,AssistDataBUF[Data_m]);
//		while(((USART3->SR&0X40)==0))
//			;
//	}
	
	// DMA发�?
	USART3_DMA_send(AssistDataBUF, 8);

//  swgPrt( "T%d ", (int32_t)UpSend_Msg.MsgNumber ); 
}


/*
 * 函数名：AssistDataSendFun
 * 描述  ：发送数据周期函�?
 * 输入  ：无
 * 输出  ：无	
 */
void AssistDataSendFun(void)
{	
	//debug要把这个屏蔽�?
	SendToAssistMsg.SwitchCmd1  = Uprece_Msg.DeviceOnOff_1;
	
	//修改 声光报警器声�? 左滚�?
	if(g_RmtUpManRealMode == RMT_MODE)
	{
		if(Rmtrece_Msg.CH_LD <= 300)  
			SendToAssistMsg.SwitchCmd1 |= (0x03<<0);			//开�?		
		else if(Rmtrece_Msg.CH_LD > 300)  
			SendToAssistMsg.SwitchCmd1 &= (~(0x03<<0));		//关闭
	}	
	
	SendToAssistMsg.SwitchCmd2  = Uprece_Msg.DeviceOnOff_2;
	SendToAssistMsg.SwitchCmd3 |= (Uprece_Msg.DeviceOnOff_3&0x0F);

	//模式
	switch(g_RmtUpManRealMode)
	{
		case RMT_MODE:		SendToAssistMsg.Reserve &= (~(0x03<<0)); 	break;
		case UP_MODE:			SendToAssistMsg.Reserve |= (0x03<<0); 		break;
		case ROCK_MODE:		SendToAssistMsg.Reserve |= (0x03<<0); 		break;
	}
	
	//CH_SB  开�?
	if(Rmtrece_Msg.CH_SB > 1700)
		SendToAssistMsg.Reserve |= (0x03<<2); 
	else if(Rmtrece_Msg.CH_SB < 300)
		SendToAssistMsg.Reserve &= (~(0x03<<2));
	else
	{
		SendToAssistMsg.Reserve &= (~(0x01<<2));
		SendToAssistMsg.Reserve |= (0x01<<3); 
	}
	
	//CH_RD  滚轮
	if(Rmtrece_Msg.CH_RD > 1700)
		SendToAssistMsg.Reserve |= (0x03<<4); 
	else if(Rmtrece_Msg.CH_RD < 300)
		SendToAssistMsg.Reserve &= (~(0x03<<4));
	else
	{
		SendToAssistMsg.Reserve &= (~(0x01<<4));
		SendToAssistMsg.Reserve |= (0x01<<5); 
	}

	Send_AssistData();
}

/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/



