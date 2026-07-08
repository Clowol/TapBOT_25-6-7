/******************** (C) COPYRIGHT 2026 *****************************************
 * @file        sys.c
 * @brief       NVIC interrupt grouping, debug print, and CRC utility functions.
 * @note        
 * @warning     
 * @license     This project is released under the MIT License.
 *********************************************************************************/
#include "sys.h"
#include "app_config.h"

char str[100]; // 全局格式化缓冲区


/*=====================================================
  * @name	swgPtb5(char c)  
  * @brief  向默认串口发送单个字符
  * @param  -c  要发送的字符	
  * @note   BIT7	发送完成标志位TC
  *===================================================*/
void swgPtb5(char c) 					
{
	#if 1
	UART5->DR = c;
	while(!(UART5->SR & BIT7));		
	#else
	USART1->DR = c;
	while(!(USART1->SR & BIT7));
	#endif
}

/*=====================================================
  * @name	swgPtbUx(USART_TypeDef* USARTx, char c) 
  * @brief  向指定串口发送单个字符
  * @param  -USARTx  要发送的串口指针
  *         -c  要发送的字符	
  * @note   BIT7	发送完成标志位TC
  *===================================================*/
void swgPtbUx(USART_TypeDef* USARTx, char c) 					
{
	u32 timeout = SYS_UART_TX_TIMEOUT;

	USARTx->DR = c;
	while((!(USARTx->SR & BIT7)) && (timeout > 0U))
	{
		timeout--;
	}
}



/*=====================================================
  * @name	swgPtn(char *str)
  * @brief  向默认串口发送字符串
  * @param  -str  要发送的字符串	
  * @note   Windows 终端通常需要\r\n组合才能正确换行
  *===================================================*/
void swgPtn(char *str)
{
	char *p = str;
	
	while(*p)
	{
		if(*p=='\n')
			swgPtb5('\r');
		swgPtb5(*p);
		p++;
	}
}

/*=====================================================
  * @name	swgPtnUx(USART_TypeDef* USARTx, char *str)
  * @brief  向指定串口发送字符串
  * @param  -USARTx  要发送的串口指针
  *         -str  要发送的字符串	
  * @note   Windows 终端通常需要\r\n组合才能正确换行
  *===================================================*/
void swgPtnUx(USART_TypeDef* USARTx, char *str)
{
	char *p = str;
	
	while(*p)
	{
		if(*p=='\n')
			swgPtbUx(USARTx, '\r');
		swgPtbUx(USARTx, *p);
		p++;
	}
}



/*=====================================================
  * @name	swgPrtData(u8 *ary, INT8U len)
  * @brief  串口1打印数组
  * @param  -ary  要发送的数组指针
  *         -len  数组长度
  * @note   %02X 十六进制
  *===================================================*/
void swgPrtData(u8 *ary, INT8U len)
{
	INT8U i = 0;
	for(i=0;i<len;i++){
		swgPrt("%02X ",ary[i]);
	}
	swgPrt("\n");
}

/*=====================================================
  * @name	swgPrtData(u8 *ary, INT8U len)
  * @brief  指定串口发送字节数组
  * @param  -ary  要发送的数组指针
  *         -len  数组长度
  * @note   %02X 十六进制
  *===================================================*/
void swgPrtDataUx(USART_TypeDef* USARTx, u8 *ary, INT8U len)
{
	INT8U i = 0;
	for(i=0 ;i<len; i++){
		swgPrtUx(USARTx,"%02X ",ary[i]);
	}
	swgPrtUx(USARTx,"\n");
}




/*=====================================================
  * @name	HBT(u16 a)
  * @brief  取数据的高8位值
  * @param  -a  要取值的16位数据
  * @note   
  *===================================================*/
u8 HBT(u16 a)	 
{
	a &= 0xFF00;
	a >>= 8;
	return a;
}


/*=====================================================
  * @name	LBT(u16 b)
  * @brief  取数据的低8位值
  * @param  -b  要取值的16位数据
  * @note   
  *===================================================*/
u8 LBT(u16 b)	
{
	b &= 0x00FF;
	return b;
}


/*=====================================================
  * @name	CheckCrc_16bit(unsigned char *buf, unsigned int len)
  * @brief  16位CRC校验
  * @param  -buf  要校验的缓冲区指针
  *         -len  缓冲区长度
  * @note   
  *===================================================*/
uint CheckCrc_16bit(uchar *buf, uint len) 
{
    int i,j;
    uint c, crc = 0xFFFF;
    for (i = 0; i < len; i++){
        c = *(buf+i) & 0x00FF;
        crc ^= c;
            for(j=0 ; j<8 ; j++){
                if (crc & 0x0001)
                    {crc >>= 1; crc ^= 0xA001;}
                else crc >>= 1;
            }
    }
    return(crc);	
}

// 8 位 CRC 校验
uint CheckCrc_8bit(uchar *buf, uint len)
{
	uchar crc=0, i, j;
	for(i=0; i<len; i++)
	{
		crc ^= *(buf+i);
		for(j=0; j<8; j++)
		{				
			if(crc & 0x01) 
				crc = (crc>>1)^0x8C;
			else 
				crc >>= 1;
		}
	}
	return crc;
} 



/*=====================================================
  * @name	NVIC_Configuration(void)
  * @brief  NVIC中断优先级分组配置->2位抢占优先级（0-3），2位响应优先级（0-3）
  * @param  无
  * @note   数字越小，中断优先级越高
  * @note   必须在使能任何中断之前调用，且整个程序只能调用一次
  *===================================================*/
void NVIC_Configuration(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	 	//设置NVIC中断分组2:
}


/******************* (C) COPYRIGHT 2026 END OF FILE *****************************/
