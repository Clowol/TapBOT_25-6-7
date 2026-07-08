/******************** (C) COPYRIGHT 2026 *****************************************
  * @file        ros2_comm.c
  * @brief       Compatibility entry for upper-computer communication task.
 *********************************************************************************/
#include "ros2_comm.h"
#include "upper_comm.h"

void ROS2_CommProc(void)
{
	Upper_CommProc();
}

/******************* (C) COPYRIGHT 2026 END OF FILE  *********************************************************************************/
