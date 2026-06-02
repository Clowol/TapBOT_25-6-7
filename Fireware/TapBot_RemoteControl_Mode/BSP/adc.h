/******************** (C) COPYRIGHT 2026 *****************************************
   * @author      Clomol
   * @date        2026-2027
   * @brief       
   * @license     [z]本代码仅用于教学与科研目的，未经作者书面许可，不得用于商业用途
   *              This project is released under the MIT License.
   * @note        
   * @warning     
*********************************************************************************/
#ifndef __ADC_H
#define __ADC_H

#include "stm32f10x.h"
#include "function.h"

/*************************** 宏定义*******************************/
#define CHANNEL_NUM        4U       //为12个通道
#define SCAN_TIMES         20U  	//每通道采20次

// ADC1转换的电压值通过MDA方式传到flash
extern volatile u16 ADC_ConvertedValue[SCAN_TIMES][CHANNEL_NUM];
extern volatile u16 ADC_FilteredValue[CHANNEL_NUM];


/*************************** ADC函数声明*******************************/
void ADC1_Init(void);

float GetVolt(u16 advalue);  

float GetRes(u16 advalue);

float GetCurrent(u16 advalue);  

void AD_filter(void);


#endif /* __ADC_H */

/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/
