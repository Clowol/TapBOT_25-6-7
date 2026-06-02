/******************** (C) COPYRIGHT 2024 *****************************************
 * File Name  : ros2_comm.c
 * Description: Compatibility entry for upper-computer communication task.
*********************************************************************************/
#include "ros2_comm.h"
#include "upper_comm.h"

void ROS2_CommProc(void)
{
	Upper_CommProc();
}

/******************* (C) COPYRIGHT 2024 END OF FILE *****************************/
