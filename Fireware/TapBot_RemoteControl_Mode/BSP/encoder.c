/******************** (C) COPYRIGHT 2026 *****************************************
 * @file        encoder.c
 * @brief       Absolute encoder readout via CAN bus (CAN1 RX).
 * @note        
 * @warning     
 * @license     This project is released under the MIT License.
 *********************************************************************************/
#include "stm32f10x.h" 
#include "can1.h"
#include "encoder.h"
#include "app_config.h"



/**************************************************************************************/
//编码器CAN默认参数:标准帧,数据帧,默认波特率100K,默认地址0x01
//                 64圈 10bit分辨率（1024）
/**************************************************************************************/

/**************************************************************************************/
//                          指令集格式 
//        字节0   字节1    字节2    字节3    字节4    字节5
//        地址    高四位   参数1    参数2    参数3    参数4
//               参数个数
//                低四位
//                功能码

//                                     指令集
//u8 Encoder_Read_Round[2]        = {0x01, 0x00};                          //读取编码器当前圈数、单圈值
//u8 Encoder_Read_Pos_Speed[2]    = {0x01, 0x01};                          //读取编码器当前位置、转速
//u8 Encoder_Read_Pos_Dirc[2]     = {0x01, 0x09};                          //读取编码器位置、工作方向 编码器位置4个字节，32位无符号数，高字节在前；方向信息1个字节，取值范围1-2

//u8 Encoder_Set_Addr[3]          = {0x01, 0x12, 0x02};                    //设置编码器节点地址，取值范围01-FF
//u8 Encoder_Set_Bund[3]          = {0x01, 0x13, 0x04};                    //设置波特率，取值范围1-5。默认值为5 1:1Mhz\2:500K\3:250K\4:125K\5:100K
//u8 Encoder_Set_Mode[3]          = {0x01, 0x14, 0x02};                    //设置工作模式，取值范围1-2。默认值为1,1:查询模式 2:主动上传(当前位置和转速)
//u8 Encoder_Set_Resp[4]          = {0x01, 0x25, 0x03, 0xE8};              //设置主动上传周期，取值范围1-10000。单位：100us 默认值为10,即10*100=1000us
//u8 Encoder_Set_Zero[3]          = {0x01, 0x16, 0xFF};                    //编码器置零,以当前位置为新的零点。
//u8 Encoder_Set_Zero_Pos[3]      = {0x01, 0x17, 0x02};                    //设置编码器零点取值，取值范围1-2。默认值为1 此指令只对多圈编码器有效 1:中间值 2:0值
//u8 Encoder_Set_Dirc[3]          = {0x01, 0x18, 0x02};                    //设置编码器值递增方向，取值范围1-2。默认值为1 1:顺时针工作值递增2:逆时针工作值递增
//u8 Encoder_Set_Pos[6]           = {0x01, 0x4A, 0x00, 0x00, 0x00, 0x01};  //设置编码器当前位置(测量范围内可设任意值为当前值) 此指令只对多圈编码器有效 取值范围0-0xFFFFFFFF
//u8 Encoder_Set_Speed_Capture[3] = {0x01, 0x1B, 0x0A};                    //设置速度采样周期，取值范围01-C8。 单位：10ms 默认值为0A,即10*10=100ms
/**************************************************************************************/

/**************************************************************************************/
//初始化步骤(一次)：设置ID->设置波特率->设置自动上报模式->零点取0->当前位置置0->设置方向->设置速度采样周期->设置自动上报频率
// 0x01            12,01   13,04(125K)    14,02         17,02     16,FF      18,02     1B,01(10ms)     25,00,64(10ms)
//反馈数据放进数组，通过CAN1打印
//之后注释掉初始化函数
/**************************************************************************************/



/**************************** 全局变量定义 ****************************************/
extern u8 canrxbuf1[8];

u8 Encoder_Set_OneByte[3]  = {0x01, 0x00, 0x00};
u8 Encoder_Set_Resp[4]     = {0x01, 0x25, 0x00, 0x00};
u8 Encoder_Set_Pos[6]      = {0x01, 0x4A, 0x00, 0x00, 0x00, 0x01};
u16 RxRoundBuffer[2]    = {0x00, 0x00};                      

u8 Code[7][2] ={
    {0x12,0x01},         // 设置ID
    {0x13,0x05},         // 设置波特率100K
    {0x14,0x02},         // 设置自动上报模式
    {0x17,0x02},         // 零点取0（多圈编码器有效）
    {0x16,0xFF},         // 当前位置置0
    {0x18,0x02},         // 设置方向:逆时针递增
    {0x1B,0x01}          // 设置速度采样周期10ms
};

static encoder_feedback_t EncoderFeedback = {0, 0, 0U};

#define ENCODER_NODE_ADDR              0x01U
#define ENCODER_POS_SPEED_CODE         0x01U
#define ENCODER_POS_SPEED_PUSH_CODE    0x59U
#define ENCODER_S32_MAX                2147483647LL


/********************************************************************************
 * @name 	Encoder_Set_OneByte_Fun（u8 Code, u8 Obj）
 * @brief   编码器单字节指令设置函数
 * @param   code 功能码（高4位参数个数+低4位功能码）
 *          obj 指令参数（1字节
 * @retval  无
 * @note    CAN发送格式：地址(1)+功能码(1)+参数(1)，共3字节
 ********************************************************************************/
void Encoder_Set_OneByte_Fun(u8 Code, u8 Obj)
{
    Encoder_Set_OneByte[1] = Code;
    Encoder_Set_OneByte[2] = Obj;

    Can1_Send_Msg(Encoder_Set_OneByte,0x00,3);
    swgPrtUx(USART2, "\r\n transmit Ok \r\n");

#if 0 
    while(rxflag)
    {
        if(canrxbuf1[2] == 0x00)
        return;
    }
#endif
}

/********************************************************************************
 * @name 	Encoder_Set_Resp_Fun(u16 RespCode)
 * @brief   设置编码器上传周期
 * @param   resp_code 上传周期（0001-2710，单位：100us）
 * @retval  无
 * @note    CAN发送格式：地址(1)+功能码(1)+参数高8位(1)+参数低8位(1)，共4字节
 ********************************************************************************/
void Encoder_Set_Resp_Fun(u16 RespCode)
{
    Encoder_Set_Resp[2] = (u8)(RespCode >> 8);
    Encoder_Set_Resp[3] = (u8)RespCode;

    Can1_Send_Msg(Encoder_Set_Resp,0x00,4);
    swgPrtUx(USART2, "\r\n transmit Ok \r\n");

#if 0 
    while(rxflag)
    {
        if(canrxbuf1[2] == 0x00)
        return;
    }
#endif
}

/********************************************************************************
 * @name 	Encoder_Set_Pos_Fun(u32 PosCode)
 * @brief   设置编码器当前位置
 * @param   pos_code 当前位置值（32位）
 * @retval  无
 * @note    CAN发送格式：地址(1)+功能码(1)+参数(4)，共6字节
 ********************************************************************************/
void Encoder_Set_Pos_Fun(u32 PosCode)
{
    u32 timeout = ENCODER_CAN_WAIT_TIMEOUT;

    Encoder_Set_Pos[2] = (u8)(PosCode >> 24);
    Encoder_Set_Pos[3] = (u8)(PosCode >> 16);
    Encoder_Set_Pos[4] = (u8)(PosCode >> 8);
    Encoder_Set_Pos[5] = (u8)PosCode;
    canrxbuf1[2] = 0xFFU;
    Can1_Send_Msg(Encoder_Set_Pos,0x00,6);

    while((canrxbuf1[2] != 0x00U) && (timeout > 0U))
    {
        timeout--;
    }
}

/********************************************************************************
 * @name 	Encoder_Read_Round_Fun(void)
 * @brief   读取编码器当前圈数、单圈值
 * @param   无
 * @retval  无
 * @note    发送读取指令后，从CAN接收缓冲区解析圈数（2字节）和单圈值（2字节）
 *              圈数2个字节，16 位无符号数，高字节在前  (单圈编码器固定为0x00 0x00)；
 *              单圈值2个字节，16 位无符号数，高字节在前；
 ********************************************************************************/
void Encoder_Read_Round_Fun(void)
{
    u8 Encoder_Read_Round[2] = {0x01, 0x00};
    Can1_Send_Msg(Encoder_Read_Round,0x00,2);
    
    for(u8 i =0; i < 2; i++)
    {
        RxRoundBuffer[i] = (u16)((u16)canrxbuf1[2*i+2]<<8 | (u16)canrxbuf1[2*i+3]);
    }
}

/********************************************************************************
 * @name 	Encoder_Read_Pos_Dirc_Fun(void)
 * @brief   读取编码器当前位置、方向信息
 * @param   无
 * @retval  无
 * @note    发送读取指令后，从CAN接收缓冲区解析当前位置（4字节）和方向信息（1字节）
 ********************************************************************************/
void Encoder_Read_Pos_Dirc_Fun(void)
{
    u8 Encoder_Read_Pos_Dirc[2] = {0x01, 0x09};
    Can1_Send_Msg(Encoder_Read_Pos_Dirc,0x00,2);
    USART2_DMA_send(canrxbuf1,7);
}

/********************************************************************************
 * @name 	Solve_Length_and_Linespeed(u8 DataArray[], u8 *Actual_Length, u8 *Actual_Speed)
 * @brief   解析编码器数据，计算带簧实际长度和线速度
 * @param   data_array CAN接收数据数组（需包含位置(4字节)+转速(2字节)）
 *          actual_length 输出：实际长度（mm，u8指针，非空）
 *          actual_speed 输出：实际线速度（mm/s，u8指针，非空）
 * @retval  无
 * @note    换算公式：长度=位置/分辨率×滚轮周长；速度=转速/换算系数
 ********************************************************************************/
#if 0
/* Deprecated unused API. Use Encoder_UpdateFromCanFrame() and Encoder_GetFeedback(). */
void Solve_Length_and_Linespeed(u8 DataArray[], u8 *Actual_Length, u8 *Actual_Speed)
{
    u32 temp_Length = ((u32)(DataArray[2]<<24)|(u32)(DataArray[3]<<16)|(u32)(DataArray[4]<<8)|(u32)(DataArray[5]));
    u16 temp_speed = (u16)(DataArray[6]<<8 | DataArray[7]);
    
    *Actual_Length = (u8)ceil(temp_Length / 1024 * Roller_Perimeter);       //unit:mm
    *Actual_Speed = (u8)ceil(temp_speed / 3);                               //unit:mm/s
    EncoderFeedback.length_mm = (s32)(*Actual_Length);
    EncoderFeedback.line_speed_mm_s = (s16)(*Actual_Speed);
    EncoderFeedback.valid = 1U;
}
#endif

void Encoder_UpdateFromCanFrame(const u8 *data, u8 len)
{
    u32 raw_pos;
    u16 raw_speed;
    unsigned long long length_scaled;

    if((data == 0) || (len < 8U))
    {
        return;
    }
    if((data[0] != ENCODER_NODE_ADDR) ||
        ((data[1] != ENCODER_POS_SPEED_CODE) && (data[1] != ENCODER_POS_SPEED_PUSH_CODE)))
    {
        return;
    }

    raw_pos = (((u32)data[2]) << 24) |
                (((u32)data[3]) << 16) |
                (((u32)data[4]) << 8) |
                ((u32)data[5]);
    raw_speed = (u16)((((u16)data[6]) << 8) | (u16)data[7]);

    length_scaled = (unsigned long long)raw_pos * (unsigned long long)Roller_Perimeter;
    length_scaled = length_scaled / 1024ULL;
    if(length_scaled > (unsigned long long)ENCODER_S32_MAX)
    {
        EncoderFeedback.length_mm = (s32)ENCODER_S32_MAX;
    }
    else
    {
        EncoderFeedback.length_mm = (s32)length_scaled;
    }
    EncoderFeedback.line_speed_mm_s = (s16)(raw_speed / 3U);
    EncoderFeedback.valid = 1U;
}

/********************************************************************************
 * 函数名：Length_Control
 * 描述  ：读取带簧当前伸长长度，进行软件限位
 * 输入  ：伸长长度
 * 输出  ：关节电机开关量
 * 
 * 调用  ：
 ******************************************************************************/
#if 0
/* Deprecated unused API. Compare Encoder_GetFeedback()->length_mm directly. */
u8 Length_Control(u8 *Length)
{
    if((*Length - Length_Max) == 0)
        return ON;
    else
        return OFF;
}
#endif

/********************************************************************************
 * @name 	Encoder_Init(void)
 * @brief   编码器初始函数
 * @param   无
 * @retval  无
 * @note    初始化步骤(一次)：设置ID->设置波特率->设置自动上报模式->零点取0->当前位置置0->设置方向->设置速度采样周期->设置自动上报频率
 *              0x01            12,01   13,04(125K)    14,02         17,02     16,FF      18,02     1B,01(10ms)     25,00,64(10ms)
 *          串口2调试信息,接收一次 8个00，0x01 0x59， （0x00 0x06 0x03 0x27）（未知） 0x01or02 共15个字节有效数据
 ********************************************************************************/
void Encoder_Init(void)              
{
    static u8 i,j,k;
    
    for(i=0;i<7;i++)
    {
        for(j=0,k=1;j<2 && k<3;j++,k++)
        {
            Encoder_Set_OneByte[k] = Code[i][j];
        }
        Can1_Send_Msg(Encoder_Set_OneByte,0x00,3); 
    }

    Encoder_Set_Resp_Fun(0x64);         // 最后一步 设置广播频率
    Encoder_Read_Pos_Dirc_Fun();        // 读取当前位置、方向

}

const encoder_feedback_t *Encoder_GetFeedback(void)
{
    return &EncoderFeedback;
}



/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/
