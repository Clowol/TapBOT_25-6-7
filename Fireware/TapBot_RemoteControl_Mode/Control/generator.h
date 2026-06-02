/******************** (C) COPYRIGHT 2018 *****************************************
 * 文件名  ：generator.h
 * 描述    ：发电机启动   
 * 硬件配置：         
 * 版本    ：
 * 修改日期： 
 * 作者    ：xiang
 * 修改日志：
*********************************************************************************/
#ifndef __GENERATOR_H
#define	__GENERATOR_H

#include "stm32f10x.h"
#include "function.h"

#define GEN_OIL_PRESS 40						//判断发电机工作的油压值


extern u8 GenSttStpKeepFlg;					//发电机启停控制  保持型  0-停止  1-启动  0xFF-无效
extern u8 g_GenSttStpCtrl;							//启动发电机命令 		0-停止  1-启动  0xFF-无效  点触型
extern u8 g_GenSttStpState;							//启动发电机状态 		0-停止  1-启动  0xFF-无效  


void GenCtrlFun(void);
void GenRmtUpCtrlFun(void);
void GenSttStpCtrlFun(void);
void GenStopCtrlFun(void);
void GenStartCtrlFun(void);
void GenReadValueFun(void);
void GenWorkCheckFun(void);


#endif

/******************* (C) COPYRIGHT 2018 END OF FILE *****************************/




