/******************** (C) COPYRIGHT 2026 *****************************************
  * @file        upper_comm.c
  * @brief       Parses upper-computer frames from USART2 ring buffer.
  *
  * @param		Frame:0xA5 cmd_id payload_len payload crc_low crc_high
  *   
  * @note        CRC16-CCITT:
  * 				init 0xFFFF, poly 0x1021, over cmd_id + payload_len + payload.
  * @warning     
  * @license     This project is released under the MIT License.
 *********************************************************************************/
#include "upper_comm.h"
#include "usart.h"
#include "control_dispatcher.h"
#include "app_config.h"


/*		-----------------------------------------------------------------
		| 0xA5 | cmd_id | len | payload (len bytes) | crc_low | crc_high |
		-----------------------------------------------------------------
*/

#define UPPER_FRAME_HEAD         0xA5U
#define UPPER_PAYLOAD_MAX_LEN    32U

/* upper to board -> parse state*/
typedef enum
{
	UPPER_PARSE_HEAD = 0,
	UPPER_PARSE_CMD,
	UPPER_PARSE_LEN,
	UPPER_PARSE_PAYLOAD,
	UPPER_PARSE_CRC_L,
	UPPER_PARSE_CRC_H
} upper_parse_state_t;


/*============================== Static functions ==================================*/
/* 计算 CRC16‑CCITT */
static u16 Upper_Crc16Ccitt(const u8 *data, u16 len)
{
	u16 crc = 0xFFFFU;
	u16 i;
	u8 bit;

	for(i = 0U; i < len; i++)
	{
		crc ^= ((u16)data[i] << 8);
		for(bit = 0U; bit < 8U; bit++)
		{
			if((crc & 0x8000U) != 0U)
			{
				crc = (u16)((crc << 1) ^ 0x1021U);
			}
			else
			{
				crc <<= 1;
			}
		}
	}

	return crc;
}


/*============================ External functions =====================================*/
/*  */
void Upper_CommProc(void)
{
	static upper_parse_state_t state = UPPER_PARSE_HEAD;
	static u8 cmd_id = 0U;
	static u8 payload_len = 0U;
	static u8 payload_index = 0U;			// Number of bytes received
	static u8 crc_low = 0U;			
	static u8 crc_buf[UPPER_PAYLOAD_MAX_LEN + 2U];  // cmd_id + len + payload
	static u8 payload[UPPER_PAYLOAD_MAX_LEN];		// Payload data buffer

	u8 byte_data;
	u8 budget = COMM_PARSE_BUDGET_BYTES;		// Set the analysis budget
	u16 crc_calc;
	u16 crc_recv;

	while((budget > 0U) && (USART2_RxRead(&byte_data, 1U) == 1U))
	{
		budget--;
		switch(state)
		{
			case UPPER_PARSE_HEAD:
				if(byte_data == UPPER_FRAME_HEAD)
				{
					state = UPPER_PARSE_CMD;
				}
				break;

			case UPPER_PARSE_CMD:
				cmd_id = byte_data;
				crc_buf[0] = cmd_id;
				state = UPPER_PARSE_LEN;
				break;

			case UPPER_PARSE_LEN:
				payload_len = byte_data;
				payload_index = 0U;
				crc_buf[1] = payload_len;
				if(payload_len > UPPER_PAYLOAD_MAX_LEN)
				{
					state = UPPER_PARSE_HEAD;
				}
				else if(payload_len == 0U)
				{
					state = UPPER_PARSE_CRC_L;
				}
				else
				{
					state = UPPER_PARSE_PAYLOAD;
				}
				break;

			case UPPER_PARSE_PAYLOAD:
				payload[payload_index] = byte_data;
				crc_buf[2U + payload_index] = byte_data;
				
				payload_index++;

				if(payload_index >= payload_len)
				{
					state = UPPER_PARSE_CRC_L;
				}
				break;

			case UPPER_PARSE_CRC_L:
				crc_low = byte_data;
				state = UPPER_PARSE_CRC_H;
				break;

			case UPPER_PARSE_CRC_H:
				crc_recv = (u16)crc_low | ((u16)byte_data << 8);
				crc_calc = Upper_Crc16Ccitt(crc_buf, (u16)(payload_len + 2U));
				if(crc_recv == crc_calc)
				{
					ControlDispatcher_OnUpperPacketReceived(cmd_id, payload, payload_len);
				}
				state = UPPER_PARSE_HEAD;
				break;

			default:
				state = UPPER_PARSE_HEAD;
				break;
		}
	}
}

/******************* (C) COPYRIGHT 2026 END OF FILE  *********************************************************************************/
