#include "rs485_link.h"
#include "ring_buf.h"
#include "usart.h"
#include "subboard_protocol.h"

#define RS485_DE_RCC      RCC_APB2Periph_GPIOA
#define RS485_DE_PORT     GPIOA
#define RS485_DE_PIN      GPIO_Pin_8
#define FRAME_HEAD        0xA6U
#define NODE_ID           0x01U
#define RX_SIZE           128U
#define MAX_PAYLOAD       32U

typedef enum
{
    PARSE_HEAD = 0,
    PARSE_NODE,
    PARSE_CMD,
    PARSE_LEN,
    PARSE_PAYLOAD,
    PARSE_CRC_L,
    PARSE_CRC_H
} parse_state_t;

static u8 RxStorage[RX_SIZE];
static ring_buf_t RxRing;
static parse_state_t ParseState;
static u8 ParseNode;
static u8 ParseCmd;
static u8 ParseLen;
static u8 ParseIndex;
static u8 ParsePayload[MAX_PAYLOAD];
static u16 ParseCrc;

static void Rs485_SetTx(void)
{
    GPIO_SetBits(RS485_DE_PORT, RS485_DE_PIN);
}

static void Rs485_SetRx(void)
{
    GPIO_ResetBits(RS485_DE_PORT, RS485_DE_PIN);
}

static u16 CrcUpdate(u16 crc, u8 data)
{
    u8 i;
    crc ^= ((u16)data << 8);
    for(i = 0U; i < 8U; i++)
    {
        crc = ((crc & 0x8000U) != 0U) ? (u16)((crc << 1) ^ 0x1021U) : (u16)(crc << 1);
    }
    return crc;
}

static u16 CalcCrc(u8 node, u8 cmd, u8 len, const u8 *payload)
{
    u8 i;
    u16 crc = 0xFFFFU;
    crc = CrcUpdate(crc, node);
    crc = CrcUpdate(crc, cmd);
    crc = CrcUpdate(crc, len);
    for(i = 0U; i < len; i++)
    {
        crc = CrcUpdate(crc, payload[i]);
    }
    return crc;
}

static void ParseByte(u8 data)
{
    u16 crc_calc;

    switch(ParseState)
    {
        case PARSE_HEAD:
            if(data == FRAME_HEAD) ParseState = PARSE_NODE;
            break;
        case PARSE_NODE:
            ParseNode = data;
            ParseState = PARSE_CMD;
            break;
        case PARSE_CMD:
            ParseCmd = data;
            ParseState = PARSE_LEN;
            break;
        case PARSE_LEN:
            ParseLen = data;
            ParseIndex = 0U;
            if(ParseLen > MAX_PAYLOAD) ParseState = PARSE_HEAD;
            else if(ParseLen == 0U) ParseState = PARSE_CRC_L;
            else ParseState = PARSE_PAYLOAD;
            break;
        case PARSE_PAYLOAD:
            ParsePayload[ParseIndex++] = data;
            if(ParseIndex >= ParseLen) ParseState = PARSE_CRC_L;
            break;
        case PARSE_CRC_L:
            ParseCrc = data;
            ParseState = PARSE_CRC_H;
            break;
        case PARSE_CRC_H:
            ParseCrc |= ((u16)data << 8);
            crc_calc = CalcCrc(ParseNode, ParseCmd, ParseLen, ParsePayload);
            if((ParseNode == NODE_ID) && (ParseCrc == crc_calc))
            {
                SubProtocol_OnFrame(ParseCmd, ParsePayload, ParseLen);
            }
            ParseState = PARSE_HEAD;
            break;
        default:
            ParseState = PARSE_HEAD;
            break;
    }
}

void Rs485Link_Init(void)
{
    GPIO_InitTypeDef gpio;

    RCC_APB2PeriphClockCmd(RS485_DE_RCC, ENABLE);
    gpio.GPIO_Pin = RS485_DE_PIN;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(RS485_DE_PORT, &gpio);
    Rs485_SetRx();

    ring_buf_init(&RxRing, RxStorage, RX_SIZE);
    ParseState = PARSE_HEAD;
}

void Rs485Link_OnRxByte(u8 data)
{
    (void)ring_buf_put(&RxRing, &data, 1U);
}

void Rs485Link_Proc(void)
{
    u8 data;
    while(ring_buf_get(&RxRing, &data, 1U) == 1U)
    {
        ParseByte(data);
    }
}

u8 Rs485Link_SendFrame(u8 cmd_id, const u8 *payload, u8 len)
{
    u8 i;
    u16 crc;

    if((payload == 0) && (len > 0U)) return 0U;
    if(len > MAX_PAYLOAD) return 0U;

    crc = CalcCrc(NODE_ID, cmd_id, len, payload);
    Rs485_SetTx();
    SubUart_SendByte(FRAME_HEAD);
    SubUart_SendByte(NODE_ID);
    SubUart_SendByte(cmd_id);
    SubUart_SendByte(len);
    for(i = 0U; i < len; i++) SubUart_SendByte(payload[i]);
    SubUart_SendByte((u8)(crc & 0xFFU));
    SubUart_SendByte((u8)(crc >> 8));
    Rs485_SetRx();
    return 1U;
}
