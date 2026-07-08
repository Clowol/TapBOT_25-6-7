#include "feetech_servo.h"
#include "app_config.h"
#include "usart.h"

#define SMS_FRAME_HEAD          0xFFU
#define SMS_BROADCAST_ID        0xFEU
#define SMS_INST_WRITE          0x03U
#define SMS_INST_SYNC_WRITE     0x83U
#define SMS_REG_MODE            0x21U
#define SMS_REG_GOAL_POS        0x2AU
#define SMS_REG_GOAL_SPEED      0x2EU
#define SMS_MODE_POSITION       0x00U
#define SMS_MODE_SPEED          0x01U

static const u8 s_servo_ids[END_SERVO_NUM] =
{
    END_SERVO_ID0,
    END_SERVO_ID1,
    END_SERVO_ID2,
    END_SERVO_ID3
};

static u8 FeetechServo_Checksum(const u8 *data, u8 len)
{
    u16 sum = 0U;
    u8 i;

    for(i = 0U; i < len; i++)
    {
        sum += data[i];
    }

    return (u8)(~sum);
}

static void FeetechServo_SendByte(u8 data)
{
    SubUart_SendByte(data);
}

static void FeetechServo_SendFrame(u8 id, u8 inst, const u8 *params, u8 param_len)
{
    u8 checksum_buf[32];
    u8 checksum_len;
    u8 i;

    if(param_len > 28U)
    {
        return;
    }

    checksum_len = (u8)(param_len + 3U);
    checksum_buf[0] = id;
    checksum_buf[1] = (u8)(param_len + 2U);
    checksum_buf[2] = inst;
    for(i = 0U; i < param_len; i++)
    {
        checksum_buf[3U + i] = params[i];
    }

    FeetechServo_SendByte(SMS_FRAME_HEAD);
    FeetechServo_SendByte(SMS_FRAME_HEAD);
    for(i = 0U; i < checksum_len; i++)
    {
        FeetechServo_SendByte(checksum_buf[i]);
    }
    FeetechServo_SendByte(FeetechServo_Checksum(checksum_buf, checksum_len));
}

static s16 FeetechServo_ClampPos(s16 pos)
{
    if(pos < END_SERVO_POS_MIN)
    {
        return END_SERVO_POS_MIN;
    }
    if(pos > END_SERVO_POS_MAX)
    {
        return END_SERVO_POS_MAX;
    }
    return pos;
}

static u16 FeetechServo_EncodeSpeed(s16 speed)
{
    u16 value;

    if(speed < 0)
    {
        value = (u16)(-speed);
        if(value > END_SERVO_SPEED_MAX)
        {
            value = END_SERVO_SPEED_MAX;
        }
        value |= 0x8000U;
    }
    else
    {
        value = (u16)speed;
        if(value > END_SERVO_SPEED_MAX)
        {
            value = END_SERVO_SPEED_MAX;
        }
    }

    return value;
}

static void FeetechServo_WriteByte(u8 id, u8 reg, u8 value)
{
    u8 params[2];

    params[0] = reg;
    params[1] = value;
    FeetechServo_SendFrame(id, SMS_INST_WRITE, params, 2U);
}

static void FeetechServo_WriteWord(u8 id, u8 reg, u16 value)
{
    u8 params[3];

    params[0] = reg;
    params[1] = (u8)(value & 0xFFU);
    params[2] = (u8)((value >> 8) & 0xFFU);
    FeetechServo_SendFrame(id, SMS_INST_WRITE, params, 3U);
}

static void FeetechServo_SyncWrite3(u8 start_reg, u8 data_len, const u8 *data)
{
    u8 params[2U + (END_SERVO_POSITION_NUM * 7U)];
    u8 idx = 0U;
    u8 i;
    u8 j;

    if((data == 0) || (data_len > 6U))
    {
        return;
    }

    params[idx++] = start_reg;
    params[idx++] = data_len;
    for(i = 0U; i < END_SERVO_POSITION_NUM; i++)
    {
        params[idx++] = s_servo_ids[i];
        for(j = 0U; j < data_len; j++)
        {
            params[idx++] = data[(i * data_len) + j];
        }
    }

    FeetechServo_SendFrame(SMS_BROADCAST_ID, SMS_INST_SYNC_WRITE, params, idx);
}

void FeetechServo_Init(void)
{
    FeetechServo_StopAll();
}

void FeetechServo_SetModeSpeed123(void)
{
    u8 i;

    for(i = 0U; i < END_SERVO_POSITION_NUM; i++)
    {
        FeetechServo_WriteByte(s_servo_ids[i], SMS_REG_MODE, SMS_MODE_SPEED);
    }
}

void FeetechServo_SetModePosition123(void)
{
    u8 i;

    for(i = 0U; i < END_SERVO_POSITION_NUM; i++)
    {
        FeetechServo_WriteByte(s_servo_ids[i], SMS_REG_MODE, SMS_MODE_POSITION);
    }
}

void FeetechServo_SetSpeed123(s16 speed0, s16 speed1, s16 speed2)
{
    u8 data[END_SERVO_POSITION_NUM * 2U];
    u16 speeds[END_SERVO_POSITION_NUM];
    u8 i;

    speeds[0] = FeetechServo_EncodeSpeed(speed0);
    speeds[1] = FeetechServo_EncodeSpeed(speed1);
    speeds[2] = FeetechServo_EncodeSpeed(speed2);

    for(i = 0U; i < END_SERVO_POSITION_NUM; i++)
    {
        data[(i * 2U)] = (u8)(speeds[i] & 0xFFU);
        data[(i * 2U) + 1U] = (u8)((speeds[i] >> 8) & 0xFFU);
    }

    FeetechServo_SyncWrite3(SMS_REG_GOAL_SPEED, 2U, data);
}

void FeetechServo_SetPosition123(s16 pos0, s16 pos1, s16 pos2, u16 run_time, u16 speed)
{
    u8 data[END_SERVO_POSITION_NUM * 6U];
    s16 pos[END_SERVO_POSITION_NUM];
    u16 speed_limited;
    u8 i;

    pos[0] = FeetechServo_ClampPos(pos0);
    pos[1] = FeetechServo_ClampPos(pos1);
    pos[2] = FeetechServo_ClampPos(pos2);
    speed_limited = (speed > END_SERVO_SPEED_MAX) ? END_SERVO_SPEED_MAX : speed;

    for(i = 0U; i < END_SERVO_POSITION_NUM; i++)
    {
        data[(i * 6U)] = (u8)((u16)pos[i] & 0xFFU);
        data[(i * 6U) + 1U] = (u8)(((u16)pos[i] >> 8) & 0xFFU);
        data[(i * 6U) + 2U] = (u8)(run_time & 0xFFU);
        data[(i * 6U) + 3U] = (u8)((run_time >> 8) & 0xFFU);
        data[(i * 6U) + 4U] = (u8)(speed_limited & 0xFFU);
        data[(i * 6U) + 5U] = (u8)((speed_limited >> 8) & 0xFFU);
    }

    FeetechServo_SyncWrite3(SMS_REG_GOAL_POS, 6U, data);
}

void FeetechServo_SetRotateSpeed(s16 speed)
{
    FeetechServo_WriteByte(s_servo_ids[END_SERVO_ROTATE_INDEX], SMS_REG_MODE, SMS_MODE_SPEED);
    FeetechServo_WriteWord(s_servo_ids[END_SERVO_ROTATE_INDEX], SMS_REG_GOAL_SPEED, FeetechServo_EncodeSpeed(speed));
}

void FeetechServo_Stop123(void)
{
    FeetechServo_SetSpeed123(0, 0, 0);
}

void FeetechServo_StopAll(void)
{
    FeetechServo_Stop123();
    FeetechServo_SetRotateSpeed(0);
}
