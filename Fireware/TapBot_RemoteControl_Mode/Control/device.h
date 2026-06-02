/******************** (C) COPYRIGHT 2017 *****************************************
 * 文件名  ：device.h
 * 描述    ：device相关函数头文件
 * 硬件配置：         
 * 版本    ： 
 * 修改日期： 
 * 作者    ： 
 * 修改日志： 
*********************************************************************************/
#ifndef __DEVICE_H
#define	__DEVICE_H

#include "sys.h"
#include "function.h"
#include "lft_engine.h"
#include "rgt_engine.h"


extern u8 g_DeviceSttStpCtrl;                         //设备启停   0-停止  1-启动  0xFF-无效  保持性 

void DeviCtrlFun(void);


#endif /* __DEVICE_H */


/******************* (C) COPYRIGHT 2017 END OF FILE *****************************/
