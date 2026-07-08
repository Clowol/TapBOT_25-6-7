/******************** (C) COPYRIGHT 2026 *****************************************
 * @file        control_math.h
 * @brief       Common control value conversion function prototypes.
 * @note        
 * @warning     
 * @license     This project is released under the MIT License.
 *********************************************************************************/
#ifndef __CONTROL_MATH_H

#define __CONTROL_MATH_H

#include "stm32f10x.h"


/* Restrict floating-point numbers to a specified range */
float control_clamp_float(float value, float min_value, float max_value);


/* Maps the input to the output using a three-segment line segment (three points) */
float control_map_3point(float input, float table[2][3]);


/* Apply a dead band to the input values to eliminate small-signal jitter */
float control_apply_deadband(float value, float deadband);



#endif /* __CONTROL_MATH_H */



/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/

