/******************** (C) COPYRIGHT 2024 *****************************************
 * 文件�? ：yushu_motor_data.c
 * 描述    ：宇树电机数据发�?
 * 硬件配置�?        
 * 版本    �?
 * 修改日期�?
 * 作�?   : 
 * 修改日志:
*********************************************************************************/ 
#ifndef __YUSHU_MOTOR_DATA_H
#define __YUSHU_MOTOR_DATA_H

#include "function.h"



enum 
{
	YUSHU_STOP   			= 0,															  //停止
	YUSHU_SHORTEN 		= 1,																//缩短 shorten
	YUSHU_STRETCH 	 	= 2,																//伸长 stretch
};



extern float YushuSpeed;


void SendYushuMotorDataFun(void);
void YushuMotor_SendControl(void);
void RmtYushuMotor_MoveCmd(void);
void YushuMotor_ControlProc(void);
void RmtYushuMotor_SpeedFun(void);


#endif



/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/


