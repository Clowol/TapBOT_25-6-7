/******************** (C) COPYRIGHT 2017 *****************************************
 * 文件�? ：rmt_data.h
 * 描述    ：处理底层与遥控器之间的通信
 * 硬件配置�?        
 * 版本    �?
 * 修改日期�?
 * 作�?   �?
 * 修改日志�?
*********************************************************************************/
#ifndef __RMT_DATA_H
#define __RMT_DATA_H

#include "function.h"


extern float RmtRudValue;		          	//遥控器舵角控制�? [-30  0  +30]
extern float RmtThroValue;      				//遥控器油门控制�? [ 0      100]
extern float RmtNaviValue;      				//遥控器倒斗控制�? [ 0       60]
extern float RmtGearValue;							//遥控器档位控制�? [-50	    +50]

extern u8 RmtLftEngStartStop;						//遥控器左主机启动停止命令		0-停止 1-启动  0xFF-无效   点触�?
extern u8 RmtRgtEngStartStop;						//遥控器右主机启动停止命令		0-停止 1-启动  0xFF-无效   点触�?
extern u8 RmtEMotorStartStop;						//遥控器电推进启动停止命令	0-停止 1-启动
extern u8 RmtGenStartStop;							//遥控器发电机启动停止命令	0-停止 1-启动
extern u8 RmtUPSStartStop;							//遥控器UPS启动停止命令 		0-停止 1-启动
extern u8 RmtAirStartStop;							//遥控器空调启动停止命�?		0-停止 1-启动
//extern u8 RmtDeviceUpDown;							//遥控器设备上升下降命�?		0-下降 1-上升
//extern u8 RmtEngUpDown;									//遥控器尾机上升下降命�?		0-下降 1-上升
//extern u8 RmtAFrameUpDown;							//遥控器A柱上升下降命�?		0-下降 1-上升

extern u8 RmtModeChgCmd;								//模式切换指令 			0x00 无效	0x0F 由当前模式切换到另一模式

extern u8 RmtHalt_Stat;

///////////////////////// 柔性机械臂 ////////////////////////////////////////////////////////////////////////////////
extern u8 Rmt_PTZ_UpDown;								//遥控器云台上下命�?			0-下降 1-上升
extern u8 Rmt_PTZ_LftRgt;								//遥控器云台左右命�?			0-下降 1-上升
extern u8 RmtClutchStartStop;						//遥控器离合器命令 				0-断开 1-吸合
extern float Rmt_Y1_Value;							//遥控器Y1控制�? [-50	    +50]


/////////////////////////////////////////////////////////////////////////////////////////////////////////



//云卓T12
/* 遥控�?�?底层控制�?指令 */
//struct Rmt_Rece_Msg				 		
//{
//	u8  Head;			  //帧头0x0F
//	s16	CH_X2;			//X2	右摇杆（左右�?														
//	s16	CH_Y2;			//Y2	右摇杆（上下�?				档位
//	s16	CH_Y1;			//Y1	左摇杆（上下�?
//	s16	CH_X1;			//X1	左摇杆（左右�?				舵角
//	s16	CH_E;				//E		
//	s16	CH_G;				//G		
//	s16	CH_H;				//H	
//	s16	CH_F;				//F
//	s16	CH_A;				//A		按键										发动�?						
//	s16	CH_B;				//B		按键										
//	s16	CH_C;				//C		按键										
//	s16	CH_D;				//D		按键										急停
//	s16	CH_RD;			//RD	旋转拨轮
//	s16	CH_RSSI;		//RSSI
//	s16	CH_A2;			//
//	s16	CH_B2;			//
//	u8  flags;			//标志�?x00
//	u8  End;				//帧尾0x00
//	
//};
struct Rmt_Rece_Msg
{
	 u8  Head;         	//帧头0x66
	 u8  length;      	//长度
	 u8  Reserved1;    	//保留�?
	 u8  DATA_ID;      	//数据ID
	 u16 CH_X2;   	//左摇杆X1,舵角,控制方向�?
	 u16 CH_Y2;          	//左摇杆Y1
	 u16 CH_Y1;          	//右摇杆X2
	 u16 CH_X1; 	//右摇杆Y2，油门，控制电机推力
	s16	CH_E;				//E		
	s16	CH_G;				//G		
	s16	CH_H;				//H	
	s16	CH_F;				//F
	s16	CH_A;				//A		按键										发动�?						
	s16	CH_B;				//B		按键										
	s16	CH_C;				//C		按键										
	s16	CH_D;				//D		按键										急停
	s16	CH_RD;			//RD	旋转拨轮
	s16	CH_RSSI;		//RSSI
	s16	CH_A2;			//
	s16	CH_B2;			//
	 u8  flags;
	 u16 CrcCheck1;        //校验�?
	 

};
extern struct Rmt_Rece_Msg Rmtrece_Msg;  


/* 底层控制�?�?遥控器指�?*/
struct  Rmt_Send_Msg          
{
	u8 SStart1;									//数据帧头 0xA8
	u8 SStart2;									//数据帧头 0x16

	u8 MsgNumber; 							//帧号 0x00~0xFF
	
	u8 LftRudder_LBT; 					//左舵角低8�?
	u8 LftRudder_HBT; 					//左舵角高8�?
	u8 LftSpeed_LBT;						//左转速低8�?  
	u8 LftSpeed_HBT;						//左转速高8�?
	u8 LftNavi_LBT; 						//左倒斗�?�?
	u8 LftNavi_HBT; 						//左倒斗�?�?
	
	u8 RgtRudder_LBT; 					//右舵角低8�?
	u8 RgtRudder_HBT; 					//右舵角高8�?
	u8 RgtSpeed_LBT;						//右转速低8�?  
	u8 RgtSpeed_HBT;						//右转速高8�?
	u8 RgtNavi_LBT; 						//右倒斗�?�?
	u8 RgtNavi_HBT; 						//右倒斗�?�?
	
	u8 DeviceState_1_Mode;			//设备运行状�?/工作模式
	u8 DeviceState_2;						//设备运行状�?
	u8 BattPercent1;						//船体电池电量1
	u8 BattPercent2;						//船体电池电量2
	
	u8 Reserve1;           			//预留1
	u8 Reserve2;           			//预留2
	u8 SumCheck;       					//校验
};
extern struct Rmt_Send_Msg Rmtsend_Msg;

/*****遥控器状态返�?*****/
struct State_Rece_Msg
{
	 u8  Head;         	//帧头0x66
	 u8  length;      	//长度
	 u8  Reserved1;    	//保留�?
	 u8  DATA_ID;      	//数据ID
   u8  RSSI;          //信号强度
	 u16 RemainingBandwidth;//剩余带宽
	 u8  PayloadSize;       //单个数据包最大载�?28字节。对于小于单包最大载荷的数据可一次发送完成，大于单包最大载荷的数据缓存后发送�? 
	 u8  DeviceID;          //设备ID
	 u8  Reserved2; 
	 u8  Reserved3; 
	 u8  Reserved4; 
	 u16 CrcCheck2;        //校验�?
	 

};
extern struct State_Rece_Msg Staterece_Msg;


void Proce_Rmtdata(u8 *ReceDat);
void RmtDataExchange(void);
void RmtDataSendFun(void);
void Send_Rmtdata(void);

#endif


/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/

