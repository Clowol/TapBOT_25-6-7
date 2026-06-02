/******************** (C) COPYRIGHT 2024 *****************************************
 * 文件名  ：up_data.c
 * 描述    ：处理底层与上层之间的通信函数
 * 硬件配置:              
 * 版本    ： 
 * 修改日期： 
 * 作者    ：
 * 修改日志：
*********************************************************************************/
#include "up_data.h"

//float UpThroValue = 0;						//上层油门控制值  			[0       100]
//float UpNaviValue = 0;						//上层倒斗控制值  			[0 				60]

//float UpLftRudValue = 0;					//上层舵角控制值  			[-30  0  +30]
//float UpLftGearValue = 0;					//上层档位控制值  			[-50  0  +50]
//float UpRgtRudValue = 0;					//上层舵角控制值  			[-30  0  +30]
//float UpRgtGearValue = 0;					//上层档位控制值  			[-50  0  +50]

//float UpHarbLftRgtValue = 0;			//上层海港左右控制值  	[-50  0  +50]
//float UpHarbFwdBwdValue = 0;			//上层海港前后控制值  	[-50  0  +50]

//u8 UpLftEngStartStop = 0xFF;			//上层启动左主机命令 		0-停止  1-启动  0xFF-无效  点触型
//u8 UpRgtEngStartStop = 0xFF;			//上层启动左主机命令 		0-停止  1-启动  0xFF-无效  点触型
//u8 UpGenStartStop = 0xFF;					//上层启动发电机命令 		0-停止  1-启动  0xFF-无效  点触型
//u8 Up_Request = 0;								//有人驾驶/无人驾驶请求 0-无请求      1-有人驾驶请求		2-无人驾驶请求
//u8 UpReceiveMode = RMT_MODE;			//接收上层的模式控制值  为保持一致		1-遥控器模式 			2-手动摇杆驾驶模式  		3-自主模式

//u8 UpModeChg=0;										//海港/常规模式切换 					0x00 无效	0x0F 由当前模式切换到另一模式
//u8 UpModeChg_Old=0;								//海港/常规模式切换上次值 		0x00 无效	0x0F 由当前模式切换到另一模式
//u8 UpModeChgCmd=0;								//海港/常规模式切换指令 			0x00 无效	0x0F 由当前模式切换到另一模式

//u8 UpBotMastCtrl = 0xFF;					//可倒桅杆控制值  01-上升  02-下降  0xFF-无效
//u8 UpBotMastCtrlOld = 0xFF;				//可倒桅杆控制值  01-上升  02-下降  0xFF-无效

//u8 Updata_BUF[100]={0};	        	//定义底层通过串口发送给上层的缓存数组

//struct Rece_Msg Uprece_Msg=	    	//定义接收的数据结构体
//{	
//						 0xEB,				//	u8  RStart1;								//数据帧头 0xEB
//						 0x90,			  //	u8  RStart2;								//数据帧头 0x90
//						 0x72,				//	u8  RStart3;								//数据帧头 0x72
//						 0xC3,		  	//	u8  RStart4;								//数据帧头 0xC3
//							 28,				//	u16 MsgLenth;             	//报文长度
//								0,				//	u32 MsgNumber;	           	//报文帧号
//								0,				//	u16 EngineStartCmd;					//发动机启动指令
//													//															//0X0000	无效
//													//															//0x1111	左右启动 
//													//															//0x2222 	左右关闭
//													//															//0x0011 	右启动
//													//															//0x0022	右关闭
//													//															//0x1100 	左启动
//													//															//0x2200	左关闭
//								0,				//	s16 LftGearCmd;     				//左档位指令，2字节，INT16，单位：%，最低有效位：0.1  ，[-50  	+50] 
//								0,				//	s16 LftRudderCmd;     			//左舵角指令，2字节，INT16，单位：°，最低有效位：0.25°，[-30   	+30]
//                0,				//	s16 RgtGearCmd;     				//右档位指令，2字节，INT16，单位：%，最低有效位：0.1  ，[-50  	+50]
//          	    0,				//	s16 RgtRudderCmd;     			//右舵角指令，2字节，INT16，单位：%，最低有效位：0.1  ，[-50  	+50]	
//								1,				//	u8	RState;									//工作模式设置指令， 1字节， UINT8，1-遥控器模式 	2-上层模式  		3-海港摇杆模式
//								0,				//	u8 	RRequest;								//有人/无人请求信号，1字节， UINT8，0-无请求      1-有人驾驶请求			2-无人驾驶请求
//								0,				//	u8  DeviceOnOff_1;					//设备启动/停止1，   1字节， UINT8
//								0,				//	u8  DeviceOnOff_2;					//设备启动/停止2，   1字节， UINT8
//								0,				//	u8  DeviceOnOff_3;					//设备启动/停止3，   1字节， UINT8
//								0,				//	u8 	Reserve1;              	//保留位1，					 1字节， UINT8
//								0,				//	u16 SumCheck;     					//校验和 前面所有数值按16位无符号书值累加和
//};


//struct Send_Msg	UpSend_Msg=			  //定义发送的数据结构体
//{	
//							0xEB,				//	u8	SStart1;								//数据帧头 0xEB
//							0x90,				//	u8  SStart2;								//数据帧头 0x90
//							0x72,				//	u8  SStart3;								//数据帧头 0x72
//							0xC4,				//	u8  SStart4;								//数据帧头 0xC4
//								60,       //  u16 MsgLenth;               //报文长度
//								 0, 			//	u32 MsgNumber;					    //报文帧号 00000000H~FFFFFFFFH的循环数
//				  	20/0.1,				//	s16	LeftRudderValue;    		//左舵角值，			1字节， INT16，单位：°，最低有效位：0.1°，[-30° - +30°] 
//					 -10/0.1,       //	s16	RightRudderValue;     	//右舵角值，			1字节， INT16，单位：°，最低有效位：0.1°，[-30° - +30°] 
//								15,       //	u16	LeftNaviValue;    			//左斗角值，			1字节，UINT16，单位：°，最低有效位：0.1°，[  0° - 45° ] 
//							  45,       //	u16	RightNaviValue;   			//右斗角值，			1字节，UINT16，单位：°，最低有效位：0.1°，[  0° - 45° ] 
//								 0,       //	u16 PumpAlarmInform;				//喷泵报警信息，	2字节，UINT16
//								80,       //	s8 	LftEngWaterTemp;				//左发动机水温值, 1字节，  INT8，单位： ℃，最低有效位：   1，[-30 - +100] ℃
//					 1000/20,       //	u8  LftEngSpeedValue;				//左发动机转速值, 1字节，	UINT8，单位：RPM，最低有效位：  20，[  0 - 4000]RPM 	
//						30/0.1,       //	u16 LftEngThrotValue;				//左主机油门值,   2字节，UINT16，单位：  %，最低有效位： 0.1，[  0 -  100]  %
//								 5,				//	u8	LftEngOilPress;					//左发动机油压值, 1字节，	UINT8，单位：kPa，最低有效位：	 4，[  0 - 1000]kPa
//								 2,       //	u8  LftEngineState;					//左发动机状态,   1字节， UINT8， 
//								 0,				//	u16 LftEngFuelRate;		  		//左发动机油耗,   2字节，UINT16，单位：L/h，最低有效位：0.05，[ 0 - 3212.75]L/h
//								60,       //	s8 	RgtEngWaterTemp;				//右发动机水温值, 1字节，  INT8，单位： ℃，最低有效位：   1，[-30 - +100] ℃
//					 3000/20,       //	u8	RgtEngSpeedValue;				//右发动机转速值, 1字节，	UINT8，单位：RPM，最低有效位：  20，[  0 - 4000]RPM 	
//						40/0.1,       //	u16 RgtEngThrotValue;				//右主机油门值,   2字节，UINT16，单位：  %，最低有效位： 0.1，[  0 -  100]  %
//							 	 5,				//	u8	RgtEngOilPress;					//右发动机油压值, 1字节，	UINT8，单位：kPa，最低有效位：	 4，[  0 - 1000]kPa
//								 4,       //	u8  RgtEngineState;					//右发动机状态,   1字节， UINT8， 
//								 0,				//	u16 RgtEngFuelRate;		  		//右发动机油耗,   2字节，UINT16，单位：L/h，最低有效位：0.05，[ 0 - 3212.75]L/h
//								80,       //	u8 	LftOilTankVolume;				//左油箱油量，		1字节， UINT8，单位：  %，最低有效位：   1，[  0 -  100]  %
//								70,       //	u8 	RgtOilTankVolume;				//右油箱油量，		1字节， UINT8，单位：  %，最低有效位：   1，[  0 -  100]  %
//								 0,				//	u16 EngineStartCmd_Sd;			//发动机启动指令  2字节
//								 0,				//	s16 LftGearCmd_Sd;     			//左档位指令，2字节，INT16，单位：%，最低有效位：0.1  ，[-50  	+50] 
//								 0,				//	s16 LftRudderCmd_Sd;     		//左舵角指令，2字节，INT16，单位：°，最低有效位：0.1° ，[-30   	+30] 
//								 0,				//	s16 RgtGearCmd_Sd;     			//右档位指令，2字节，INT16，单位：%，最低有效位：0.1  ，[-50  	+50] 
//								 0,				//	s16 RgtRudderCmd_Sd;     		//右舵角指令，2字节，INT16，单位：°，最低有效位：0.1° ，[-30   	+30]
//								 0,				//	u8  DeviceOnOff_1_Sd;				//设备启动/停止1，   1字节， UINT8
//								 0,				//	u8  DeviceOnOff_2_Sd;				//设备启动/停止2，   1字节， UINT8
//								 0,				//	u8  DeviceOnOff_3_Sd;				//设备启动/停止3，   1字节， UINT8
//								 0,				//	u8	RState_sd;							//工作模式设置指令， 1字节， UINT8，1-遥控器模式 	2-摇杆驾驶模式	3-航行自主模式	
//								 2,       //	u8 	LLCModeState;						//工作模式，      	 1字节， UINT8，1-有人驾驶  	2-遥控器模式  	3-手动摇杆模式  4-航行自主模式
//								 1,      	//  u8 	RmtOutComState;         //遥控器失联报警，	 1字节，UINT8，底层与遥控器连通  0x01  地层与遥控器失联 0x00

//						12/0.1,				//	u8  BattVal_LftEngStart; 		//12V左发动机启动电池， 1字节， UINT8，单位：V，最低有效位：0.1，[0  	14.3]   
//						12/0.1,				//	u8  BattVal_RgtEngStart;    //12V右发动机启动电池， 1字节， UINT8，单位：V，最低有效位：0.1，[0  	14.3]  
//						12/0.1,				//  u8  BattVal_GenStart;    		//12V发电机启动电池，		1字节， UINT8，单位：V，最低有效位：0.1，[0  	14.3]  
//				(24-5)/0.1,				//  u8  BattVal_Mast;        		//24V桅杆电池，				 	1字节， UINT8，单位：V，最低有效位：0.1，[5  	  30]		=5V即0V
//				(24-5)/0.1,				//	u8  BattVal_Daly;         	//24V日用电池，					1字节， UINT8，单位：V，最低有效位：0.1，[5  	  30]		=5V即0V
//				(24-5)/0.1,				//	u8  BattVal_Reserve;        //24V预留电池，					1字节， UINT8，单位：V，最低有效位：0.1，[5  	  30]		=5V即0V
//						
//		 						 0,      	//  u16	AssistCtrlState;        //辅助控制板状态，2字节，UINT16
//								 0,				//	s16	TempData;								//温度，					2字节， INT16
//								 0,				//	u16	HumiData;								//湿度，				 	2字节，UINT16
//								 0,       //	u16	SumCheck;    						//校验和 前面所有数值按16位无符号书值累加和
//};
//					

///*
// * 函数名：CheckSum
// * 描述  ：计算校验和
// * 输入  ：
// * 输出  ：无	
// */
//static u16 CheckSum(u16 * pusBuff, int iLen)
//{
//    u16 usSum;
//    int i;
//    usSum = 0;
//    for (i = 1; i <=iLen; i++)
//    {
//        usSum = usSum +(u16)*pusBuff;
//        pusBuff++;
//    }
//    
//    return usSum;
//}


///*
// * 函数名：Proce_UpData
// * 描述  ：解析处理智能信息层发送到底层的控制命令
// * 输入  ：ReceDat，接收到的数据数组
// * 输出  ：无	
// */
//void Proce_UpData(char *ReceDat)
//{
//	u16 Pro_crct=0;  																		//校验和

//	Uprece_Msg.RStart1=ReceDat[0];	    								//接收数据字节0 帧头 数据帧头 0xEB
//	Uprece_Msg.RStart2=ReceDat[1];	    								//接收数据字节0 帧头 数据帧头 0x90
//	Uprece_Msg.RStart3=ReceDat[2];	    								//接收数据字节0 帧头 数据帧头 0x72
//	Uprece_Msg.RStart4=ReceDat[3];	    								//接收数据字节0 帧头 数据帧头 0xC3

//	Uprece_Msg.SumCheck=ReceDat[27];	 						   		//接收数据字节作为高字节
//	Uprece_Msg.SumCheck<<=8;			 								    	//高字节左移8位
//	Uprece_Msg.SumCheck|=ReceDat[26];	 					    		//接收数据字节作为低字节

//  Pro_crct = CheckSum((u16*)ReceDat, 13);
//	
//	
////	swgPrt("\r\n Uprece_Msg.SumCheck = %x \r\n", Uprece_Msg.SumCheck ); 
////	swgPrt("\r\n Pro_crct = %x \r\n", Pro_crct ); 
//	
//	//帧头及校验位同时满足
//	if(Uprece_Msg.RStart1==0xEB && Uprece_Msg.RStart2==0x90 && Uprece_Msg.RStart3==0x72 \
//	&& Uprece_Msg.RStart4==0xC3 && Uprece_Msg.SumCheck==Pro_crct)   
//	{
////	  swgPrt( "ReceUpOk \r\n "); 

//		Uprece_Msg.MsgLenth=ReceDat[5];	 									//接收数据字节作为高字节
//		Uprece_Msg.MsgLenth<<=8;			 										//高字节左移8位
//		Uprece_Msg.MsgLenth|=ReceDat[4];	 								//接收数据字节作为低字节
//		
//		Uprece_Msg.MsgNumber=ReceDat[9];	 								//接收数据字节作为高字节
//		Uprece_Msg.MsgNumber<<=8;			 										//高字节左移8位
//		Uprece_Msg.MsgNumber|=ReceDat[8];	 								//接收数据字节作为高字节
//		Uprece_Msg.MsgNumber<<=8;			 										//高字节左移8位
//		Uprece_Msg.MsgNumber|=ReceDat[7];	 								//接收数据字节作为高字节
//		Uprece_Msg.MsgNumber<<=8;			 										//高字节左移8位
//		Uprece_Msg.MsgNumber|=ReceDat[6];	 								//接收数据字节作为低字节
//		
////		swgPrt("UP_n=%d\r\n", Uprece_Msg.MsgNumber ); 

//		Uprece_Msg.EngineStartCmd=ReceDat[11];	 					//接收数据字节作为高字节
//		Uprece_Msg.EngineStartCmd<<=8;			 							//高字节左移8位
//		Uprece_Msg.EngineStartCmd|=ReceDat[10];	 					//接收数据字节作为低字节

//		Uprece_Msg.LftGearCmd=ReceDat[13];	 									//接收数据字节作为高字节
//		Uprece_Msg.LftGearCmd<<=8;			 											//高字节左移8位
//		Uprece_Msg.LftGearCmd|=ReceDat[12];	 								//接收数据字节作为低字节

//		Uprece_Msg.LftRudderCmd=ReceDat[15];	 								//接收数据字节作为高字节
//		Uprece_Msg.LftRudderCmd<<=8;			 										//高字节左移8位
//		Uprece_Msg.LftRudderCmd|=ReceDat[14];	 							//接收数据字节作为低字节
//		
//		Uprece_Msg.RgtGearCmd=ReceDat[17];	 									//接收数据字节作为高字节
//		Uprece_Msg.RgtGearCmd<<=8;			 											//高字节左移8位
//		Uprece_Msg.RgtGearCmd|=ReceDat[16];	 								//接收数据字节作为低字节

//		Uprece_Msg.RgtRudderCmd=ReceDat[19];	 								//接收数据字节作为高字节
//		Uprece_Msg.RgtRudderCmd<<=8;			 										//高字节左移8位
//		Uprece_Msg.RgtRudderCmd|=ReceDat[18];	 							//接收数据字节作为低字节

//		Uprece_Msg.RState=ReceDat[20];	 									//接收数据字节作为低字节
//		Uprece_Msg.RRequest=ReceDat[21];	 								//接收数据字节作为低字节

//		Uprece_Msg.DeviceOnOff_1=ReceDat[22];	 				  	//接收数据字节
//		Uprece_Msg.DeviceOnOff_2=ReceDat[23];	 				  	//接收数据字节
//		Uprece_Msg.DeviceOnOff_3=ReceDat[24];	 				  	//接收数据字节
//		
//		Uprece_Msg.Reserve1=ReceDat[25];	 				  			//接收数据字节
//		
//		CntRxUp = 20;																			//通信失联时间 50ms x 20 = 1s 
//		
//		UpOutCommunFlg = 0;
//		
//		UpDataExchange();
//	}			
//}


///*
// * 函数名：UpDataExchange
// * 描述  ：将接收到的数据转换为要使用的数值
// * 输入  ：无
// * 输出  ：无	
// */
//void UpDataExchange(void)
//{
//	//左发动机
//	switch(Uprece_Msg.EngineStartCmd & 0xFF00)
//	{  
//		case 0x0000:	//无效
//			UpLftEngStartStop = 0xFF;
//			break;
//		case 0x1100:	//启动
//			UpLftEngStartStop = 1;  
//			swgPrt("UpLftEngStart\r\n");
//			break;
//		case 0x2200:	//停止
//			UpLftEngStartStop = 0;
//			swgPrt("UpLftEngStop\r\n");
//			break;	
//	}

//	//右发动机
//	switch(Uprece_Msg.EngineStartCmd & 0x00FF)
//	{  
//		case 0x0000:	//无效
//			UpRgtEngStartStop = 0xFF;
//			break;
//		case 0x0011: 
//			UpRgtEngStartStop = 1;
//			swgPrt("UpRgtEngStart\r\n");
//			break;
//		case 0x0022:
//			UpRgtEngStartStop = 0;
//			swgPrt("UpRgtEngStop\r\n");
//			break;
//	}
//	
//	//发电机
//	switch((Uprece_Msg.DeviceOnOff_3 >> 4) & 0x03)
//	{
//		case 0x01:	//无效
//			UpGenStartStop = 0xFF;
//			break;
//		case 0x02:	//无效
//			UpGenStartStop = 0xFF;
//			break;
//		case 0x03: 
//			UpGenStartStop = 1;
//			swgPrt("UpGenStart\r\n");
//			break;
//		case 0x00:
//			UpGenStartStop = 0;
//			swgPrt("UpGenStop\r\n");
//			break;
//	}

//	//用swicth防止对应错误
//	switch(Uprece_Msg.RState)
//	{  
//		case 1:	UpReceiveMode = RMT_MODE;
//		break;
//		case 2: UpReceiveMode = ROCK_MODE;	
//		break;
//		case 3:	UpReceiveMode = UP_MODE;		
//		break;
//		default:
//		break;
//	}
//	
//	UpLftGearValue = (float)Uprece_Msg.LftGearCmd*0.1/2;
//	UpLftRudValue = (float)Uprece_Msg.LftRudderCmd*0.1;
//	UpRgtGearValue = (float)Uprece_Msg.RgtGearCmd*0.1/2;
//	UpRgtRudValue = (float)Uprece_Msg.RgtRudderCmd*0.1;
//}




///*
// * 函数名：Send_Updata
// * 描述  ：将发送给智能层的数据压入发送缓存中
// * 输入  ：无
// * 输出  ：无	
// */
//void Send_Updata(void)
//{
////	u8 Data_m=0;	   																			//累加计数
//	u16 Data_crct=0;   																		//校验累加位 

//	Updata_BUF[0]=UpSend_Msg.SStart1;
//	Updata_BUF[1]=UpSend_Msg.SStart2;
//	Updata_BUF[2]=UpSend_Msg.SStart3;
//	Updata_BUF[3]=UpSend_Msg.SStart4;
//	
//	Updata_BUF[4]=LBT(UpSend_Msg.MsgLenth);
//	Updata_BUF[5]=HBT(UpSend_Msg.MsgLenth);
//	
//	Updata_BUF[6]=BYTE0(UpSend_Msg.MsgNumber);
//	Updata_BUF[7]=BYTE1(UpSend_Msg.MsgNumber);
//	Updata_BUF[8]=BYTE2(UpSend_Msg.MsgNumber);
//	Updata_BUF[9]=BYTE3(UpSend_Msg.MsgNumber);
//	UpSend_Msg.MsgNumber++;
//	
//	/* 喷泵状态 */
//	Updata_BUF[10]=LBT(UpSend_Msg.LeftRudderValue);		 
//	Updata_BUF[11]=HBT(UpSend_Msg.LeftRudderValue);
//	
//	Updata_BUF[12]=LBT(UpSend_Msg.RightRudderValue);		 
//	Updata_BUF[13]=HBT(UpSend_Msg.RightRudderValue);
//	
//	Updata_BUF[14]=LBT(UpSend_Msg.LeftNaviValue);		 
//	Updata_BUF[15]=HBT(UpSend_Msg.LeftNaviValue);
//	
//	Updata_BUF[16]=LBT(UpSend_Msg.RightNaviValue);		 
//	Updata_BUF[17]=HBT(UpSend_Msg.RightNaviValue);

//	Updata_BUF[18]=LBT(UpSend_Msg.PumpAlarmInform);
//	Updata_BUF[19]=HBT(UpSend_Msg.PumpAlarmInform);

//	/* 主机状态 左 */
//	Updata_BUF[20]=UpSend_Msg.LftEngWaterTemp;
//  Updata_BUF[21]=UpSend_Msg.LftEngSpeedValue;
//	
//	Updata_BUF[22]=LBT(UpSend_Msg.LftEngThrotValue);
//	Updata_BUF[23]=HBT(UpSend_Msg.LftEngThrotValue);

//	Updata_BUF[24]=UpSend_Msg.LftEngOilPress;
//	
//	Updata_BUF[25]=UpSend_Msg.LftEngineState;

//	Updata_BUF[26]=LBT(UpSend_Msg.LftEngFuelRate);
//	Updata_BUF[27]=HBT(UpSend_Msg.LftEngFuelRate);

//	/* 主机状态 右 */
//	Updata_BUF[28]=UpSend_Msg.RgtEngWaterTemp;

//	Updata_BUF[29]=UpSend_Msg.RgtEngSpeedValue;

//	Updata_BUF[30]=LBT(UpSend_Msg.RgtEngThrotValue);
//	Updata_BUF[31]=HBT(UpSend_Msg.RgtEngThrotValue);

//	Updata_BUF[32]=UpSend_Msg.RgtEngOilPress;
//	
//	Updata_BUF[33]=UpSend_Msg.RgtEngineState;

//	Updata_BUF[34]=LBT(UpSend_Msg.RgtEngFuelRate);
//	Updata_BUF[35]=HBT(UpSend_Msg.RgtEngFuelRate);

//	/* 油箱 */
//	Updata_BUF[36]=UpSend_Msg.LftOilTankVolume;
//	Updata_BUF[37]=UpSend_Msg.RgtOilTankVolume;
//	
//	/* 喷泵和主机控制值 */
//	Updata_BUF[38]=LBT(UpSend_Msg.EngineStartCmd_Sd);
//	Updata_BUF[39]=HBT(UpSend_Msg.EngineStartCmd_Sd);
//	
//	Updata_BUF[40]=LBT(UpSend_Msg.LftGearCmd_Sd);
//	Updata_BUF[41]=HBT(UpSend_Msg.LftGearCmd_Sd);
//	
//	Updata_BUF[42]=LBT(UpSend_Msg.LftRudderCmd_Sd);
//	Updata_BUF[43]=HBT(UpSend_Msg.LftRudderCmd_Sd);
//	
//	Updata_BUF[44]=LBT(UpSend_Msg.RgtGearCmd_Sd);
//	Updata_BUF[45]=HBT(UpSend_Msg.RgtGearCmd_Sd);
//	
//	Updata_BUF[46]=LBT(UpSend_Msg.RgtRudderCmd_Sd);
//	Updata_BUF[47]=HBT(UpSend_Msg.RgtRudderCmd_Sd);
//	
//	Updata_BUF[48]=UpSend_Msg.DeviceOnOff_1_Sd;
//	Updata_BUF[49]=UpSend_Msg.DeviceOnOff_2_Sd;
//	
//	Updata_BUF[50]=UpSend_Msg.DeviceOnOff_3_Sd;
//	Updata_BUF[51]=UpSend_Msg.RState_sd;
///*工作模式*/
//	Updata_BUF[52]=UpSend_Msg.LLCModeState;
//	
///*遥控器失联 */	
//	Updata_BUF[53]=UpSend_Msg.RmtOutComState;
//	
///* 电池电压  */
//	Updata_BUF[54]=UpSend_Msg.BattVal_LftEngStart;
//	Updata_BUF[55]=UpSend_Msg.BattVal_RgtEngStart;
//	Updata_BUF[56]=UpSend_Msg.BattVal_GenStart;
//	Updata_BUF[57]=UpSend_Msg.BattVal_Mast;
//	Updata_BUF[58]=UpSend_Msg.BattVal_Daly;
//	Updata_BUF[59]=UpSend_Msg.BattVal_Reserve;
//	
//	Updata_BUF[60]=LBT(UpSend_Msg.AssistCtrlState);
//	Updata_BUF[61]=HBT(UpSend_Msg.AssistCtrlState);

//	Updata_BUF[62]=LBT(UpSend_Msg.TempData);
//	Updata_BUF[63]=HBT(UpSend_Msg.TempData);

//	Updata_BUF[64]=LBT(UpSend_Msg.HumiData);
//	Updata_BUF[65]=HBT(UpSend_Msg.HumiData);

//  Data_crct = CheckSum((u16*)Updata_BUF, 33);
//  UpSend_Msg.SumCheck=Data_crct;

//	Updata_BUF[66]=LBT(UpSend_Msg.SumCheck);
//	Updata_BUF[67]=HBT(UpSend_Msg.SumCheck); 

////	for(Data_m=0;Data_m<68;Data_m++)
////	{
////		USART_SendData(USART2,Updata_BUF[Data_m]);
////		while(((USART2->SR&0X40)==0))
////			;
////	}

////	swgPrt("\r\n Up Send: \r\n"); 
////	for(Data_m=0;Data_m<68;Data_m++)
////	{
////		swgPrt("%X ", Updata_BUF[Data_m]); 
////	}
////	swgPrt("\r\n"); 	

//	// DMA发送
//	USART2_DMA_send(Updata_BUF, 68);

////  swgPrt( "T%d ", (int32_t)UpSend_Msg.MsgNumber ); 
//}


///*
// * 函数名：UpDataSendFun
// * 描述  ：发送上层数据周期函数 
// * 输入  ：无
// * 输出  ：无	
// */
//void UpDataSendFun(void)
//{	
////	static s16 temp = 0;
////	static s8 IO_GearValue = 0;

//	UpSend_Msg.LeftRudderValue = (s16)(g_fLftRudRealVal*10);										
//	UpSend_Msg.RightRudderValue = (s16)(g_fRgtRudRealVal*10);

//	UpSend_Msg.LeftNaviValue = (u16)(g_fLftNaviRealVal*10);
//	UpSend_Msg.RightNaviValue = (u16)(g_fRgtNaviRealVal*10);
//	
//	switch(PumpMotorAlsONFlg)
//	{
//		case 0:	UpSend_Msg.PumpAlarmInform &= (~(0x01<<0));
//		case 1: UpSend_Msg.PumpAlarmInform |= (0x01<<0); 
//	}

//	UpSend_Msg.LftEngWaterTemp = ReceAssistMsg.LftEngWaterTemp;			
//	UpSend_Msg.LftEngSpeedValue = ReceAssistMsg.LftEngSpeedValue/20;
//	UpSend_Msg.LftEngThrotValue = ReceAssistMsg.LftEngThrotValue;			
//	UpSend_Msg.LftEngOilPress = ReceAssistMsg.LftEngOilPress;			
//	UpSend_Msg.LftEngineState = ReceAssistMsg.LftEngineState;			
//	UpSend_Msg.LftEngFuelRate = ReceAssistMsg.LftEngFuelRate;			

//	UpSend_Msg.RgtEngWaterTemp = ReceAssistMsg.RgtEngWaterTemp;			
//	UpSend_Msg.RgtEngSpeedValue = ReceAssistMsg.RgtEngSpeedValue/20;
//	UpSend_Msg.RgtEngThrotValue = ReceAssistMsg.RgtEngThrotValue;			
//	UpSend_Msg.RgtEngOilPress = ReceAssistMsg.RgtEngOilPress;			
//	UpSend_Msg.RgtEngineState = ReceAssistMsg.RgtEngineState;			
//	UpSend_Msg.RgtEngFuelRate = ReceAssistMsg.RgtEngFuelRate;			
//	
//	UpSend_Msg.LftOilTankVolume = ReceAssistMsg.LftOilTankVolume;		//油箱油量
//	UpSend_Msg.RgtOilTankVolume = ReceAssistMsg.RgtOilTankVolume;		//油箱油量

//	//用swicth防止对应错误
//	switch(g_RmtUpManRealMode)
//	{
//		case MAN_MODE: 	UpSend_Msg.LLCModeState = 1;	break;		//有人驾驶
//		case RMT_MODE: 	UpSend_Msg.LLCModeState = 2;	break;		//遥控器
//		case ROCK_MODE: UpSend_Msg.LLCModeState = 3;	break;		//手动摇杆
//		case UP_MODE: 	UpSend_Msg.LLCModeState = 4;	break;		//航行自主
//	}
//  
//	switch(g_RmtUpManRealMode)
//	{
//		case UP_MODE:			//如果是上层模式  返回上层控制指令
//			UpSend_Msg.EngineStartCmd_Sd = Uprece_Msg.EngineStartCmd;
//			UpSend_Msg.LftGearCmd_Sd = Uprece_Msg.LftGearCmd;
//			UpSend_Msg.LftRudderCmd_Sd = Uprece_Msg.LftRudderCmd;
//			UpSend_Msg.RgtGearCmd_Sd = Uprece_Msg.RgtGearCmd;
//			UpSend_Msg.RgtRudderCmd_Sd = Uprece_Msg.RgtRudderCmd;
//			break;
//		case RMT_MODE:
////			UpSend_Msg.EngineStartCmd_Sd = Rmtrece_Msg.DeviceOnOff;
//			UpSend_Msg.LftGearCmd_Sd = (s16)(RmtGearValue *10);
//			UpSend_Msg.LftRudderCmd_Sd = (s16)(RmtRudValue*10);
//			UpSend_Msg.RgtGearCmd_Sd = (s16)(RmtGearValue*10);
//			UpSend_Msg.RgtRudderCmd_Sd = (s16)(RmtRudValue*10);
//			break;
//		case MAN_MODE:
//			UpSend_Msg.EngineStartCmd_Sd = Uprece_Msg.EngineStartCmd;
//			UpSend_Msg.LftGearCmd_Sd = 0;
//			UpSend_Msg.LftRudderCmd_Sd = 0;
//			UpSend_Msg.RgtGearCmd_Sd = 0;
//			UpSend_Msg.RgtRudderCmd_Sd = 0;
//			break;
//		case ROCK_MODE:
//			UpSend_Msg.EngineStartCmd_Sd = Uprece_Msg.EngineStartCmd;
//			UpSend_Msg.LftGearCmd_Sd = ManRock_Msg.LftGearCmd;
//			UpSend_Msg.LftRudderCmd_Sd = ManRock_Msg.LftRudderCmd;
//			UpSend_Msg.RgtGearCmd_Sd = ManRock_Msg.RgtGearCmd;
//			UpSend_Msg.RgtRudderCmd_Sd = ManRock_Msg.RgtRudderCmd;
//			break;
//	}

//	UpSend_Msg.DeviceOnOff_1_Sd = Uprece_Msg.DeviceOnOff_1;
//	UpSend_Msg.DeviceOnOff_2_Sd = Uprece_Msg.DeviceOnOff_2;
//	UpSend_Msg.DeviceOnOff_3_Sd = Uprece_Msg.DeviceOnOff_3;
//	UpSend_Msg.RState_sd = Uprece_Msg.RState;

//	if(RmtOutCommunFlg == 1)
//	{
//		UpSend_Msg.RmtOutComState = 0;
//	}
//	else if(RmtOutCommunFlg == 0)
//	{
//		UpSend_Msg.RmtOutComState = 1;
//	}

//	UpSend_Msg.BattVal_LftEngStart = ReceAssistMsg.BatteryVol_1;
//	UpSend_Msg.BattVal_RgtEngStart = ReceAssistMsg.BatteryVol_2;
//	UpSend_Msg.BattVal_GenStart    = ReceAssistMsg.BatteryVol_3;
//	UpSend_Msg.BattVal_Mast        = ReceAssistMsg.BatteryVol_5;
//	UpSend_Msg.BattVal_Daly        = ReceAssistMsg.BatteryVol_4;
//	UpSend_Msg.BattVal_Reserve     = ReceAssistMsg.BatteryVol_6;

//	//bit0-bit11
//	UpSend_Msg.AssistCtrlState = (ReceAssistMsg.DeviceState & 0x0FFF);
//	
//	//发电机 bit12
//	switch(g_GenSttStpState)
//	{
//		case 0:	UpSend_Msg.AssistCtrlState &= (~(0x01<<12)); 	break;
//		case 1:	UpSend_Msg.AssistCtrlState |= (0x01<<12); 		break;
//	}
//	
//	UpSend_Msg.TempData = ReceAssistMsg.TempData;
//	UpSend_Msg.HumiData = ReceAssistMsg.HumiData;
//	
//	Send_Updata();
//}

/******************* (C) COPYRIGHT 2024 END OF FILE *****************************/



