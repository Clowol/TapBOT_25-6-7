/******************** (C) COPYRIGHT 2026 *****************************************
 * @file    subboard_link.c
 * @brief   UART5 byte buffering and 0xA6 frame transport for the end-effector board.
 * @note        
 * @warning     
 * @license     This project is released under the MIT License.
 *********************************************************************************/
#include "subboard_link.h"
#include "subboard_protocol.h"
#include "ring_buf.h"
#include "usart.h"
#include "app_config.h"

#define SUBBOARD_FRAME_HEAD        0xA6U
#define SUBBOARD_NODE_ID           0x01U
#define SUBBOARD_ONLINE_TIMEOUT    50U
#define SUBBOARD_RS485_DIR_RCC     RCC_APB2Periph_GPIOE
#define SUBBOARD_RS485_DIR_PORT    GPIOE
#define SUBBOARD_RS485_DIR_PIN     GPIO_Pin_0

typedef enum
{
    SUBBOARD_PARSE_WAIT_HEAD = 0,
    SUBBOARD_PARSE_NODE,
    SUBBOARD_PARSE_CMD,
    SUBBOARD_PARSE_LEN,
    SUBBOARD_PARSE_PAYLOAD,
    SUBBOARD_PARSE_CRC_L,
    SUBBOARD_PARSE_CRC_H
} subboard_parse_state_t;

static u8 SubBoardRxStorage[SUBBOARD_RX_RING_SIZE];
static ring_buf_t SubBoardRxRing;
static subboard_parse_state_t ParseState;
static u8 ParseNode;
static u8 ParseCmd;
static u8 ParseLen;
static u8 ParsePayload[SUBBOARD_FRAME_MAX_PAYLOAD];
static u8 ParseIndex;
static u16 ParseCrc;
static u8 LastState;
static u8 LastError;
static u8 LastAckCmd;
static u8 OnlineTicks;

void SubBoard_LinkRs485SetRx(void)
{
#if APP_SUBBOARD_RS485_AUTO_DIR == 0U
    GPIO_ResetBits(SUBBOARD_RS485_DIR_PORT, SUBBOARD_RS485_DIR_PIN);
#endif
}

void SubBoard_LinkRs485SetTx(void)
{
#if APP_SUBBOARD_RS485_AUTO_DIR == 0U
    GPIO_SetBits(SUBBOARD_RS485_DIR_PORT, SUBBOARD_RS485_DIR_PIN);
#endif
}

void SubBoard_LinkRs485DirInit(void)
{
#if APP_SUBBOARD_RS485_AUTO_DIR == 0U
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(SUBBOARD_RS485_DIR_RCC, ENABLE);

    GPIO_InitStructure.GPIO_Pin = SUBBOARD_RS485_DIR_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(SUBBOARD_RS485_DIR_PORT, &GPIO_InitStructure);

    SubBoard_LinkRs485SetRx();
#endif
}

static u16 SubBoard_Crc16CcittUpdate(u16 crc, u8 data)
{
    u8 i;

    crc ^= ((u16)data << 8);
    for(i = 0U; i < 8U; i++)
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

    return crc;
}

static u16 SubBoard_CalcFrameCrc(u8 node, u8 cmd, u8 len, const u8 *payload)
{
    u8 i;
    u16 crc = 0xFFFFU;

    crc = SubBoard_Crc16CcittUpdate(crc, node);
    crc = SubBoard_Crc16CcittUpdate(crc, cmd);
    crc = SubBoard_Crc16CcittUpdate(crc, len);
    for(i = 0U; i < len; i++)
    {
        crc = SubBoard_Crc16CcittUpdate(crc, payload[i]);
    }

    return crc;
}

static u8 SubBoard_SendByte(u8 data)
{
    u32 timeout = 0x0000FFFFU;

    USART_SendData(UART5, data);
    while(((UART5->SR & 0x40U) == 0U) && (timeout > 0U))
    {
        timeout--;
    }

    return (timeout > 0U) ? 1U : 0U;
}

static void SubBoard_ParseByte(u8 data)
{
    u16 crc_calc;

    switch(ParseState)
    {
        case SUBBOARD_PARSE_WAIT_HEAD:
            if(data == SUBBOARD_FRAME_HEAD)
            {
                ParseState = SUBBOARD_PARSE_NODE;
            }
            break;

        case SUBBOARD_PARSE_NODE:
            ParseNode = data;
            ParseState = SUBBOARD_PARSE_CMD;
            break;

        case SUBBOARD_PARSE_CMD:
            ParseCmd = data;
            ParseState = SUBBOARD_PARSE_LEN;
            break;

        case SUBBOARD_PARSE_LEN:
            ParseLen = data;
            ParseIndex = 0U;
            if(ParseLen > SUBBOARD_FRAME_MAX_PAYLOAD)
            {
                ParseState = SUBBOARD_PARSE_WAIT_HEAD;
            }
            else if(ParseLen == 0U)
            {
                ParseState = SUBBOARD_PARSE_CRC_L;
            }
            else
            {
                ParseState = SUBBOARD_PARSE_PAYLOAD;
            }
            break;

        case SUBBOARD_PARSE_PAYLOAD:
            ParsePayload[ParseIndex++] = data;
            if(ParseIndex >= ParseLen)
            {
                ParseState = SUBBOARD_PARSE_CRC_L;
            }
            break;

        case SUBBOARD_PARSE_CRC_L:
            ParseCrc = data;
            ParseState = SUBBOARD_PARSE_CRC_H;
            break;

        case SUBBOARD_PARSE_CRC_H:
            ParseCrc |= ((u16)data << 8);
            crc_calc = SubBoard_CalcFrameCrc(ParseNode, ParseCmd, ParseLen, ParsePayload);
            if((ParseNode == SUBBOARD_NODE_ID) && (ParseCrc == crc_calc))
            {
                OnlineTicks = SUBBOARD_ONLINE_TIMEOUT;
                SubBoardProtocol_OnFrame(ParseCmd, ParsePayload, ParseLen);
            }
            ParseState = SUBBOARD_PARSE_WAIT_HEAD;
            break;

        default:
            ParseState = SUBBOARD_PARSE_WAIT_HEAD;
            break;
    }
}

void SubBoard_LinkInit(void)
{
    SubBoard_LinkRs485DirInit();
    ring_buf_init(&SubBoardRxRing, SubBoardRxStorage, SUBBOARD_RX_RING_SIZE);
    ParseState = SUBBOARD_PARSE_WAIT_HEAD;
    ParseNode = 0U;
    ParseCmd = 0U;
    ParseLen = 0U;
    ParseIndex = 0U;
    ParseCrc = 0U;
    LastState = SUBBOARD_STATE_UNKNOWN;
    LastError = SUBBOARD_ERR_NONE;
    LastAckCmd = 0U;
    OnlineTicks = 0U;
}

void SubBoard_LinkOnRxByte(u8 data)
{
    (void)ring_buf_put(&SubBoardRxRing, &data, 1U);
}

void SubBoard_LinkProc(void)
{
    u8 data;
    u8 budget = COMM_PARSE_BUDGET_BYTES;

    while((budget > 0U) && (ring_buf_get(&SubBoardRxRing, &data, 1U) == 1U))
    {
        budget--;
        SubBoard_ParseByte(data);
    }
}

void SubBoard_LinkTick10ms(void)
{
    if(OnlineTicks > 0U)
    {
        OnlineTicks--;
    }
}

u8 SubBoard_LinkSendFrame(u8 cmd_id, const u8 *payload, u8 len)
{
    u8 i;
    u16 crc;
    u8 ok = 1U;

    if((payload == 0) && (len > 0U))
    {
        return 0U;
    }
    if(len > SUBBOARD_FRAME_MAX_PAYLOAD)
    {
        return 0U;
    }

    crc = SubBoard_CalcFrameCrc(SUBBOARD_NODE_ID, cmd_id, len, payload);

    SubBoard_LinkRs485SetTx();

    if(SubBoard_SendByte(SUBBOARD_FRAME_HEAD) == 0U) ok = 0U;
    if((ok != 0U) && (SubBoard_SendByte(SUBBOARD_NODE_ID) == 0U)) ok = 0U;
    if((ok != 0U) && (SubBoard_SendByte(cmd_id) == 0U)) ok = 0U;
    if((ok != 0U) && (SubBoard_SendByte(len) == 0U)) ok = 0U;
    for(i = 0U; i < len; i++)
    {
        if((ok != 0U) && (SubBoard_SendByte(payload[i]) == 0U))
        {
            ok = 0U;
        }
    }
    if((ok != 0U) && (SubBoard_SendByte((u8)(crc & 0xFFU)) == 0U)) ok = 0U;
    if((ok != 0U) && (SubBoard_SendByte((u8)(crc >> 8)) == 0U)) ok = 0U;

    SubBoard_LinkRs485SetRx();

    return ok;
}

u8 SubBoard_LinkIsOnline(void)
{
    return (OnlineTicks > 0U) ? 1U : 0U;
}

u8 SubBoard_LinkGetLastState(void)
{
    return LastState;
}

u8 SubBoard_LinkGetLastError(void)
{
    return LastError;
}

u8 SubBoard_LinkGetLastAckCmd(void)
{
    return LastAckCmd;
}

void SubBoard_LinkSetStatus(u8 state, u8 error)
{
    LastState = state;
    LastError = error;
}

void SubBoard_LinkSetAck(u8 state, u8 status, u8 ack_cmd)
{
    LastState = state;
    LastError = status;
    LastAckCmd = ack_cmd;
}

/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/
