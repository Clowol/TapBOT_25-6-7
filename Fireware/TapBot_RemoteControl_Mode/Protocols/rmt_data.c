/******************** (C) COPYRIGHT 2024 *****************************************
 * 文件名  ：rmt_data.c
 * 描述    ：处理遥控器接收和发送的数据
 * 硬件配置：         
 * 版本    ：
 * 修改日期：
 * 作者    : 
 * 修改日志:
*********************************************************************************/ 
#include "rmt_data.h"
#include "control_math.h"


//限幅线性化
//舵角
float Num_Rud_Arr[2][3] =						//遥控器接收数值 和 遥控舵角指令 对应表
{  
	//  左     中       右
	{  283,  1002,   	1722},					//遥控器接收数值?
	{  -30,     0,      30}						//遥控舵角指令
};

//档位
float Num_Gear_Arr[2][3] =					//遥控器接收数值 和 遥控油门指令 对应表
{  
	//  倒     中       前
	{ 1950,  1500,   	 1050},					//遥控器接收数值?
	{  -50,     0,       50}					//遥控档位指令  遥控油门限幅 70% 50*0.7=35
};

//Y1
float Num_Y1_Arr[2][3] =						//遥控器接收数值 和 遥控油门指令 对应表
{  
	//  倒     中       前
	{ 1950,  1500,   	 1050},					//遥控器接收数值?
	{  -50,     0,       50}					//遥控档位指令  遥控油门限幅 70% 50*0.7=35
};


float RmtRudValue = 0;				//遥控器舵角控制值  [-30  0   +30]
float RmtThroValue = 0;				//遥控器油门控制值  [0        100]
float RmtNaviValue = 0; 			//遥控器倒斗控制值  [0-上	  60-下]
float RmtGearValue = 0;				//遥控器档位控制值  [-50	    +50]

u8 RmtLftEngStartStop=0xFF;		//遥控器左主机启动停止命令		0-停止 1-启动  0xFF-无效   点触型
u8 RmtRgtEngStartStop=0xFF;		//遥控器右主机启动停止命令		0-停止 1-启动  0xFF-无效   点触型
u8 RmtEMotorStartStop=0xFF;		//遥控器电推进启动停止命令	0-停止 1-启动
u8 RmtGenStartStop=0xFF;			//遥控器发电机启动停止命令	0-停止 1-启动
u8 RmtUPSStartStop=0xFF;			//遥控器UPS启动停止命令 		0-停止 1-启动
u8 RmtAirStartStop=0xFF;			//遥控器空调启动停止命令 		0-停止 1-启动
//u8 RmtDeviceUpDown=0xFF;			//遥控器设备上升下降命令 		0-下降 1-上升
//u8 RmtEngUpDown=0xFF;					//遥控器尾机上升下降命令 		0-下降 1-上升
//u8 RmtAFrameUpDown=0xFF;			//遥控器A柱上升下降命令 		0-下降 1-上升

u8 RmtModeChg=0;							//遥控器模式切换 		0x00 无效	0x0F 由当前模式切换到另一模式
u8 RmtModeChg_Old=0;					//模式切换上次值 		0x00 无效	0x0F 由当前模式切换到另一模式
u8 RmtModeChgCmd=0;						//模式切换指令 			0x00 无效	0x0F 由当前模式切换到另一模式

///////////////////////// 柔性机械臂 ////////////////////////////////////////////////////////////////////////////////
u8 Rmt_PTZ_UpDown=0xFF;				//遥控器云台上下命令 			0-下降 1-上升
u8 Rmt_PTZ_LftRgt=0xFF;				//遥控器云台左右命令 			0-下降 1-上升
u8 RmtClutchStartStop=0;			//遥控器离合器命令 				0-断开 1-吸合
float Rmt_Y1_Value = 0;				//遥控器Y1控制值  [-50	    +50]


/////////////////////////////////////////////////////////////////////////////////////////////////////////

u8 RmtHalt_Stat=0;            //急停标志位

u8 Rmtdata_BUF[100]={0};	        	//定义底层通过串口发送给上层的缓存数组

//struct Rmt_Rece_Msg Rmtrece_Msg=    //定义接收的数据结构体
//{
//	0x00,							//u8  Head;			  //帧头0x0F
//		0,							//s16	CH_X2;			//X2	右摇杆（左右）															
//		0,							//s16	CH_Y2;			//Y2	右摇杆（上下）					档位
//		0,							//s16	CH_Y1;			//Y1	左摇杆（上下）
//		0,							//s16	CH_X1;			//X1	左摇杆（左右）					舵角
//		0,							//s16	CH_E;				//E		
//		0,							//s16	CH_G;				//G		
//		0,							//s16	CH_H;				//H	
//		0,							//s16	CH_F;				//F
//		0,							//s16	CH_A;				//A		按键										发动机							
//		0,							//s16	CH_B;				//B		按键										
//		0,							//s16	CH_C;				//C		按键										
//		0,							//s16	CH_D;				//D		按键										急停
//		0,							//s16	CH_RD;			//RD	旋转拨轮
//		0,							//s16	CH_RSSI;		//RSSI
//		0,							//s16	CH_A2;			//
//		0,							//s16	CH_B2;			//
//	0x00,							//u8  flags;			//标志位0x00
//	0x00,							//u8  End;				//帧尾0x00
//	
//};

struct Rmt_Rece_Msg Rmtrece_Msg=                         //初始化遥控器的接收数据
{
	0x66,                                //u8  Head;         //帧头0x66
	0x19,                                //u8  length;       //长度
	0,                                   //u8  Reserved;     //保留位
	0xB0,                                //u8  DATA_ID;      //数据ID
	1500,                                //CH1_Rudder
	1500,                                //CH2
	1500,                                //CH3
	1500,                                //CH4_Throttle
	1500,                                //CH5
	1500,                                //CH6_ModeChange       
	1500,                                //CH7
	1500,                                //CH8
	1000,                                //CH9
	1000,                                //CH10
	1000,                                //CH11
	1000,                                //CH12
	0,									 //CH13
	0,									 //CH14
	0,									 //CH15
	0,									 //CH16
	0,                                   //u16  CrcCheck        //校验位
  
};


struct Rmt_Send_Msg Rmtsend_Msg=			  //定义发送的数据结构体
{	
	0xA8,												//0		u8 SStart1;									//数据帧头 0xA8
	0x16,												//1		u8 SStart2;									//数据帧头 0x16

	0x00,												//2		u8 MsgNumber; 							//帧号 0x00~0xFF
															//		
	0x64,												//3		u8 LftRudder_LBT; 					//左舵角低8位
	0x00,												//4		u8 LftRudder_HBT; 					//左舵角高8位
	0xD2,												//5		u8 LftSpeed_LBT;						//左转速低8位   
	0x07,												//6		u8 LftSpeed_HBT;						//左转速高8位 
	0x00,												//7		u8 LftNavi_LBT; 						//左倒斗低8位
	0x00,												//8		u8 LftNavi_HBT; 						//左倒斗高8位
															//		
	0x00,												//9		u8 RgtRudder_LBT; 					//右舵角低8位
	0x00,												//10	u8 RgtRudder_HBT; 					//右舵角高8位
	0x25,												//11	u8 RgtSpeed_LBT;						//右转速低8位   
	0x03,												//12	u8 RgtSpeed_HBT;						//右转速高8位 
	0x00,												//13	u8 RgtNavi_LBT; 						//右倒斗低8位
	0x00,												//14	u8 RgtNavi_HBT; 						//右倒斗高8位
															//		
	0x83,												//15	u8 DeviceState_1_Mode;			//设备运行状态1/工作模式
	0x00,												//16	u8 DeviceState_2;						//设备运行状态2
	0x00,												//17	u8 BattPercent1;						//船体电池电量1
	0x00,												//18	u8 BattPercent2;						//船体电池电量2
															//		
	0x00,												//19	u8 Reserve1;           			//预留1
	0x22,												//20	u8 Reserve2;           			//预留2
	0x00,												//21	u8 SumCheck;       					//校验
};
	

struct State_Rece_Msg Staterece_Msg=
{
 0x66,
 0x08,
 0,
 0xB1,
 100,
 65535,
 28,
 0x01,
 0,
 0,
 0,
 0,	
};


/***************16位校验和*********************/
u16 cbus_crc1(u8 pbuf[ ])  //校验算法与MavLink校验相似，长度不同，无额外字节
{

	u8 length = pbuf[1];
	u16 sum = 0xFFFF;
	u16  i, stoplen;
 	u8 tmp;
	stoplen = length + 3;
	i = 1;
	while (i<stoplen) 
		{
			
			tmp = pbuf[i] ^ (u8)(sum&0xff);
			tmp ^= (tmp<<4);
			sum = (sum>>8) ^ (tmp<<8) ^ (tmp<<3) ^ (tmp>>4);
			i++;
		} 

	return sum;
		
}

u16 cbus_crc2(u8 pbuf[ ])  //校验算法与MavLink校验相似，长度不同，无额外字节
{

	u8 length = pbuf[32];
	u16 sum = 0xFFFF;
	u16  i, stoplen;
 	u8 tmp;
	stoplen = length +3+31;	
	i = 32;
	while (i<stoplen) 
		{			
			tmp = pbuf[i] ^ (u8)(sum&0xff);
			tmp ^= (tmp<<4);
			sum = (sum>>8) ^ (tmp<<8) ^ (tmp<<3) ^ (tmp>>4);
			i++;
		} 

	return sum;
		
}



/*
 * 函数名：Proce_Rmtdata
 * 描述  ：解析处理智能信息层发送到底层的控制命令
 * 输入  ：ReceDat，接收到的数据数组
 * 输出  ：无	
 */
void Proce_Rmtdata(u8 *ReceDat)
{
	u16 Pro_crc1=0;
	u16 Pro_crc2=0;
	Pro_crc1=cbus_crc1(ReceDat);                      //调用校验和  从Length开始
	Pro_crc2=cbus_crc2(ReceDat);
//  printf("Pro_crc2=%x\r\n",Pro_crc2);
	
	Rmtrece_Msg.Head=ReceDat[0];
	Rmtrece_Msg.CrcCheck1=((u16)ReceDat[29]<<8)|ReceDat[30];
	
	Staterece_Msg.Head=ReceDat[31];
  Staterece_Msg.CrcCheck2=((u16)ReceDat[43]<<8)|ReceDat[44];
	
//	printf("Rmtrece_Msg.CrcCheck1=%x\r\n",Rmtrece_Msg.CrcCheck1);
//	printf("Staterece_Msg.CrcCheck2=%x\r\n",Staterece_Msg.CrcCheck2);
	if(Rmtrece_Msg.Head==0x66&&Pro_crc1==Rmtrece_Msg.CrcCheck1)          //帧头以及校验位同时满足
	{

		Rmtrece_Msg.length=ReceDat[1];
	  Rmtrece_Msg.Reserved1=ReceDat[2];
	  Rmtrece_Msg.DATA_ID=ReceDat[3];
	  Rmtrece_Msg.CH_X2=((u16)ReceDat[4]<<4)|((ReceDat[5]&0xF0)>>4);
	  Rmtrece_Msg.CH_Y2=((u16)(ReceDat[5]&0x0F)<<8)|ReceDat[6];
	  Rmtrece_Msg.CH_Y1=((u16)ReceDat[7]<<4)|((ReceDat[8]&0xF0)>>4);
	  Rmtrece_Msg.CH_X1=((u16)(ReceDat[8]&0x0F)<<8)|ReceDat[9];
	  Rmtrece_Msg.CH_E=((u16)ReceDat[10]<<4)|((ReceDat[11]&0xF0)>>4);
	  Rmtrece_Msg.CH_G=((u16)(ReceDat[11]&0x0F)<<8)|ReceDat[12];
	  Rmtrece_Msg.CH_H=((u16)ReceDat[13]<<4)|((ReceDat[14]&0xF0)>>4);
	  Rmtrece_Msg.CH_F=((u16)(ReceDat[14]&0x0F)<<8)|ReceDat[15];
	  Rmtrece_Msg.CH_A=((u16)ReceDat[16]<<4)|((ReceDat[17]&0xF0)>>4);
	  Rmtrece_Msg.CH_B=((u16)(ReceDat[17]&0x0F)<<8)|ReceDat[18];
	  Rmtrece_Msg.CH_C=((u16)ReceDat[19]<<4)|((ReceDat[20]&0xF0)>>4);
	  Rmtrece_Msg.CH_D=((u16)(ReceDat[20]&0x0F)<<8)|ReceDat[21];
	  Rmtrece_Msg.CH_RD=((u16)ReceDat[22]<<4)|((ReceDat[23]&0xF0)>>4);
	  Rmtrece_Msg.CH_RSSI=((u16)(ReceDat[23]&0x0F)<<8)|ReceDat[24];
	  Rmtrece_Msg.CH_A2=((u16)ReceDat[25]<<4)|((ReceDat[26]&0xF0)>>4);
	  Rmtrece_Msg.CH_B2=((u16)(ReceDat[26]&0x0F)<<8)|ReceDat[27];
	  Rmtrece_Msg.flags=ReceDat[28]; 
	  
    CntRxRmt = 1000;
		RmtOutCommunFlg = 0;
		RmtPwrOffFlg = 0;
  	RmtDataExchange();
 }
	
if(Staterece_Msg.Head==0x66&&Pro_crc2==Staterece_Msg.CrcCheck2)
  {

	 Staterece_Msg.RSSI=ReceDat[35];
	}
}
	
//	//要把这个放在外边，20230213
//	CntRxRmt = 60;
//	RmtPwrOffFlg = 0;

//	//失联
//	if(Rmtrece_Msg.flags == 0x08)
//		RmtOutCommunFlg = 1;
//	else if(Rmtrece_Msg.flags == 0x00)
//		RmtOutCommunFlg = 0;
//}



//云卓T12
//               
//Y2	档位					1050下-1500中-1950上
//X1	舵角					1050左-1500中-1950右
//A		发动机				1050弹起-1950按下
//B		右发动机			1050弹起-1950按下
//C		模式切换			1050弹起-1950按下
//D		挂机升降			1050弹起-1950按下
//SA	发电机				1050停止-1500无效-1950启动
//SB	UPS						1050停止-1500无效-1950启动
//SC								1050停止-1500无效-1950启动
//LD								1050下降-1500无效-1950上升
//RD								1050下降-1500无效-1950上升
//云卓T12
// 
//
//柔性机械臂
//
//Y1	舵机					1050左-1500中-1950右
//Y2	宇树电机			1050下-1500中-1950上
//A		离合器				1050弹起-1950按下
//B		舵机选择2X		1050弹起-1950按下
//C		模式切换			1050弹起-1950按下
//D									1050弹起-1950按下
//E									1050停止-1500无效-1950启动
//F									1050停止-1500无效-1950启动
//G									1050停止-1500无效-1950启动
//H									1050下降-1500无效-1950上升

/*
 * 函数名：RmtDataExchange
 * 描述  ：将接收到的数据转换为要使用的数值
 * 输入  ：无
 * 输出  ：无	
 */
void RmtDataExchange(void)
{	
	static u8 FirstFlag = 0;
//	static u8 LftEngKey = 0, LftEngKey_Old = 0;		  //遥控尾机启动
//	static u8 RgtEngKey = 0, RgtEngKey_Old = 0;			//遥控尾机启动
//	static u8 EMotorKey = 0, EMotorKey_Old = 0;		  //电推启动
//	static u8 GenKey = 0, GenKey_Old = 0;		  			//发电机启动
//	static u8 AirKey = 0, AirKey_Old = 0;		  			//发动机启动
//	static u8 UPSKey = 0, UPSKey_Old = 0;		  			//UPS启动
//	static u8 RudDebugKey = 0, RudDebugKey_Old = 0;	//舵角调试模式
//	static u8 CntRudDebugKey = 0;

//	static u8 A_RmtKey = 0, A_RmtKey_Old = 0;		  		//舵机1
//	static u8 B_RmtKey = 0, B_RmtKey_Old = 0;		  		//舵机2
//	static u8 C_RmtKey = 0, C_RmtKey_Old = 0;		  		//舵机3
//	static u8 D_RmtKey = 0, D_RmtKey_Old = 0;		  		//舵机4
	static u8 E_RmtKey = 0xFF, E_RmtKey_Old = 0xFF;		//云台上下
	static u8 G_RmtKey = 0xFF, G_RmtKey_Old = 0xFF;		//云台左右
	static u8 F_RmtKey = 0xFF, F_RmtKey_Old = 0xFF;		//离合器
	
	u16 RudTempNum = 0, GearTempNum = 0, Y1_TempNum = 0;
	
	//E
	if(Rmtrece_Msg.CH_E > RMT_SWITCH_HIGH_THRESHOLD)
		E_RmtKey = 1;
	else if (Rmtrece_Msg.CH_E < RMT_SWITCH_LOW_THRESHOLD)
		E_RmtKey = 0;
	else
		E_RmtKey = 0xFF;

	//F
	if(Rmtrece_Msg.CH_F > RMT_SWITCH_HIGH_THRESHOLD)
		F_RmtKey = 1;
	else if (Rmtrece_Msg.CH_F < RMT_SWITCH_LOW_THRESHOLD)
		F_RmtKey = 0;
	else
		F_RmtKey = 0xFF;

	//G
	if(Rmtrece_Msg.CH_G > RMT_SWITCH_HIGH_THRESHOLD)
		G_RmtKey = 1;
	else if (Rmtrece_Msg.CH_G < RMT_SWITCH_LOW_THRESHOLD)
		G_RmtKey = 0;
	else
		G_RmtKey = 0xFF;

	//第一次老值赋值  重启后第一次接收的发电机控制值也舍去
	//上电后 CH_SA=1023再变成1712 上电后CH_SB?
	if(FirstFlag < 2)
	{
//		A_RmtKey_Old = A_RmtKey;
//		B_RmtKey_Old = B_RmtKey;
//		C_RmtKey_Old = C_RmtKey;
//		D_RmtKey_Old = D_RmtKey;
		E_RmtKey_Old = E_RmtKey;
		G_RmtKey_Old = G_RmtKey;
		F_RmtKey_Old = F_RmtKey;

		FirstFlag ++;
	}	
	
	//模式切换
	if(Rmtrece_Msg.CH_H >= RMT_SWITCH_HIGH_THRESHOLD)
	{
		g_RmtUpManCtrlMode = RMT_MODE;
//		g_RudDebugMode = 0;	//退出调试模式
	}
	else if(Rmtrece_Msg.CH_H <= RMT_SWITCH_LOW_THRESHOLD)
	{
		g_RmtUpManCtrlMode = UP_MODE;
//		g_RudDebugMode = 0;	//退出调试模式
	}
	else
	{
		;
//		g_RudDebugMode = 1;	//进入调试模式
	}

//	//A柱上升下降
//	if(Rmtrece_Msg.CH_Y1 >= 1700)
//	{
//		if(Rmtrece_Msg.CH_F < 1300)
//			RmtAFrameUpDown = 1;									//上升
//		else if (Rmtrece_Msg.CH_F > 1700)
//			RmtAFrameUpDown = 0;                	//下降     
//		else
//			RmtAFrameUpDown = 0xFF;								//无效
//	}

//	//抽油泵 遥控控制
//	if(Rmtrece_Msg.CH_Y1 <= 1300 && g_RmtUpManRealMode == RMT_MODE)
//	{
//		if(Rmtrece_Msg.CH_F < 1300)
//		{
//			OIL_PUMP(ON);													//抽油泵工作
//		}
//		else
//		{
//			OIL_PUMP(OFF);												//抽油泵停止
//		}
//	}
	
	//云台上升下降 E
	if((E_RmtKey_Old == 0xFF && E_RmtKey == 0) || (E_RmtKey_Old == 1 && E_RmtKey == 0))
	{
		Rmt_PTZ_UpDown = 0;									//上升
		PTZ_UpDownMoveFlg = PTZ_UP;
		PTZ_SendMsg_Cmd.Up_Down_Speed = PTZ_UP_DOWN_SPEED_DEFAULT;
		Send_PTZ_Data();
		swgPrtUx(USART2, "PTZ Up\r\n"); 						 
	}
	else if((E_RmtKey_Old == 0xFF && E_RmtKey == 1) || (E_RmtKey_Old == 0 && E_RmtKey == 1))
	{
		Rmt_PTZ_UpDown = 1;                	//下降  
		PTZ_UpDownMoveFlg = PTZ_DOWN;
		PTZ_SendMsg_Cmd.Up_Down_Speed = PTZ_UP_DOWN_SPEED_DEFAULT;
		swgPrtUx(USART2, "PTZ Down\r\n"); 						 
		Send_PTZ_Data();
	}		
	else if((E_RmtKey_Old == 0 && E_RmtKey == 0xFF) || (E_RmtKey_Old == 1 && E_RmtKey == 0xFF))
	{
		Rmt_PTZ_UpDown = 0xFF;							//停止
		PTZ_UpDownMoveFlg = PTZ_STOP;
		swgPrtUx(USART2, "PTZ Up Down Stop\r\n"); 						 
		Send_PTZ_Data();
	}
//	else
//	{
//		Rmt_PTZ_UpDown = 0xFF;							//停止
//		PTZ_UpDownMoveFlg = PTZ_STOP;
//	}
	E_RmtKey_Old = E_RmtKey;
	
	
	//云台左转右转  G 左0  中0xFF  右1
	if((G_RmtKey_Old == 0xFF && G_RmtKey == 0) || (G_RmtKey_Old == 1 && G_RmtKey == 0))
	{
		Rmt_PTZ_LftRgt = 0;									//左转
		PTZ_LftRgtMoveFlg = PTZ_LEFT;
		PTZ_SendMsg_Cmd.Lft_Rgt_Speed = PTZ_LEFT_RIGHT_SPEED_DEFAULT;
		Send_PTZ_Data();
		swgPrtUx(USART2, "PTZ Left\r\n"); 						 
	}
	else if((G_RmtKey_Old == 0xFF && G_RmtKey == 1) || (G_RmtKey_Old == 0 && G_RmtKey == 1))
	{
		Rmt_PTZ_LftRgt = 1;                	//右转  
		PTZ_LftRgtMoveFlg = PTZ_RIGHT;
		PTZ_SendMsg_Cmd.Lft_Rgt_Speed = PTZ_LEFT_RIGHT_SPEED_DEFAULT;
		Send_PTZ_Data();
		swgPrtUx(USART2, "PTZ Right\r\n"); 						 
	}		
	else if((G_RmtKey_Old == 0 && G_RmtKey == 0xFF) || (G_RmtKey_Old == 1 && G_RmtKey == 0xFF))
	{
		Rmt_PTZ_LftRgt = 0xFF;              //停止  
		PTZ_LftRgtMoveFlg = PTZ_STOP;
		Send_PTZ_Data();
		swgPrtUx(USART2, "PTZ Lft Rgt STOP\r\n"); 						 
	}
//	else
//	{
//		Rmt_PTZ_LftRgt = 0xFF;							//停止
//		PTZ_LftRgtMoveFlg = PTZ_STOP;
//	}
	G_RmtKey_Old = G_RmtKey;
	
	
	//离合器 F
	if((F_RmtKey_Old == 0xFF && F_RmtKey == 0) || (F_RmtKey_Old == 1 && F_RmtKey == 0))
	{
		RmtClutchStartStop = 1;
		swgPrtUx(USART2, "RmtClutchStart\r\n"); 						 
	}
	else if((F_RmtKey_Old == 0 && F_RmtKey == 0xFF) || (F_RmtKey_Old == 0 && F_RmtKey == 1))
	{
		RmtClutchStartStop = 0;
		swgPrtUx(USART2, "RmtClutchStop\r\n"); 						 
	}		
	F_RmtKey_Old = F_RmtKey;


//	//舵机0
//	if(A_RmtKey_Old == 0 && A_RmtKey == 1)						//检测到上升沿
//	{
//		SteerSendMsgArr[0].PosData = 2048;
//		swgPrtUx(USART2, "Rmt Steer 00 Start\r\n"); 						 
//	}	
//	else if(A_RmtKey_Old == 1 && A_RmtKey == 0)				//检测到下降沿
//	{
//		SteerSendMsgArr[0].PosData = 0;
//		swgPrtUx(USART2, "Rmt Steer 00 Stop\r\n"); 						 
//	}
//	A_RmtKey_Old = A_RmtKey;

//	//舵机1
//	if(B_RmtKey_Old == 0 && B_RmtKey == 1)									//检测到上升沿
//	{
//		SteerSendMsgArr[1].PosData = 1024;
//		swgPrtUx(USART2, "Rmt Steer 01 Start\r\n"); 						 
//	}	
//	else if(B_RmtKey_Old == 1 && B_RmtKey == 0)							//检测到下降沿
//	{
//		SteerSendMsgArr[1].PosData = 0;
//		swgPrtUx(USART2, "Rmt Steer 01 Stop\r\n"); 						 
//	}
//	B_RmtKey_Old = B_RmtKey;
//	
//	//舵机2
//	if(C_RmtKey_Old == 0 && C_RmtKey == 1)									//检测到上升沿
//	{
//		SteerSendMsgArr[2].PosData = 2048;
//		swgPrtUx(USART2, "Rmt Steer 02 Start\r\n"); 						 
//	}	
//	else if(C_RmtKey_Old == 1 && C_RmtKey == 0)							//检测到下降沿
//	{
//		SteerSendMsgArr[2].PosData = 0;
//		swgPrtUx(USART2, "Rmt Steer 02 Stop\r\n"); 						 
//	}
//	C_RmtKey_Old = C_RmtKey;

//	//舵机3
//	if(D_RmtKey_Old == 0 && D_RmtKey == 1)									//检测到上升沿
//	{
//		SteerSendMsgArr[3].PosData = 2420;
//		swgPrtUx(USART2, "Rmt Steer 03 Start\r\n"); 						 
//	}
//	else if(D_RmtKey_Old == 1 && D_RmtKey == 0)							//检测到下降沿
//	{
//		SteerSendMsgArr[3].PosData = 3130;
//		swgPrtUx(USART2, "Rmt Steer 03 Stop\r\n"); 						 
//	}
//	D_RmtKey_Old = D_RmtKey;
	
	
	//油门、倒斗、舵角
	RudTempNum  = Rmtrece_Msg.CH_X1;										//遥控器接收值
	GearTempNum = Rmtrece_Msg.CH_Y2;										//遥控器接收值
	Y1_TempNum = Rmtrece_Msg.CH_Y1;										//遥控器接收值

	//////////////////////////////////////////////////////////////////////////////////////////
	//舵角线性化
	RmtRudValue = control_map_3point((float)RudTempNum, Num_Rud_Arr);
		
	//////////////////////////////////////////////////////////////////////////////////////////
	//档位线性化
	if(GearTempNum < Num_Gear_Arr[0][2])  GearTempNum = Num_Gear_Arr[0][2];
	if(GearTempNum > Num_Gear_Arr[0][0]) 	GearTempNum = Num_Gear_Arr[0][0];
		
	if(GearTempNum > Num_Gear_Arr[0][1])
		RmtGearValue = ((float)GearTempNum-Num_Gear_Arr[0][1])/(Num_Gear_Arr[0][2]-Num_Gear_Arr[0][1])*(Num_Gear_Arr[1][2]-Num_Gear_Arr[1][1])+Num_Gear_Arr[1][1];
	else if(GearTempNum == Num_Gear_Arr[0][1])
		RmtGearValue = (float)Num_Gear_Arr[1][1];
	else if(GearTempNum < Num_Gear_Arr[0][1])
		RmtGearValue = Num_Gear_Arr[1][1]-(Num_Gear_Arr[0][1]-(float)GearTempNum)/(Num_Gear_Arr[0][1] - Num_Gear_Arr[0][0]) * (Num_Gear_Arr[1][1] - Num_Gear_Arr[1][0]);				
	
	if(RmtGearValue < Num_Gear_Arr[1][0])	
		RmtGearValue = Num_Gear_Arr[1][0];
	else if(RmtGearValue > Num_Gear_Arr[1][2])	
		RmtGearValue = Num_Gear_Arr[1][2];
	
	//中位附近均为0
	if(-1 <= RmtGearValue && RmtGearValue <= 1) 	RmtGearValue = 0;

	//////////////////////////////////////////////////////////////////////////////////////////
	//Y1线性化
	if(Y1_TempNum < Num_Y1_Arr[0][2])  Y1_TempNum = Num_Y1_Arr[0][2];
	if(Y1_TempNum > Num_Y1_Arr[0][0])  Y1_TempNum = Num_Y1_Arr[0][0];
		
	if(Y1_TempNum > Num_Y1_Arr[0][1])
		Rmt_Y1_Value = ((float)Y1_TempNum-Num_Y1_Arr[0][1])/(Num_Y1_Arr[0][2]-Num_Y1_Arr[0][1])*(Num_Y1_Arr[1][2]-Num_Y1_Arr[1][1])+Num_Y1_Arr[1][1];
	else if(Y1_TempNum == Num_Y1_Arr[0][1])
		Rmt_Y1_Value = (float)Num_Y1_Arr[1][1];
	else if(Y1_TempNum < Num_Y1_Arr[0][1])
		Rmt_Y1_Value = Num_Y1_Arr[1][1]-(Num_Y1_Arr[0][1]-(float)Y1_TempNum)/(Num_Y1_Arr[0][1] - Num_Y1_Arr[0][0]) * (Num_Y1_Arr[1][1] - Num_Y1_Arr[1][0]);				
	
	if(Rmt_Y1_Value < Num_Y1_Arr[1][0])	
		Rmt_Y1_Value = Num_Y1_Arr[1][0];
	else if(Rmt_Y1_Value > Num_Y1_Arr[1][2])	
		Rmt_Y1_Value = Num_Y1_Arr[1][2];
	
	//中位附近均为0
	if(-1 <= Rmt_Y1_Value && Rmt_Y1_Value <= 1) 	Rmt_Y1_Value = 0;

	//打印
//		swgPrtUx(USART2, "RmtRud=%.1f\r\n", RmtRudValue); 											
//		swgPrtUx(USART2, "RmtThro=%.1f\r\n", RmtThroValue); 	
		
	if(RmtHalt_Stat)
	{
//		RmtRudValue = 0;
		RmtGearValue = 0;

//		swgPrtUx(USART2, "HS\r\n"); 												//遥控器测试
	}

//	//舵角调试模式
//	if(Rmtrece_Msg.CH_D >= 1700)  
//		g_RudDebugMode = 1;	//进入调试模式
//	else if(Rmtrece_Msg.CH_D <= 300)  
//		g_RudDebugMode = 0;	//退出调试模式
	
}






/******************* (C) COPYRIGHT 2024 END OF FILE *****************************/
