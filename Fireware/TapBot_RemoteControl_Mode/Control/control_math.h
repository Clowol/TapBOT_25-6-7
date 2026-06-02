/******************** (C) COPYRIGHT 2026 *****************************************
   * @author      Clomol
   * @date        2026-2027
   * @brief       Common control value conversion helpers.
   * @license     [z]本代码仅用于教学与科研目的，未经作者书面许可，不得用于商业用途
   *              This project is released under the MIT License.
   * @note        
   * @warning     
*********************************************************************************/
#ifndef __CONTROL_MATH_H
#define __CONTROL_MATH_H

#include "stm32f10x.h"

/*********限幅函数*********/
float control_clamp_float(float value, float min_value, float max_value);

/*********三点折现映射函数**************/
float control_map_3point(float input, float table[2][3]);

/*********死区处理函数*********/
float control_apply_deadband(float value, float deadband);


#endif /* __CONTROL_MATH_H */

/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/
