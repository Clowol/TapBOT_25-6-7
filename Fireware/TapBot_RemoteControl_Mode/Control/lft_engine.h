/******************** (C) COPYRIGHT 2018 *****************************************
 * 文件名  ：lft_engine.c
 * 描述    ：左  主机启动、停止控制
 * 硬件配置：         
 * 版本    ：
 * 修改日期： 
 * 作者    ：
 * 修改日志：
*********************************************************************************/
#ifndef __LFT_ENGINE_H
#define	__LFT_ENGINE_H

#include "stm32f10x.h"
#include "switch.h"
#include "function.h"

#define IDLE_SPEED			400										//怠速值  使用时候可以加一个范围


extern u8 g_LftEngSttStpCtrl;									//启动左主机命令 			0-停止  1-启动  0xFF-无效  点触型
extern u8 g_LftEngSttStpState;								//左主机启动状态 		0-停止  1-启动  0xFF-无效  
extern u16 g_LftSpeedRealVal;									//左主机转速值			[0       3500]
extern float g_fLftEngWaterTemp;							//左主机水温
extern float g_fLftEngOilPress;             	//左主机油压


void ThroCtrlFun(void);
void LftEngCtrlFun(void);
void LftEngRmtUpCtrlFun(void);
void LftEngSttStpCtrlFun(void);
void LftEngWorkCheckFun(void);
void LftEngStopCtrlFun(void);
void LftEngStartCtrlFun(void);
void LftEngineStartProtectFun(void);
void LineChangeSetThro(float SetThro);
void LftEngParReadFun(void);

#endif

/******************* (C) COPYRIGHT 2016 END OF FILE *****************************/




