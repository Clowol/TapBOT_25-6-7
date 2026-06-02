/******************** (C) COPYRIGHT 2026 *****************************************
   * @author      Clomol
   * @date        2026-2027
   * @brief       快速映射和限幅函数实现。
   * @license     [z]本代码仅用于教学与科研目的，未经作者书面许可，不得用于商业用途
   *              This project is released under the MIT License.
   * @note        
   * @warning     
*********************************************************************************/
#include "control_math.h"


/************************************************************************
 * @brief   限制浮点数到指定闭区间
 * @param   value     待限制的值
 * @param   min_value 区间最小值
 * @param   max_value 区间最大值
 * @return  限制后的值（若 value 小于 min_value 返回 min_value，大于 max_value 返回 max_value，否则返回 value）
 *************************************************************************/
float control_clamp_float(float value, float min_value, float max_value)
{
    if(value < min_value)
    {
        return min_value;
    }
    if(value > max_value)
    {
        return max_value;
    }
    return value;
}



/***************************************************************************
 * @brief   对输入值应用死区处理（对称死区）
 * @param   value    输入值
 * @param   deadband 死区宽度（非负）
 * @return  如果 |value| <= deadband 则返回 0.0f，否则返回原值
 ****************************************************************************/
float control_apply_deadband(float value, float deadband)
{
    if((value >= -deadband) && (value <= deadband))
    {
        return 0.0f;
    }
    return value;
}

/************************************************************************
 * @brief   三点折线映射表
 * @param   input 输入值（会被自动限制到 x0 和 x2 范围内）
 * @param   table 大小为 2x3 的数组：[x0, x1, x2; y0, y1, y2]
 * @return  映射后的输出值
 * 
 * 详细说明：
 * - 当 x0 < x2 时，输入被限制到 [x0, x2]；若 input <= x1，则使用 (x0,y0)-(x1,y1) 插值；
 *   否则使用 (x1,y1)-(x2,y2) 插值。
 * - 当 x0 > x2 时，输入被限制到 [x2, x0]；若 input >= x1，则使用 (x1,y1)-(x0,y0) 插值；
 *   否则使用 (x1,y1)-(x2,y2) 插值（注意此时 x2 < x1）。
 ***********************************************************************/
float control_map_3point(float input, float table[2][3])
{
    float x0 = table[0][0];
    float x1 = table[0][1];
    float x2 = table[0][2];
    float y0 = table[1][0];
    float y1 = table[1][1];
    float y2 = table[1][2];

    if(x0 < x2)
    {
        input = control_clamp_float(input, x0, x2);
        if(input <= x1)
        {
            return y0 + (input - x0) * (y1 - y0) / (x1 - x0);
        }
        return y1 + (input - x1) * (y2 - y1) / (x2 - x1);
    }
    
    input = control_clamp_float(input, x2, x0);
    if(input >= x1)
    {
        return y1 + (input - x1) * (y0 - y1) / (x0 - x1);
    }
    return y1 + (input - x1) * (y2 - y1) / (x2 - x1);
}



/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/
