/******************** (C) COPYRIGHT 2026 *****************************************
 * @file        yushu_motor_data.h
 * @brief       Unitree M8010 motor control data structures and declarations.
 * @note        
 * @warning     
 * @license     This project is released under the MIT License.
 *********************************************************************************/
#ifndef __YUSHU_MOTOR_DATA_H
#define __YUSHU_MOTOR_DATA_H

#include "function.h"

enum 
{
	YUSHU_STOP   		= 0,															  //停止
	YUSHU_SHORTEN 		= 1,																//缩短 shorten
	YUSHU_STRETCH 	 	= 2,																//伸长 stretch
};


/**********************************  Global Variables ********************************************/
extern float YushuSpeed;


/**********************************  Function prototype ********************************************/
void SendYushuMotorDataFun(void);
void YushuMotor_SendControl(void);

void RmtYushuMotor_MoveCmd(void);

void YushuMotor_ControlProc(void);

void RmtYushuMotor_SpeedFun(void);


#endif



/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/


