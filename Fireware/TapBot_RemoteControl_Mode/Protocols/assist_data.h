/******************** (C) COPYRIGHT 2018 *****************************************
 * 文件名  ：assist_data.h
 * 描述    ：处理主板与辅助板之间的通信函数
 * 硬件配置：         
 * 版本    ： 
 * 修改日期： 
 * 作者    ： 
 * 修改日志：
*********************************************************************************/
#ifndef __ASSIST_DATA_H
#define __ASSIST_DATA_H

#include "stm32f10x.h"
#include "timer.h"
#include "function.h"



/* 辅助板 到 主板 信息 */
struct ReceAssist_Msg					//定义 辅助板 到 主板 数据结构体
{
	u8	RStart1;								// 0  数据帧头 0xA8
	u8  RStart2;								// 1  数据帧头 0x99
	u8 	MsgNumber;					   	// 2  通信确认 0x00-FF的循环数
	
	s8 	LftEngWaterTemp;				// 3  左发动机水温值, 1字节，  INT8，单位： ℃，最低有效位：   1，[-30 - +100] ℃
	u16 LftEngSpeedValue;				// 4  左发动机转速值, 2字节，UINT16，单位：RPM，最低有效位：   1，[  0 - 5000]RPM 	
	u8 	LftEngThrotValue;				// 6  左主机油门值,   1字节， UINT8，单位：  %，最低有效位：   1，[  0 -  100]  %
	u8 	LftEngOilPress;					// 7  左发动机油压值, 1字节， UINT8，单位：kPa，最低有效位：   4，[  0 - 1000]kPa
	u8  LftEngineState;					// 8  左发动机状态,   1字节， UINT8， 
	u16 LftEngFuelRate;		    	// 9  左发动机油耗,   2字节，UINT16，单位：L/h，最低有效位：0.05，[0 - 3212.75]L/h

	s8 	RgtEngWaterTemp;				// 11 右发动机水温值, 1字节，  INT8，单位： ℃，最低有效位：   1，[-30 - +100] ℃
	u16	RgtEngSpeedValue;				// 12 右发动机转速值, 2字节，UINT16，单位：RPM，最低有效位：  1，[  0 - 5000]RPM 	
	u8  RgtEngThrotValue;				// 14 右主机油门值,   1字节， UINT8，单位：  %，最低有效位：  1，[  0 -  100]  %
	u8  RgtEngOilPress;					// 15 右发动机油压值, 1字节， UINT8，单位：kPa，最低有效位：   4，[  0 - 1000]kPa
	u8  RgtEngineState;					// 16 右发动机状态,   1字节， UINT8， 
	u16 RgtEngFuelRate;		    	// 17 右发动机油耗,   2字节，UINT16，单位：L/h，最低有效位：0.05，[0 - 3212.75]L/h 
	
	u8	LftOilTankVolume;				// 19 左油量反馈			1字节， UINT8，单位：  %，最低有效位：   1，[  0 -  100]  %
	u8	RgtOilTankVolume;				// 20 右油量反馈			1字节， UINT8，单位：  %，最低有效位：   1，[  0 -  100]  %

	u8	BatteryVol_1;						// 21 电池1 	1字节， UINT8，单位：  V，最低有效位： 0.1，[  0 - 14.3]  V
	u8	BatteryVol_2;						// 22 电池2 	1字节， UINT8，单位：  V，最低有效位： 0.1，[  0 - 14.3]  V
	u8	BatteryVol_3;						// 23 电池3 	1字节， UINT8，单位：  V，最低有效位： 0.1，[  0 - 14.3]  V
	u8	BatteryVol_4;						// 24 电池4 	1字节， UINT8，单位：  V，最低有效位： 0.1，[  5 - 30.3]  V   0V=5V
	u8	BatteryVol_5;						// 25 电池5 	1字节， UINT8，单位：  V，最低有效位： 0.1，[  5 - 30.3]  V		0V=5V
	u8	BatteryVol_6;						// 26 电池6 	1字节， UINT8，单位：  V，最低有效位： 0.1，[  5 - 30.3]  V		0V=5V
	
	u16 DeviceState;					  // 27 bit0-15 
																		//bit0	机舱风机							
																		//bit1	管道风机
																		//bit2	浮子开关1
																		//bit3	浮子开关2
																		//bit4	浮子开关3
																		//bit5	浮子开关4
																		//bit6	温度感应器
																		//bit7  烟雾感应器
																		//bit8	接近开关1  桅杆倒下
																		//bit9	接近开关2  桅杆立起
																		//bit10	接近开关3  声呐收起	
																		//bit11	接近开关4  声呐下放
																		//bit12	预留
																		//bit13	预留
																		//bit14	预留
																		//bit15	预留

	s16	TempData;								// 29 温度，					2字节， INT16
	u16	HumiData;								// 31 湿度，				 	2字节，UINT16
	u8	Reserve1;								// 33 预留位1，				1字节，UINT8
	u8	Reserve2;								// 34 预留位2，				1字节，UINT8
	u8	SumCheck;    						// 35 校验和
};
extern struct ReceAssist_Msg ReceAssistMsg;  


/* 主板 到 辅助板 指令 */
struct  SendToAssist_Msg       	//定义底层 主板 到 辅助板  数据结构体
{
	u8 	SStart1;								// 0  数据帧头 0xA8
	u8 	SStart2;								// 1  数据帧头 0x99	
	u8 	MsgNumber;	           	// 2  通信确认
	u8 	SwitchCmd1;							// 3  开关命令1	
																		//bit7-bit6	桅灯				11-开 		00-关    	01 10-无效
																		//bit5-bit4	艉灯   			11-开 		00-关    	01 10-无效
																		//bit3-bit2	左舷  			11-开 		00-关    	01 10-无效
																		//bit1-bit0	右舷    		11-开 		00-关    	01 10-无效
	u8 	SwitchCmd2;             // 4  开关命令2
																		//bit7			预留急停
																		//bit6			预留
																		//bit5-bit4	预留
																		//bit3-bit2	声呐 				01-收起 	10-下放 	00 11-无效
																		//bit1-bit0	可倒桅   		01-上升 	10-下降  	00 11-无效
	u8 	SwitchCmd3;							// 5  开关命令3
																		//bit7-bit6	预留		
																		//bit5-bit4	预留   		
																		//bit3-bit2	管道风机  	11-开 		00-关    	01 10-无效
																		//bit1-bit0	机舱风机  	11-开 		00-关    	01 10-无效
	u8 	Reserve;								// 6  预留
	u8 	SumCheck;     					// 7  校验和  
};
extern struct SendToAssist_Msg SendToAssistMsg, bkpSendToAssist_Msg;


extern u8 AssistBotMastCtrl;			//可倒桅杆控制值  01-上升  02-下降  0xFF-无效

extern u8 g_u8OilTankVolume;						//油箱油量
extern u8 g_u8BattPercent_1;						//电池电量 01 底层电池 百分比  0-100  最小单位10
extern u8 g_u8BattPercent_2;						//电池电量 02 启动电池 百分比  0-100  最小单位10
extern u8 g_u8RoomTemp;									//房间温度


extern u8 AssistDataBUF[100];

void Proce_AssistData(char *ReceDat);
void AssistDataExchange(void);
void Send_AssistData(void);
void AssistDataSendFun(void);


#endif

/******************* (C) COPYRIGHT 2018 END OF FILE *****************************/
 
