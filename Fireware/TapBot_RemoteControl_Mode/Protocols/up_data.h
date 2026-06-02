/******************** (C) COPYRIGHT 2017 *****************************************
 * 文件名  ：up_data.h
 * 描述    ：处理底层与上层之间的通信函数头文件
 * 硬件配置：         
 * 版本    ： 
 * 修改日期： 
 * 作者    ： 
 * 修改日志：
*********************************************************************************/
#ifndef __UP_DATA_H
#define __UP_DATA_H

#include "stm32f10x.h"
#include "sys.h"
#include "stdio.h"
#include "usart.h"
#include "function.h"
#include "stdlib.h"
#include "math.h"


/* 航控板 到 底层控制箱 指令 (IF_RS_NGC2LLC) */
struct Rece_Msg								//定义上层发送给底层数据结构体
{
	u8  RStart1;								//数据帧头 0xEB
	u8  RStart2;								//数据帧头 0x90
	u8  RStart3;								//数据帧头 0x72
	u8  RStart4;								//数据帧头 0xC3
	u16 MsgLenth;             	//报文长度
	u32 MsgNumber;	           	//报文帧号
	u16 EngineStartCmd;					//发动机启动指令
															//															//0X0000	无效
															//															//0x1111	左右启动 
															//															//0x2222 	左右关闭
															//															//0x0011 	右启动
															//															//0x0022	右关闭
															//															//0x1100 	左启动
															//															//0x2200	左关闭
	s16 LftGearCmd;     				//左档位指令，2字节，INT16，单位：%，最低有效位：0.1  ，[-50  	+50] 
	s16 LftRudderCmd;     			//左舵角指令，2字节，INT16，单位：°，最低有效位：0.25°，[-30   	+30] 
	s16 RgtGearCmd;     				//右档位指令，2字节，INT16，单位：%，最低有效位：0.1  ，[-50  	+50] 
	s16 RgtRudderCmd;     			//有舵角指令，2字节，INT16，单位：°，最低有效位：0.25°，[-30   	+30] 
	u8	RState;									//工作模式设置指令， 1字节， UINT8，1-遥控器模式 	2-上层模式  		3-海港摇杆模式	
	u8 	RRequest;								//有人/无人请求信号，1字节， UINT8，0-无请求      1-有人驾驶请求	2-无人驾驶请求
	u8  DeviceOnOff_1;					//设备启动/停止1，   1字节， UINT8
	u8  DeviceOnOff_2;					//设备启动/停止2，   1字节， UINT8
	u8  DeviceOnOff_3;					//设备启动/停止3，   1字节， UINT8
	u8 	Reserve1;              	//保留位1，					 1字节， UINT8
	u16 SumCheck;     					//校验和 前面所有数值按16位无符号书值累加和
};
extern struct Rece_Msg Uprece_Msg;  



/* 底层控制箱 到 航控板 指令 (IF_RS_LLC2NGC) */
struct  Send_Msg            		//定义底层发送给上层数据结构体
{
	u8	SStart1;								//数据帧头 0xEB
	u8  SStart2;								//数据帧头 0x90
	u8  SStart3;								//数据帧头 0x72
	u8  SStart4;								//数据帧头 0xC4
	u16 MsgLenth;               //报文长度
	u32 MsgNumber;					    //报文帧号 00000000H~FFFFFFFFH的循环数
	s16	LeftRudderValue;    		//左舵角值，			1字节， INT16，单位：°，最低有效位：0.1°，[-30° - +30°] 
	s16	RightRudderValue;     	//右舵角值，			1字节， INT16，单位：°，最低有效位：0.1°，[-30° - +30°] 
	u16	LeftNaviValue;    			//左斗角值，			1字节，UINT16，单位：°，最低有效位：0.1°，[  0° - 45° ] 
	u16	RightNaviValue;   			//右斗角值，			1字节，UINT16，单位：°，最低有效位：0.1°，[  0° - 45° ] 
	u16 PumpAlarmInform;				//喷泵报警信息，	2字节，UINT16
	s8 	LftEngWaterTemp;				//左发动机水温值, 1字节，  INT8，单位： ℃，最低有效位：   1，[-30 - +100] ℃
	u8  LftEngSpeedValue;				//左发动机转速值, 1字节，	UINT8，单位：RPM，最低有效位：  20，[  0 - 4000]RPM 	
	u16 LftEngThrotValue;				//左主机油门值,   2字节，UINT16，单位：  %，最低有效位： 0.1，[  0 -  100]  %
	u8	LftEngOilPress;					//左发动机油压值, 1字节，	UINT8，单位：kPa，最低有效位：	 4，[  0 - 1000]kPa
	u8  LftEngineState;					//左发动机状态,   1字节， UINT8， 
	u16 LftEngFuelRate;		  		//左发动机油耗,   2字节，UINT16，单位：L/h，最低有效位：0.05，[ 0 - 3212.75]L/h
	s8 	RgtEngWaterTemp;				//右发动机水温值, 1字节，  INT8，单位： ℃，最低有效位：   1，[-30 - +100] ℃
	u8	RgtEngSpeedValue;				//右发动机转速值, 1字节，	UINT8，单位：RPM，最低有效位：  20，[  0 - 4000]RPM 	
	u16 RgtEngThrotValue;				//右主机油门值,   2字节，UINT16，单位：  %，最低有效位： 0.1，[  0 -  100]  %
	u8	RgtEngOilPress;					//右发动机油压值, 1字节，	UINT8，单位：kPa，最低有效位：	 4，[  0 - 1000]kPa
	u8  RgtEngineState;					//右发动机状态,   1字节， UINT8， 
	u16 RgtEngFuelRate;		  		//右发动机油耗,   2字节，UINT16，单位：L/h，最低有效位：0.05，[ 0 - 3212.75]L/h
	u8 	LftOilTankVolume;				//左油箱油量，			1字节， UINT8，单位：  %，最低有效位：   1，[  0 -  100]  %
	u8 	RgtOilTankVolume;				//右油箱油量，			1字节， UINT8，单位：  %，最低有效位：   1，[  0 -  100]  %	
	u16 EngineStartCmd_Sd;			//发动机启动指令   2字节
	s16 LftGearCmd_Sd;     			//左档位指令，2字节，INT16，单位：%，最低有效位：0.1  ，[-50  	+50] 
	s16 LftRudderCmd_Sd;     		//左舵角指令，2字节，INT16，单位：°，最低有效位：0.1° ，[-30   	+30] 
	s16 RgtGearCmd_Sd;     			//右档位指令，2字节，INT16，单位：%，最低有效位：0.1  ，[-50  	+50] 
	s16 RgtRudderCmd_Sd;     		//右舵角指令，2字节，INT16，单位：°，最低有效位：0.1° ，[-30   	+30] 
	u8  DeviceOnOff_1_Sd;				//设备启动/停止1，   1字节， UINT8
	u8  DeviceOnOff_2_Sd;				//设备启动/停止2，   1字节， UINT8
	u8  DeviceOnOff_3_Sd;				//设备启动/停止3，   1字节， UINT8
	u8	RState_sd;							//工作模式设置指令， 1字节， UINT8，1-遥控器模式 	2-上层模式		3-海港摇杆模式	
	u8 	LLCModeState;						//工作模式，         1字节， UINT8，1-有人驾驶  	2-遥控器模式  3-手动摇杆模式  4-航行自主模式
	u8 	RmtOutComState;         //遥控器失联报警，	 1字节，UINT8，底层与遥控器连通  0x01  地层与遥控器失联 0x00
	u8  BattVal_LftEngStart; 		//12V左发动机启动电池， 1字节， UINT8，单位：V，最低有效位：0.1，[0  	14.3]   
	u8  BattVal_RgtEngStart;    //12V右发动机启动电池， 1字节， UINT8，单位：V，最低有效位：0.1，[0  	14.3]  
  u8  BattVal_GenStart;    		//12V发电机启动电池，		1字节， UINT8，单位：V，最低有效位：0.1，[0  	14.3]  
  u8  BattVal_Mast;        		//24V桅杆电池，				 	1字节， UINT8，单位：V，最低有效位：0.1，[5  	  30]		=5V即0V
	u8  BattVal_Daly;         	//24V日用电池，					1字节， UINT8，单位：V，最低有效位：0.1，[5  	  30]		=5V即0V
	u8  BattVal_Reserve;        //24V预留电池，					1字节， UINT8，单位：V，最低有效位：0.1，[5  	  30]		
	u16	AssistCtrlState;        //辅助控制板状态，			2字节，UINT16
	s16	TempData;								//温度，								2字节， INT16
	u16	HumiData;								//湿度，				 				2字节，UINT16
	u16	SumCheck;    						//校验和 前面所有数值按16位无符号书值累加和
};
extern struct Send_Msg UpSend_Msg,bkUpSend_Msg;


extern float UpThroValue;					//上层油门控制值  			[0       100]
extern float UpNaviValue;					//上层倒斗控制值  			[0-上  60-下]
extern float UpLftRudValue;				//上层舵角控制值  			[-30  0  +30]
extern float UpLftGearValue;			//上层档位控制值  			[-50  0  +50]
extern float UpRgtRudValue;				//上层舵角控制值  			[-30  0  +30]
extern float UpRgtGearValue;			//上层档位控制值  			[-50  0  +50]
extern float UpHarbLftRgtValue;		//上层海港左右控制值  	[-50  0  +50]
extern float UpHarbFwdBwdValue;		//上层海港前后控制值  	[-50  0  +50]
extern u8 UpLftEngStartStop;			//上层启动左主机命令 		0-停止  1-启动  0xFF-无效  点触型
extern u8 UpRgtEngStartStop;			//上层启动左主机命令 		0-停止  1-启动  0xFF-无效  点触型
extern u8 UpGenStartStop;					//上层启动发电机命令 		0-停止  1-启动  0xFF-无效  点触型
extern u8 Up_Request;							//有人驾驶/无人驾驶请求 0-无请求      1-有人驾驶请求		2-无人驾驶请求
extern u8 UpReceiveMode;					//接收上层的模式控制值  为保持一致		1-遥控器模式 			2-上层模式  		3-无 		4-海港摇杆模式

extern u8 UpModeChgCmd;						//海港/常规模式切换指令 0x00 无效	0x0F 由当前模式切换到另一模式


extern u8 Updata_BUF[100];

void Proce_UpData(char *ReceDat);
void UpDataExchange(void);
void Send_Updata(void);
void UpDataSendFun(void);



#endif

/******************* (C) COPYRIGHT 2017 END OF FILE *****************************/
 
