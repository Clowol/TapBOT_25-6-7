#include "wit_c_sdk.h"

static CanWrite p_WitCanWriteFunc = NULL;
static RegUpdateCb p_WitRegUpdateCbFunc = NULL;
static DelaymsCb p_WitDelaymsFunc = NULL;

static uint8_t s_ucAddr = 0xff;
static uint32_t s_uiProtoclo = 0, s_uiReadRegIndex = 0;
int16_t sReg[REGSIZE];


#define FuncW 0x06
#define FuncR 0x03

////////////////////////////////////////////////////////////////////////////
static void CopeWitData(uint8_t ucIndex, uint16_t *p_data, uint32_t uiLen)
{
    uint32_t uiReg1 = 0, uiReg2 = 0, uiReg1Len = 0, uiReg2Len = 0;
    uint16_t *p_usReg1Val = p_data;
    uint16_t *p_usReg2Val = p_data+3;
    
    uiReg1Len = 4;
    switch(ucIndex)
    {
        case WIT_ACC:   uiReg1 = AX;    uiReg1Len = 3;  uiReg2 = TEMP;  uiReg2Len = 1;  break;
        case WIT_ANGLE: uiReg1 = Roll;  uiReg1Len = 3;  uiReg2 = VERSION;  uiReg2Len = 1;  break;
        case WIT_TIME:  uiReg1 = YYMM;	break;
        case WIT_GYRO:  uiReg1 = GX;  uiLen = 3;break;
        case WIT_MAGNETIC: uiReg1 = HX;  uiLen = 3;break;
        case WIT_DPORT: uiReg1 = D0Status;  break;
        case WIT_PRESS: uiReg1 = PressureL;  break;
        case WIT_GPS:   uiReg1 = LonL;  break;
        case WIT_VELOCITY: uiReg1 = GPSHeight;  break;
        case WIT_QUATER:    uiReg1 = q0;  break;
        case WIT_GSA:   uiReg1 = SVNUM;  break;
        case WIT_REGVALUE:  uiReg1 = s_uiReadRegIndex;  break;
		default:
			return ;

    }
    if(uiLen == 3)
    {
        uiReg1Len = 3;
        uiReg2Len = 0;
    }
    if(uiReg1Len)
	{
		memcpy(&sReg[uiReg1], p_usReg1Val, uiReg1Len<<1);
		p_WitRegUpdateCbFunc(uiReg1, uiReg1Len);
	}
    if(uiReg2Len)
	{
		memcpy(&sReg[uiReg2], p_usReg2Val, uiReg2Len<<1);
		p_WitRegUpdateCbFunc(uiReg2, uiReg2Len);
	}
}
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
int32_t WitCanWriteRegister(CanWrite Write_func)
{
    if(!Write_func)return WIT_HAL_INVAL;
    p_WitCanWriteFunc = Write_func;
    return WIT_HAL_OK;
}
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
void WitCanDataIn(uint8_t ucData[8], uint8_t ucLen)
{
	uint16_t usData[3];
    if(p_WitRegUpdateCbFunc == NULL)return ;
    if(ucLen < 8)return ;
    switch(s_uiProtoclo)
    {
        case WIT_PROTOCOL_CAN:
            if(ucData[0] != 0x55)return ;
            usData[0] = ((uint16_t)ucData[3] << 8) | ucData[2];
            usData[1] = ((uint16_t)ucData[5] << 8) | ucData[4];
            usData[2] = ((uint16_t)ucData[7] << 8) | ucData[6];
            CopeWitData(ucData[1], usData, 3);
            break;
    }
}
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
int32_t WitRegisterCallBack(RegUpdateCb update_func)
{
    if(!update_func)return WIT_HAL_INVAL;
    p_WitRegUpdateCbFunc = update_func;
    return WIT_HAL_OK;
}
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
int32_t WitWriteReg(uint32_t uiReg, uint16_t usData)
{
    uint8_t ucBuff[8];
    if(uiReg >= REGSIZE)return WIT_HAL_INVAL;
    switch(s_uiProtoclo)
    {

        case WIT_PROTOCOL_CAN:
            if(p_WitCanWriteFunc == NULL)return WIT_HAL_EMPTY;
            ucBuff[0] = 0xFF;
            ucBuff[1] = 0xAA;
            ucBuff[2] = uiReg & 0xFF;
            ucBuff[3] = usData & 0xff;
            ucBuff[4] = usData >> 8;
            p_WitCanWriteFunc(s_ucAddr, ucBuff, 5);
            break;
	default: 
            return WIT_HAL_INVAL;        
    }
    return WIT_HAL_OK;
}
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
int32_t WitReadReg(uint32_t uiReg, uint32_t uiReadNum)
{
    uint8_t ucBuff[8];
    if((uiReg + uiReadNum) >= REGSIZE)return WIT_HAL_INVAL;
    switch(s_uiProtoclo)
    {
        case WIT_PROTOCOL_CAN:
            if(uiReadNum > 3)return WIT_HAL_INVAL;
            if(p_WitCanWriteFunc == NULL)return WIT_HAL_EMPTY;
            ucBuff[0] = 0xFF;
            ucBuff[1] = 0xAA;
            ucBuff[2] = 0x27;
            ucBuff[3] = uiReg & 0xff;
            ucBuff[4] = uiReg >> 8;
            p_WitCanWriteFunc(s_ucAddr, ucBuff, 5);
            break;
		default: 
            return WIT_HAL_INVAL;
    }
    s_uiReadRegIndex = uiReg;

    return WIT_HAL_OK;
}
////////////////////////////////////////////////////////////////////////////

int32_t WitInit(uint32_t uiProtocol, uint8_t ucAddr)
{
	if(uiProtocol > WIT_PROTOCOL_I2C)return WIT_HAL_INVAL;
    s_uiProtoclo = uiProtocol;
    s_ucAddr = ucAddr;
    return WIT_HAL_OK;
}

int32_t WitDelayMsRegister(DelaymsCb delayms_func)
{
    if(!delayms_func)return WIT_HAL_INVAL;
    p_WitDelaymsFunc = delayms_func;
    return WIT_HAL_OK;
}

char CheckRange(short sTemp,short sMin,short sMax)
{
    if ((sTemp>=sMin)&&(sTemp<=sMax)) return 1;
    else return 0;
}
/*Acceleration calibration demo*/
int32_t WitStartAccCali(void)
{
/*
	First place the equipment horizontally, and then perform the following operations
*/
	if(WitWriteReg(KEY, KEY_UNLOCK) != WIT_HAL_OK)	    return  WIT_HAL_ERROR;// unlock reg
	if(s_uiProtoclo == WIT_PROTOCOL_MODBUS)	p_WitDelaymsFunc(20);
	else if(s_uiProtoclo == WIT_PROTOCOL_NORMAL) p_WitDelaymsFunc(1);
	else ;
	if(WitWriteReg(CALSW, CALGYROACC) != WIT_HAL_OK)	return  WIT_HAL_ERROR;
	return WIT_HAL_OK;
}
int32_t WitStopAccCali(void)
{
	if(WitWriteReg(CALSW, NORMAL) != WIT_HAL_OK)	return  WIT_HAL_ERROR;
	if(s_uiProtoclo == WIT_PROTOCOL_MODBUS)	p_WitDelaymsFunc(20);
	else if(s_uiProtoclo == WIT_PROTOCOL_NORMAL) p_WitDelaymsFunc(1);
	else ;
	if(WitWriteReg(SAVE, SAVE_PARAM) != WIT_HAL_OK)	return  WIT_HAL_ERROR;
	return WIT_HAL_OK;
}
/*Magnetic field calibration*/
int32_t WitStartMagCali(void)
{
	if(WitWriteReg(KEY, KEY_UNLOCK) != WIT_HAL_OK)	return  WIT_HAL_ERROR;
	if(s_uiProtoclo == WIT_PROTOCOL_MODBUS)	p_WitDelaymsFunc(20);
	else if(s_uiProtoclo == WIT_PROTOCOL_NORMAL) p_WitDelaymsFunc(1);
	else ;
	if(WitWriteReg(CALSW, CALMAGMM) != WIT_HAL_OK)	return  WIT_HAL_ERROR;
	return WIT_HAL_OK;
}
int32_t WitStopMagCali(void)
{
	if(WitWriteReg(KEY, KEY_UNLOCK) != WIT_HAL_OK)	return  WIT_HAL_ERROR;
	if(s_uiProtoclo == WIT_PROTOCOL_MODBUS)	p_WitDelaymsFunc(20);
	else if(s_uiProtoclo == WIT_PROTOCOL_NORMAL) p_WitDelaymsFunc(1);
	else ;
	if(WitWriteReg(CALSW, NORMAL) != WIT_HAL_OK)	return  WIT_HAL_ERROR;
	return WIT_HAL_OK;
}
/*change Band*/
int32_t WitSetUartBaud(int32_t uiBaudIndex)
{
	if(!CheckRange(uiBaudIndex,WIT_BAUD_4800,WIT_BAUD_230400))
	{
		return WIT_HAL_INVAL;
	}
	if(WitWriteReg(KEY, KEY_UNLOCK) != WIT_HAL_OK)	return  WIT_HAL_ERROR;
	if(s_uiProtoclo == WIT_PROTOCOL_MODBUS)	p_WitDelaymsFunc(20);
	else if(s_uiProtoclo == WIT_PROTOCOL_NORMAL) p_WitDelaymsFunc(1);
	else ;
	if(WitWriteReg(BAUD, uiBaudIndex) != WIT_HAL_OK)	return  WIT_HAL_ERROR;
	return WIT_HAL_OK;
}
/*change Can Band*/
int32_t WitSetCanBaud(int32_t uiBaudIndex)
{
	if(!CheckRange(uiBaudIndex,CAN_BAUD_1000000,CAN_BAUD_3000))
	{
		return WIT_HAL_INVAL;
	}
	if(WitWriteReg(KEY, KEY_UNLOCK) != WIT_HAL_OK)	return  WIT_HAL_ERROR;
	if(s_uiProtoclo == WIT_PROTOCOL_MODBUS)	p_WitDelaymsFunc(20);
	else if(s_uiProtoclo == WIT_PROTOCOL_NORMAL) p_WitDelaymsFunc(1);
	else ;
	if(WitWriteReg(BAUD, uiBaudIndex) != WIT_HAL_OK)	return  WIT_HAL_ERROR;
	return WIT_HAL_OK;
}
/*change Bandwidth*/
int32_t WitSetBandwidth(int32_t uiBaudWidth)
{	
	if(!CheckRange(uiBaudWidth,BANDWIDTH_256HZ,BANDWIDTH_5HZ))
	{
		return WIT_HAL_INVAL;
	}
	if(WitWriteReg(KEY, KEY_UNLOCK) != WIT_HAL_OK)	return  WIT_HAL_ERROR;
	if(s_uiProtoclo == WIT_PROTOCOL_MODBUS)	p_WitDelaymsFunc(20);
	else if(s_uiProtoclo == WIT_PROTOCOL_NORMAL) p_WitDelaymsFunc(1);
	else ;
	if(WitWriteReg(BANDWIDTH, uiBaudWidth) != WIT_HAL_OK)	return  WIT_HAL_ERROR;
	return WIT_HAL_OK;
}

/*change output rate */
int32_t WitSetOutputRate(int32_t uiRate)
{	
	if(!CheckRange(uiRate,RRATE_02HZ,RRATE_NONE))
	{
		return WIT_HAL_INVAL;
	}
	if(WitWriteReg(KEY, KEY_UNLOCK) != WIT_HAL_OK)	return  WIT_HAL_ERROR;
	if(s_uiProtoclo == WIT_PROTOCOL_MODBUS)	p_WitDelaymsFunc(20);
	else if(s_uiProtoclo == WIT_PROTOCOL_NORMAL) p_WitDelaymsFunc(1);
	else ;
	if(WitWriteReg(RRATE, uiRate) != WIT_HAL_OK)	return  WIT_HAL_ERROR;
	return WIT_HAL_OK;
}

/*change WitSetContent */
int32_t WitSetContent(int32_t uiRsw)
{	
	if(!CheckRange(uiRsw,RSW_TIME,RSW_MASK))
	{
		return WIT_HAL_INVAL;
	}
	if(WitWriteReg(KEY, KEY_UNLOCK) != WIT_HAL_OK)	return  WIT_HAL_ERROR;
	if(s_uiProtoclo == WIT_PROTOCOL_MODBUS)	p_WitDelaymsFunc(20);
	else if(s_uiProtoclo == WIT_PROTOCOL_NORMAL) p_WitDelaymsFunc(1);
	else ;
	if(WitWriteReg(RSW, uiRsw) != WIT_HAL_OK)	return  WIT_HAL_ERROR;
	return WIT_HAL_OK;
}



