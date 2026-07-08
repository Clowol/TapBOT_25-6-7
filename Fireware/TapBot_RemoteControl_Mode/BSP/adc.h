/******************** (C) COPYRIGHT 2026 *****************************************
 * @file        adc.h
 * @brief       ADC pin definitions and function prototypes.
 * @note        
 * @warning     
 * @license     This project is released under the MIT License.
 *********************************************************************************/
#ifndef __ADC_H
#define __ADC_H

#include "stm32f10x.h"
#include "function.h"

/*************************** Macro definition *******************************/
#define CHANNEL_NUM        4U       // total 12 channels
#define SCAN_TIMES         20U  	// Take 20 samples per channel

// The voltage values converted by the ADC1 are transferred to the flash via MDA
extern volatile u16 ADC_ConvertedValue[SCAN_TIMES][CHANNEL_NUM];
extern volatile u16 ADC_FilteredValue[CHANNEL_NUM];


/*************************** Function declaration *******************************/
void ADC1_Init(void);

float GetVolt(u16 advalue);  

float GetRes(u16 advalue);

float GetCurrent(u16 advalue);  

void AD_filter(void);


#endif /* __ADC_H */

/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/
