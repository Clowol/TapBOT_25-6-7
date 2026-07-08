/******************** (C) COPYRIGHT 2026 *****************************************
  * @file        rmt_comm.c
  * @brief       Remote controller serial protocol parser task.
 *********************************************************************************/
#include "rmt_comm.h"

#include "usart.h"

#include "rmt_data.h"
#include "app_config.h"



#define RMT_CBUS_FRAME_HEAD        0x66U	// 帧头字节
#define RMT_CBUS_CTRL_LEN          0x19U	// 控制帧长度指示
#define RMT_CBUS_STATE_OFFSET      31U		// 状态帧起始偏移

#define RMT_CBUS_STATE_LEN         0x08U	// 状态帧长度指示
#define RMT_CBUS_FRAME_LEN         45U		// 总帧长



void Rmt_CommProc(void)
{
	static uint8_t frame_buf[RMT_CBUS_FRAME_LEN];	// 缓存完整的 45 字节帧
	static uint32_t frame_len = 0U;					// 已接收字节数

	uint8_t byte_data;
	uint8_t budget = COMM_PARSE_BUDGET_BYTES;


	while((budget > 0U) && (USART3_RxRead(&byte_data, 1U) == 1U))
	{
		budget--;
		if(frame_len == 0U)
		{
			if(byte_data != RMT_CBUS_FRAME_HEAD)
			{
				continue;
			}
		}

		frame_buf[frame_len++] = byte_data;

		
		if(frame_len == 2U)
		{
			if(frame_buf[1] != RMT_CBUS_CTRL_LEN)
			{
				frame_len = 0U;
			}
		}
		else if(frame_len == (RMT_CBUS_STATE_OFFSET + 2U))
		{
			if((frame_buf[RMT_CBUS_STATE_OFFSET] != RMT_CBUS_FRAME_HEAD) ||
				(frame_buf[RMT_CBUS_STATE_OFFSET + 1U] != RMT_CBUS_STATE_LEN))
			{
				frame_len = 0U;
			}
		}
		else if(frame_len >= RMT_CBUS_FRAME_LEN)
		{
			Proce_Rmtdata(frame_buf);
			frame_len = 0U;
		}
	}
}



/******************* (C) COPYRIGHT 2026 END OF FILE  *********************************************************************************/
