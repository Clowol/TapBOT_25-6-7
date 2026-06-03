/******************** (C) COPYRIGHT 2024 *****************************************
 * 鏂囦欢锟? 锛歴teer_data.c
 * 鎻忚堪    锛氳埖鏈烘暟鎹彂锟?
 * 纭欢閰嶇疆锟?        
 * 鐗堟湰    锟?
 * 淇敼鏃ユ湡锟?
 * 浣滐拷?   : 
 * 淇敼鏃ュ織:
*********************************************************************************/ 
#include "steer_data.h"
#include "control_math.h"
#include "control_dispatcher.h"
#include "app_config.h"
#include "subboard_protocol.h"


#define END_AUTO_FORWARD_TIME_MS    3000U
#define END_AUTO_REVERSE_TIME_MS    3000U
#define END_AUTO_ROTATE_SPEED       600

//閫熷害 Y1瀵瑰簲
float Y1_Speed_Arr[2][3] =				// 
{  
	//  锟?     锟?      锟?
	{ STEER_Y1_IN_LEFT,     STEER_Y1_IN_MID,     STEER_Y1_IN_RIGHT},					// 
	{ STEER_SPEED_OUT_LEFT, STEER_SPEED_OUT_MID, STEER_SPEED_OUT_RIGHT}					// 
};


s16 SteerRunSpeed = 0;							//

u8 SteerRunMode = STEER_SPEED_MODE;
u8 SteerActualMode = STEER_MODE_UNKNOWN;


//寮傛鍐欏崟涓埖锟?
u8 SteerDataBuf[13] = \
{
//  0     1     2     3     4     5     6     7     8     9     10    11    12     
//															鍐欏叆锟?
//    瀛楀ご      ID   闀垮害  鎸囦护 棣栧湴鍧€  浣嶇疆鏁版嵁    鏃堕棿鏁版嵁    閫熷害鏁版嵁 	 鏍￠獙							
	 0xFF, 0xFF, 0x01, 0x09, 0x03, 0x2A, 0x00, 0x08, 0x00, 0x00, 0xE8, 0x03, 0x00
};


//鍚屾鍐欏涓埖锟?浣嶇疆 鏃堕棿 閫熷害
u8 SteerDataBuf_SYNC[36] = \
{
//  0     1     2     3     4     5     6     
//															鍐欏叆锟?鏁版嵁
//    瀛楀ご      ID   闀垮害  鎸囦护 棣栧湴鍧€ 闀垮害
   0xFF, 0xFF, 0xFE, 0x20, 0x83, 0x2A, 0x06, \

//	7     8     9     10    11    12    13
// 鑸垫満
// ID 0   浣嶇疆鏁版嵁    鏃堕棿鏁版嵁    閫熷害鏁版嵁 	 
   0x00, 0x00, 0x08, 0x00, 0x00, 0xE8, 0x03, \
		
//	14    15    16    17    18    19    20
// 鑸垫満
// ID 1   浣嶇疆鏁版嵁    鏃堕棿鏁版嵁    閫熷害鏁版嵁 	 
   0x01, 0x00, 0x08, 0x00, 0x00, 0xE8, 0x03, \

//	21    22    23    24    25    26    27
// 鑸垫満
// ID 2   浣嶇疆鏁版嵁    鏃堕棿鏁版嵁    閫熷害鏁版嵁 	 
   0x02, 0x00, 0x08, 0x00, 0x00, 0xE8, 0x03, \

//	28    29    30    31    32    33    34
// 鑸垫満
// ID 3   浣嶇疆鏁版嵁    鏃堕棿鏁版嵁    閫熷害鏁版嵁 	 
   0x03, 0x00, 0x08, 0x00, 0x00, 0xE8, 0x03, \

//	35
// 鏍￠獙							
	 0x00
};


//鍚屾鍐欏涓埖锟?璁剧疆杩愯妯″紡
u8 SteerDataBuf_RunMode[16] = \
{
//  0     1     2     3     4     5     6     
//															鍐欏叆锟?鏁版嵁
//    瀛楀ご      ID   闀垮害  鎸囦护 棣栧湴鍧€ 闀垮害
   0xFF, 0xFF, 0xFE, 0x0C, 0x83, 0x21, 0x01, \

//	7     8     9     10    11    12    13    14
// 鑸垫満  杩愯  鑸垫満  杩愯  鑸垫満  杩愯  鑸垫満  杩愯
// ID 0  妯″紡  ID 1  妯″紡  ID 2  妯″紡  ID 3  妯″紡
   0x00, 0x01, 0x01, 0x01, 0x02, 0x01, 0x03, 0x01, \

//	15
// 鏍￠獙							
	 0x00
};


//鍚屾鍐欏涓埖锟?閫熷害妯″紡涓嬭缃繍琛岄€熷害
u8 SteerDataBuf_SpeedRun[20] = \
{
//  0     1     2     3     4     5     6     
//															鍐欏叆锟?鏁版嵁
//    瀛楀ご      ID   闀垮害  鎸囦护 棣栧湴鍧€ 闀垮害
   0xFF, 0xFF, 0xFE, 0x10, 0x83, 0x2E, 0x02, \

//	7     8     9     10    11    12    13    14    15    16    17    18
// 鑸垫満              鑸垫満              鑸垫満              鑸垫満
// ID 0   杩愯閫熷害   ID 1   杩愯閫熷害   ID 2   杩愯閫熷害   ID 3   杩愯閫熷害
   0x00, 0xE8, 0x03, 0x01, 0xE8, 0x03, 0x02, 0xE8, 0x03, 0x03, 0xE8, 0x03, \

//	19
// 鏍￠獙
	 0x00
};


struct SteerSendMsg SteerSendMsg_1 = 
{
				1,		//	u8 	SteerID;			//ID锟?
				0,		//	u8	AccData;			//鍔犻€熷害
		 2048,		//	s16 PosData;			//浣嶇疆
				0,		//	u16	RunTime;			//杩愯鏃堕棿
		 1000,		//	s16	SpeedData;		//閫熷害
};


//4涓數鏈虹殑鍙傛暟  2048 = 180掳
struct SteerSendMsg SteerSendMsgArr[4] = 
{
//  SteerID,  AccData,  PosData,  RunTime,  SpeedData
	{    0,        0,       0,      0,        300},	
	{    1,        0,       0,      0,        300},	
	{    2,        0,       0,      0,        300},	
	{    3,        0,    2420,      0,        300}
};
/*
 * 鍑芥暟鍚嶏細SteerCheckSum
 * 鎻忚堪  锛氳绠楁牎楠屽拰
 * 杈撳叆  锟?
 * 杈撳嚭  锛氭棤	
 */
/*
 * 鍑芥暟鍚嶏細SendSteerData
 * 鎻忚堪  锛氬皢鍙戦€佺粰鏅鸿兘灞傜殑鏁版嵁鍘嬪叆鍙戦€佺紦瀛樹腑
 * 杈撳叆  锛氭棤
 * 杈撳嚭  锛氭棤	
 */
void SendSteerData(void)
{
    if(SteerSendMsg_1.SteerID < STEER_POSITION_SERVO_NUM)
    {
        SteerSendMsgArr[SteerSendMsg_1.SteerID].PosData = SteerSendMsg_1.PosData;
        SteerSendMsgArr[SteerSendMsg_1.SteerID].RunTime = SteerSendMsg_1.RunTime;
        SteerSendMsgArr[SteerSendMsg_1.SteerID].SpeedData = SteerSendMsg_1.SpeedData;
        SendSteer_Position3_SYNC_DataFun();
    }
}



/*
 * 鍑芥暟鍚嶏細SendSteer_SYNC_DataFun
 * 鎻忚堪  锛氬悓姝ュ啓鎸囦护
 * 杈撳叆  锛氭棤
 * 杈撳嚭  锛氭棤	
 */
void SendSteer_SYNC_DataFun(void)
{
    SendSteer_Position3_SYNC_DataFun();
}



/*
 * 鍑芥暟鍚嶏細SendSteer_SYNC_SetMode
 * 鎻忚堪  锛氬悓姝ュ啓鎸囦护
 * 杈撳叆  锛氭棤
 * 杈撳嚭  锛氭棤	
 */
void SendSteer_SYNC_SetMode(u8 SetMode)
{
    u8 sub_mode;

    if((SetMode != STEER_POSITION_MODE) && (SetMode != STEER_SPEED_MODE) && (SetMode != STEER_MIXED_MODE))
    {
        return;
    }

    SteerRunMode = SetMode;
    if(SteerActualMode == SetMode)
    {
        return;
    }

    sub_mode = (SetMode == STEER_SPEED_MODE) ? SUBBOARD_MODE_REMOTE_SPEED : SUBBOARD_MODE_UPPER_POSITION;
    SubBoardProtocol_SendSetMode(sub_mode);
    SteerActualMode = SetMode;
}


/*
 * 鍑芥暟鍚嶏細SendSteer_SYNC_SpeedRun
 * 鎻忚堪  锛氬悓姝ュ啓鎸囦护
 * 杈撳叆  锛氭棤
 * 杈撳嚭  锛氭棤	
 */
void SendSteer_Position3_SYNC_DataFun(void)
{
    SubBoardProtocol_SendServo123Position(SteerSendMsgArr[0].PosData,
                                          SteerSendMsgArr[1].PosData,
                                          SteerSendMsgArr[2].PosData,
                                          SteerSendMsgArr[0].RunTime,
                                          (u16)SteerSendMsgArr[0].SpeedData);
}

void SendSteer_SYNC_SetDefaultMode(void)
{
	SendSteer_SYNC_SetMode(STEER_MIXED_MODE);
}

void SendSteer_Rotate_SpeedRun(void)
{
    /* Servo 4 is owned by the end-effector automatic task on the sub-board. */
}
void SendSteer_SYNC_SpeedRun(void)
{
    SubBoardProtocol_SendServo123Speed(SteerSendMsgArr[0].SpeedData,
                                       SteerSendMsgArr[1].SpeedData,
                                       SteerSendMsgArr[2].SpeedData);
}


/*
 * 鍑芥暟鍚嶏細Steer_ControlProc
 * 鎻忚堪  锛氳埖锟? 閫熷害鎺у埗
 * 杈撳叆  锛氭棤
 * 杈撳嚭  锛氭棤	
 */
void Steer_ControlProc(void)
{
    static u8 auto_req_old = 0U;
    u8 auto_req;
    u8 i;

    SteerRunSpeed = (s16)control_map_3point(g_ctrl_cmd.steer_axis_cmd, Y1_Speed_Arr);
    SteerRunSpeed = -SteerRunSpeed;

    for(i = 0U; i < STEER_POSITION_SERVO_NUM; i++)
    {
        if(g_ctrl_cmd.steer_enable[i] != 0U)
        {
            SteerSpeedValue_SpeedMode(&SteerSendMsgArr[i].SpeedData, &SteerRunSpeed);
        }
        else
        {
            SteerSendMsgArr[i].SpeedData = 0;
        }
    }

    if(SteerRunMode != STEER_SPEED_MODE)
    {
        SteerRunMode = STEER_SPEED_MODE;
        SendSteer_SYNC_SetMode(STEER_SPEED_MODE);
    }

    SendSteer_SYNC_SpeedRun();

    auto_req = (g_ctrl_cmd.steer_enable[STEER_ROTATE_SERVO_INDEX] != 0U) ? 1U : 0U;
    if((auto_req != 0U) && (auto_req_old == 0U))
    {
        SubBoardProtocol_SendAutoStart(END_AUTO_FORWARD_TIME_MS, END_AUTO_REVERSE_TIME_MS, END_AUTO_ROTATE_SPEED);
    }
    else if((auto_req == 0U) && (auto_req_old != 0U))
    {
        SubBoardProtocol_SendAutoStop();
    }
    auto_req_old = auto_req;
}

void SteerSpeedValue_SpeedMode(s16 * SpeedData, s16 * SteerSpeed)
{
	if(*SteerSpeed >= 0)
	{
		*SpeedData = *SteerSpeed;
		*SpeedData &= (~(1<<15));
	}
	else
	{
		*SpeedData = -*SteerSpeed;
		*SpeedData |= (1<<15);
	}
}


void RmtSteer_SpeedFun(void)
{
	Steer_ControlProc();
}



/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/






