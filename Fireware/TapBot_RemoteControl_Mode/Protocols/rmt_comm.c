/******************** (C) COPYRIGHT 2024 *****************************************
 * File Name  : rmt_comm.c
 * Description: Remote controller serial protocol parser task.
*********************************************************************************/
#include "rmt_comm.h"
#include "usart.h"
#include "rmt_data.h"

#define RMT_CBUS_FRAME_HEAD        0x66U
#define RMT_CBUS_CTRL_LEN          0x19U
#define RMT_CBUS_STATE_OFFSET      31U
#define RMT_CBUS_STATE_LEN         0x08U
#define RMT_CBUS_FRAME_LEN         45U

void Rmt_CommProc(void)
{
	static uint8_t frame_buf[RMT_CBUS_FRAME_LEN];
	static uint32_t frame_len = 0U;
	uint8_t byte_data;

	while(USART3_RxRead(&byte_data, 1U) == 1U)
	{
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

/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/
