/******************** (C) COPYRIGHT 2018 *****************************************
 * 文件名  ：rgt_engine.c
 * 描述    ：右 主机启动、停止控制
 * 硬件配置：         
 * 版本    ：
 * 修改日期： 
 * 作者    ：
 * 修改日志：
*********************************************************************************/
#ifndef __RGT_ENGINE_H
#define	__RGT_ENGINE_H

#include "stm32f10x.h"
#include "switch.h"
#include "function.h"

#define IDLE_SPEED			400										//怠速值  使用时候可以加一个范围


extern u8 g_RgtEngSttStpCtrl;									//启动右主机命令 			0-停止  1-启动  0xFF-无效  点触型
extern u8 g_RgtEngSttStpState;								//右主机启动状态 		0-停止  1-启动  0xFF-无效  
extern u16 g_RgtSpeedRealVal;									//右主机转速值			[0       3500]
extern float g_fRgtEngWaterTemp;              //右主机水温
extern float g_fRgtEngOilPress;               //右主机油压


void ThroCtrlFun(void);
void RgtEngCtrlFun(void);
void RgtEngRmtUpCtrlFun(void);
void RgtEngSttStpCtrlFun(void);
void RgtEngWorkCheckFun(void);
void RgtEngStopCtrlFun(void);
void RgtEngStartCtrlFun(void);
void RgtEngineStartProtectFun(void);
void LineChangeSetThro(float SetThro);
void RgtEngParReadFun(void);

#endif

/******************* (C) COPYRIGHT 2016 END OF FILE *****************************/




