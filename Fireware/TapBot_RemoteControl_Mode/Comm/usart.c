/******************** (C) COPYRIGHT 2026 *****************************************
 * @file        usart.c
 * @brief       USART1-5 initialization, DMA configuration, and interrupt handlers.
 * @note        
 * @warning     
 * @license     This project is released under the MIT License.
 *********************************************************************************/
#include "usart.h"
#include "subboard_link.h"


/********************************* 全局变量定义**************************************/
/**************************************************************************************************
 * @note 	每个串口接收缓冲区及状态变量
 * @param	-RX_BUF：接收缓冲区
 * 			-RX_STA：接收状态标志
 * 			-LEN：接收数据长度
 ***************************************************************************************************/
//串口1变量定义  	DMA
INT8U 	USART1_DMA_Tx_Buf[USART1_DMA_TX_BSIZE] = {0};	// USART1 发送数组
char 	USART1_DMA_Rece_Buf[USART1_MAX_RECV_LEN]; 		// USART1 接收数组

char 	USART1_RX_BUF[USART1_MAX_RECV_LEN]; 			// 接收缓冲,最大USART1_MAX_RECV_LEN个字节.
u16 	USART1_RX_STA = 0;                         		// 串口1标志位
int 	LEN1 = 0;										// 串口1数据长度
												

//串口2变量定义  	DMA
INT8U 	USART2_DMA_Tx_Buf[USART2_DMA_TX_BSIZE] = {0};	// USART2 发送数组
char 	USART2_DMA_Rece_Buf[USART2_MAX_RECV_LEN]; 		// USART2 接收数组

char 	USART2_RX_BUF[USART2_MAX_RECV_LEN]; 			// 接收缓冲,最大USART2_MAX_RECV_LEN个字节.
u16 	USART2_RX_STA = 0;                         		// 串口2标志位
int 	LEN2 = 0;										// 串口2数据长度

 
//串口3变量定义		DMA
INT8U 	USART3_DMA_Tx_Buf[USART3_DMA_TX_BSIZE]={0};	 	// USART3 发送数组
char 	USART3_DMA_Rece_Buf[USART3_MAX_RECV_LEN];		// USART3 接收数组

char 	USART3_RX_BUF[USART3_MAX_RECV_LEN]; 			// 接收缓冲,最大USART3_MAX_RECV_LEN个字节.
u16 	USART3_RX_STA = 0;                             	// 串口3标志位
int 	LEN3 = 0;										// 串口3数据长度


//串口4变量定义		DMA
INT8U 	UART4_DMA_Tx_Buf[UART4_DMA_TX_BSIZE]={0};	   	// USART4 发送数组
char 	UART4_DMA_Rece_Buf[USART4_MAX_RECV_LEN];     	// USART4 接收数组

char 	UART4_RX_BUF[USART4_MAX_RECV_LEN]; 				// 接收缓冲,最大USART4_MAX_RECV_LEN个字节.
u16 	USART4_RX_STA = 0;                             	// 串口4标志位
int 	LEN4 = 0;										// 串口4数据长度


//串口5变量定义
char 	USART5_RX_BUF[USART5_MAX_RECV_LEN];           	// 串口5接收缓存数组(最大USART1_MAX_RECV_LEN个字节)
u16 	USART5_RX_STA = 0;                             	// 串口5标志位
int 	LEN5 = 0;										// 串口5数据长度



/********************************** 串口1 *****************************************/
/**
 * @brief   串口1初始化函数
 * @param   CNDTR：所剩余待传输的数据数量；
 * @param   NDTR：初始化时设定的总传输数据量
 * @retval  None 
 * @note    注意USART1是APB2时钟树！！
 */
void USART1_Init(u32 bound)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;
	USART_InitTypeDef 	USART_InitStructure;
	NVIC_InitTypeDef 	NVIC_InitStructure;
	DMA_InitTypeDef 	DMA_InitStructure;
	
	// 1. 时钟使能（串口+GPIO+DMA）
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	
	
	USART_DeInit(USART1);	// 复位USART1

    // 2. GPIO配置
    // USART1_RX (PA10) -> push-pull    
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// USART1_TX (PA9)	-> push-pull
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
    // 3. 中断配置（开启空闲中断，禁用RXNE中断）
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_Init(&NVIC_InitStructure);
	
	// 4. USART1 核心参数配置
	USART_InitStructure.USART_BaudRate = bound;					// 波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b; // 数据长度 8个字节
	USART_InitStructure.USART_StopBits = USART_StopBits_1;		// 1位停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;			// 无奇偶校验
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);					

	// 清除标志位，以防以外判断有中断请求
	USART_ClearITPendingBit(USART1, USART_IT_RXNE); 				// 清除"接收寄存器非空"中断标志
	USART_ClearITPendingBit(USART1, USART_IT_TXE);					// 清除"发送寄存器为空"中断标志
	USART_ClearITPendingBit(USART1, USART_IT_TC);					// 清除"发送完成"中断标志	

	USART_ITConfig(USART1, USART_IT_TC,   DISABLE);					// 禁用"发送完成"中断
	USART_ITConfig(USART1, USART_IT_RXNE, DISABLE); 				// 禁用"接收寄存器非空"中断
	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);	   				// 开启空闲中断
	
	USART_DMACmd(USART1, USART_DMAReq_Tx|USART_DMAReq_Rx, ENABLE);	// 使能USART1 DMA发送和接收
	USART_Cmd(USART1, ENABLE);									    // 使能串口

	// 5. DMA配置
	// Tx DMA:	USART1 TX 对应DAM1通道4
	DMA_Cmd(DMA1_Channel4,DISABLE); 									// close DMA Channel
	DMA_DeInit(DMA1_Channel4);

	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART1->DR;   		// 外设地址：USART1得数据寄存器	
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)USART1_DMA_Tx_Buf;	 	// 内存地址：要传输的变量的指针 
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;	 				// 方向：发送	从内存->外设 			
	DMA_InitStructure.DMA_BufferSize = 0;								// 初始发送长度为0（避免误发送） 	
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;    // 外设地址不增		
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;   			// 内存地址自增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; 	// 外设数据单位		
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;   	// 内存数据单位 	
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal	;					// DMA模式，一次或者循环模式		
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;	 			// 优先级：中 	
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;						// 禁止内存到内存的传输	   
	
	DMA_Init(DMA1_Channel4, &DMA_InitStructure);		// 配置DMA通道		
	DMA_Cmd (DMA1_Channel4, DISABLE);					// 初始：禁止 -> 否则使能后就开始发送了！
	
	// Rx DMA:   USART1 RX 对应DAM通道5
	DMA_DeInit(DMA1_Channel5);	 						// 串口1对应DMA1通道5
	
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART1->DR; 		// 外设地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)USART1_DMA_Rece_Buf;    // 内存地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;					// 方向：接受 从外设->内存
	DMA_InitStructure.DMA_BufferSize = USART1_MAX_RECV_LEN;				// 接收缓冲区大小（最大接收长度）
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; 	// 外设地址寄存器不变
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  			// 内存地址寄存器递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  // 外设数据字长
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;  		 // 内存数据字长
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  						// 工作在正常缓冲模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; 				// DMA通道x有中优先级 
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  						// DMA通道x没有设置内存到内存传输
	
	DMA_Init(DMA1_Channel5, &DMA_InitStructure);  		// 配置DMA通道
	DMA_Cmd(DMA1_Channel5, ENABLE); 					// 初始使能，开始等待接收数据

	USART1_RX_STA=0;
}


void USART1_DMA_send(INT8U* buffer, u8 size)
{
    if((buffer == 0) || (size == 0U) || (size > USART1_DMA_TX_BSIZE))
    {
        return;
    }

    memset(USART1_DMA_Tx_Buf, 0, USART1_DMA_TX_BSIZE);
	memcpy(USART1_DMA_Tx_Buf, buffer,size);
	DMA_Cmd(DMA1_Channel4, DISABLE);		// 禁用DMA （*）
	DMA1_Channel4->CNDTR = size;			// 设置发送长度
	DMA_Cmd(DMA1_Channel4, ENABLE);  		// 启动DMA发送
}


void USART1_IRQHandler(void)
{	
	INT8U temp = temp;
	if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET) //接收中断
	{
		// 读取SR+DR， 清除空闲中断标志
		temp = (USART1->SR);							 
		temp = (USART1->DR);	

		DMA_Cmd(DMA1_Channel5, DISABLE);

		// 计算本帧数据长度
		LEN1 = USART1_MAX_RECV_LEN - DMA_GetCurrDataCounter(DMA1_Channel5); 
		memcpy(USART1_RX_BUF, USART1_DMA_Rece_Buf, USART1_MAX_RECV_LEN);
			
		DMA1_Channel5->CNDTR = USART1_MAX_RECV_LEN;		// 重置DMA接收长度
		DMA_Cmd(DMA1_Channel5,ENABLE);					// 重启DMA接收

		USART1_RX_STA = 1;								// 标记接收完成
	
		/*******************test***********************/
#if 0	
		swgPrt ("USART1_DMA_Rece_Cnt:%d\r\n",USART1_DMA_Rece_Cnt);
		prtData(USART1_RX_BUF,USART1_DMA_Rece_Cnt);
		/*********************************************/
#endif		
			
		USART_ClearITPendingBit(USART1,USART_IT_IDLE);		   //清除中断标志 	
	}
}






/********************************** 串口2 *****************************************/
/**
 * @name    USART2
 * @brief   串口2 初始化函数 
 * @retval  none
 * @note    
 */
void USART2_Init(u32 bound)
{
	GPIO_InitTypeDef    GPIO_InitStructure;
	USART_InitTypeDef   USART_InitStructure;
	NVIC_InitTypeDef    NVIC_InitStructure;
	DMA_InitTypeDef     DMA_InitStructure;
	
	// 1. 时钟使能（串口+GPIO+DMA）
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD|RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);		
	
	USART_DeInit(USART2);	//复位USART2
	

    // 2. GPIO配置
    /* Configure USART2 Rx as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

    /* Configure USART2 Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	//#define GPIO_Remap_USART2    GPIO_Remap_USART2 
    GPIO_PinRemapConfig(GPIO_Remap_USART2, ENABLE); 


    // 3. 中断配置（开启空闲中断，禁用RXNE中断）
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel7_IRQn;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel6_IRQn;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_Init(&NVIC_InitStructure);
	

	// 4. USART2 核心参数配置
	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);	

	// 清除标志位，以防以外判断有中断请求
	USART_ClearITPendingBit(USART2, USART_IT_RXNE); 			
	USART_ClearITPendingBit(USART2, USART_IT_TXE);				
	USART_ClearITPendingBit(USART2, USART_IT_TC);				

	USART_ITConfig(USART2, USART_IT_TC, DISABLE);				
	USART_ITConfig(USART2, USART_IT_RXNE, DISABLE); 		
	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);	  

	USART_DMACmd(USART2,USART_DMAReq_Tx|USART_DMAReq_Rx,ENABLE);	
	USART_Cmd(USART2, ENABLE);		
	
    // 5. DMA配置
	//Tx DMA CONFIG	USART2 TX 对应DAM通道7
	DMA_Cmd(DMA1_Channel7,DISABLE); 									//close DMA Channel
	DMA_DeInit(DMA1_Channel7);

	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART2->DR;  
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)USART2_DMA_Tx_Buf;	 
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;	 		        // 方向：发送	从内存->外设
	DMA_InitStructure.DMA_BufferSize = 0;		                        // 初始发送长度为0（避免误发送）
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;   
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; 
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;  	
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal	;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;	
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable; 
	
	DMA_Init(DMA1_Channel7, &DMA_InitStructure);		// 配置DMA通道		
	DMA_Cmd (DMA1_Channel7,DISABLE);	                // 初始：禁止 -> 否则使能后就开始发送了！
	
	//Rx DMA CONFIG   USART2 RX 对应DAM通道6
	DMA_DeInit(DMA1_Channel6);	                        // 串口2对应DMA1通道6
	
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART2->DR; 
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)USART2_DMA_Rece_Buf; 
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;	                // 方向：接受 从外设->内存
	DMA_InitStructure.DMA_BufferSize = USART2_MAX_RECV_LEN;	            // 接收缓冲区大小（最大接收长度）
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; 
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; 
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; 
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

	DMA_Init(DMA1_Channel6, &DMA_InitStructure);  
	DMA_ITConfig(DMA1_Channel6, DMA_IT_TC, ENABLE);
	
	uart_rx_driver_init(UART_RX_PORT_USART2);

	DMA_Cmd(DMA1_Channel6, ENABLE);                     // 初始使能，开始等待接收数据

    USART2_RX_STA=0;    
}


void USART2_DMA_send(INT8U* buffer, u8 size)
{
	if((buffer == 0) || (size == 0U) || (size > USART2_DMA_TX_BSIZE))
	{
		return;
	}

	memcpy(USART2_DMA_Tx_Buf, buffer,size);
	DMA_Cmd(DMA1_Channel7, DISABLE);
	DMA1_Channel7->CNDTR = size;
	DMA_Cmd(DMA1_Channel7, ENABLE); 
}


void USART2_IRQHandler(void)
{	
	INT8U temp = temp;
	if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET) //接收中断
	{
		temp = (USART2->SR);
		temp = (USART2->DR);	
		DMA_Cmd(DMA1_Channel6,DISABLE);
		DMA_ClearITPendingBit(DMA1_IT_TC6);
		
        // 计算本帧数据长度
		LEN2 =USART2_MAX_RECV_LEN-DMA_GetCurrDataCounter(DMA1_Channel6); 
		if(LEN2 > 0)
		{
			uart_rx_driver_put(UART_RX_PORT_USART2, (uint8_t *)USART2_DMA_Rece_Buf, (uint32_t)LEN2);
		}
			
		DMA1_Channel6->CNDTR = USART2_MAX_RECV_LEN;     // 重置DMA接收长度
		DMA_Cmd(DMA1_Channel6,ENABLE);                  // 重启DMA接收
			
		USART2_RX_STA=1;
	
		USART_ClearITPendingBit(USART2,USART_IT_IDLE);		   //清除中断标志 	
		/*******************test***********************/
#if 0	
		swgPrt ("UP:%d\r\n",LEN2);
//		prtData(USART2_RX_BUF,LEN2);
		/*********************************************/
#endif		
			
		}
}




void DMA1_Channel6_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA1_IT_TC6) != RESET)
	{
		DMA_ClearITPendingBit(DMA1_IT_TC6);
		DMA_Cmd(DMA1_Channel6, DISABLE);

		LEN2 = USART2_MAX_RECV_LEN;
		uart_rx_driver_put(UART_RX_PORT_USART2, (uint8_t *)USART2_DMA_Rece_Buf, (uint32_t)LEN2);

		DMA1_Channel6->CNDTR = USART2_MAX_RECV_LEN;
		DMA_Cmd(DMA1_Channel6, ENABLE);
	}
}


/********************************** 串口3 *****************************************/
/**
 * @name    USART3
 * @brief   串口3 初始化函数
 * @param   
 * @retval  None 
 */
void USART3_Init(u32 bound)
{
	GPIO_InitTypeDef    GPIO_InitStructure;
	USART_InitTypeDef   USART_InitStructure;
	NVIC_InitTypeDef    NVIC_InitStructure;
	DMA_InitTypeDef     DMA_InitStructure;
	
	// 1. 时钟使能（串口+GPIO+DMA）
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	
	
	USART_DeInit(USART3);	//复位USART3

    // 2. GPIO配置
    // USART3_TX	 PB.10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure); 

	//USART3_RX  PB.11
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure); 

    // 3. 中断配置（开启空闲中断，禁用RXNE中断）
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel2_IRQn;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel3_IRQn;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_Init(&NVIC_InitStructure);


	// 4. USART3 核心参数配置
	USART_InitStructure.USART_BaudRate = bound; 				  // 波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;   // 数据长度 8个字节
	USART_InitStructure.USART_StopBits = USART_StopBits_1;		  // 1位停止位
	USART_InitStructure.USART_Parity = USART_Parity_No; 		  // 无奇偶校验
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure);					
	
    // 清除标志位，以防以外判断有中断请求
	USART_ClearITPendingBit(USART3, USART_IT_RXNE); 			// 清除"接收寄存器非空"中断标志
	USART_ClearITPendingBit(USART3, USART_IT_TXE);				// 清除"发送寄存器为空"中断标志
	USART_ClearITPendingBit(USART3, USART_IT_TC);				// 清除"发送完成"中断标志

	USART_ITConfig(USART3, USART_IT_TC, DISABLE);				// 禁用"发送完成"中断
	USART_ITConfig(USART3, USART_IT_RXNE, DISABLE); 			// 禁用"接收寄存器非空"中断
	USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);	            // 开启空闲中断

	USART_DMACmd(USART3,USART_DMAReq_Tx|USART_DMAReq_Rx,ENABLE);	//使能USART3 DMA发送和接收
	USART_Cmd(USART3, ENABLE);		                                //使能串口

    // 5. DMA配置
    // Tx DMA CONFIG - USART3 TX 对应DAM通道2
	DMA_Cmd(DMA1_Channel2,DISABLE); 									//close DMA Channel
	DMA_DeInit(DMA1_Channel2);

	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART3->DR;   
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)USART3_DMA_Tx_Buf;	
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;		
	DMA_InitStructure.DMA_BufferSize = 0;	
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;   
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; 	
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; 	
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;	
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;   
	
	DMA_Init(DMA1_Channel2, &DMA_InitStructure);	// 配置DMA通道		
	DMA_Cmd (DMA1_Channel2,DISABLE);	            // 初始：禁止 -> 否则使能后就开始发送了！
	

	//Rx DMA CONFIG   USART3 RX 对应DAM通道3
	DMA_DeInit(DMA1_Channel3);	 //串口3对应DMA通道3
	
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART3->DR; 
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)USART3_DMA_Rece_Buf; 
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = USART3_MAX_RECV_LEN;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; 
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; 
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; 
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; 
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  

	DMA_Init(DMA1_Channel3, &DMA_InitStructure);  
	DMA_ITConfig(DMA1_Channel3, DMA_IT_TC, ENABLE);
	
	uart_rx_driver_init(UART_RX_PORT_USART3);
	DMA_Cmd(DMA1_Channel3, ENABLE);          // 初始使能，开始等待接收数据

    USART3_RX_STA=0;

} 



void USART3_DMA_send(INT8U* buffer, u8 size)
	{
		if((buffer == 0) || (size == 0U) || (size > USART3_DMA_TX_BSIZE))
		{
			return;
		}

		memcpy(USART3_DMA_Tx_Buf, buffer,size);
		DMA_Cmd(DMA1_Channel2, DISABLE);        // 禁用DMA （*）
		DMA1_Channel2->CNDTR = size;            // 设置发送长度
		DMA_Cmd(DMA1_Channel2, ENABLE);         // 启动DMA发送
	}

void USART3_IRQHandler(void)
{	
	INT8U temp = temp;
//	INT8U i=0;
	if(USART_GetITStatus(USART3, USART_IT_IDLE) != RESET) //接收中断
	{
        // 读取SR+DR， 清除空闲中断标志
		temp = (USART3->SR);
		temp = (USART3->DR);	

		DMA_Cmd(DMA1_Channel3,DISABLE);
		DMA_ClearITPendingBit(DMA1_IT_TC3);

        // 计算本帧数据长度
		LEN3 =USART3_MAX_RECV_LEN-DMA_GetCurrDataCounter(DMA1_Channel3); 
		if(LEN3 > 0)
		{
			uart_rx_driver_put(UART_RX_PORT_USART3, (uint8_t *)USART3_DMA_Rece_Buf, (uint32_t)LEN3);
		}
		
		DMA1_Channel3->CNDTR = USART3_MAX_RECV_LEN;
		DMA_Cmd(DMA1_Channel3,ENABLE);

	
			/*******************test***********************/
	#if 0
			swgPrt ("USART3_DMA_Rece_Cnt:%d\r\n",USART3_DMA_Rece_Cnt);
			prtData(USART3_DMA_Rece_Buf,USART3_DMA_Rece_Cnt);
	#endif
			/*********************************************/
			
		USART_ClearITPendingBit(USART3,USART_IT_IDLE);		   //清除中断标志 	
	}
}

void DMA1_Channel3_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA1_IT_TC3) != RESET)
	{
		DMA_ClearITPendingBit(DMA1_IT_TC3);
		DMA_Cmd(DMA1_Channel3, DISABLE);

		LEN3 = USART3_MAX_RECV_LEN;
		uart_rx_driver_put(UART_RX_PORT_USART3, (uint8_t *)USART3_DMA_Rece_Buf, (uint32_t)LEN3);

		DMA1_Channel3->CNDTR = USART3_MAX_RECV_LEN;
		DMA_Cmd(DMA1_Channel3, ENABLE);
	}
}




/********************************** 串口4 *****************************************/
/** 
 * @name    UART4
 * @brief   串口4 初始化函数 
 * @retval  none
 * @note    
 */
void USART4_Init(u32 bound)
{
	GPIO_InitTypeDef    GPIO_InitStructure;
	USART_InitTypeDef   USART_InitStructure;
	NVIC_InitTypeDef    NVIC_InitStructure;
	DMA_InitTypeDef     DMA_InitStructure;
	
    // 1. 时钟使能（串口+GPIO+DMA）
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4,ENABLE);	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
	
	USART_DeInit(UART4);   //复位UART4


	// 2. GPIO配置
    /* Configure USART4 Rx as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* Configure USART4 Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure); 

	// 3. 中断配置（开启空闲中断，禁用RXNE中断）
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Channel4_IRQn;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;;
	NVIC_Init(&NVIC_InitStructure);


	// 4. USART4 核心参数配置
	USART_InitStructure.USART_BaudRate = bound; 				  // 波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;   // 数据长度 8个字节
	USART_InitStructure.USART_StopBits = USART_StopBits_1;		  // 1位停止位
	USART_InitStructure.USART_Parity = USART_Parity_No; 		  // 无奇偶校验
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(UART4, &USART_InitStructure);

    // 清除标志位，以防以外判断有中断请求
	USART_ClearITPendingBit(UART4, USART_IT_RXNE);			// 清除"接收寄存器非空"中断标志
	USART_ClearITPendingBit(UART4, USART_IT_TXE);			// 清除"发送寄存器为空"中断标志
	USART_ClearITPendingBit(UART4, USART_IT_TC);			// 清除"发送完成"中断标志	

	USART_ITConfig(UART4, USART_IT_TC, DISABLE);			// 禁用"发送完成"中断
	USART_ITConfig(UART4, USART_IT_RXNE, DISABLE);			// 禁用"接收寄存器非空"中断
	USART_ITConfig(UART4, USART_IT_IDLE, ENABLE);	        // 开启空闲中断
	
	USART_DMACmd(UART4,USART_DMAReq_Tx|USART_DMAReq_Rx,ENABLE);  // 使能UART4 DMA发送和接收
	USART_Cmd(UART4, ENABLE);		                             // 使能串口

    // 5. DMA配置
    //Tx DMA CONFIG:	UART4 TX 对应DAM通道5
	DMA_Cmd(DMA2_Channel5,DISABLE); 									//close DMA Channe5
	DMA_DeInit(DMA2_Channel5);

	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&UART4->DR;   
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)UART4_DMA_Tx_Buf;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;		
	DMA_InitStructure.DMA_BufferSize = 0;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; 
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; 	
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; 
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;   
	
	DMA_Init(DMA2_Channel5, &DMA_InitStructure);	// 配置DMA通道		
	DMA_Cmd (DMA2_Channel5,DISABLE);	            // 初始：禁止 -> 否则使能后就开始发送了！
	

    //Rx DMA CONFIG:	UART4 RX 对应DAM2通道3
	DMA_DeInit(DMA2_Channel3);	 //串口4对应DMA2通道3
	
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&UART4->DR; 
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)UART4_DMA_Rece_Buf;  
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;	
	DMA_InitStructure.DMA_BufferSize = USART4_MAX_RECV_LEN; 
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; 
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; 
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; 
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; 
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal; 
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; 
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA2_Channel3, &DMA_InitStructure);  
	
	DMA_Cmd(DMA2_Channel3, ENABLE);         // 初始使能，开始等待接收数据

	USART4_RX_STA = 0;

} 

void UART4_DMA_send(INT8U* buffer, u8 size)
	{
		if((buffer == 0) || (size == 0U) || (size > UART4_DMA_TX_BSIZE))
		{
			return;
		}

		memcpy(UART4_DMA_Tx_Buf, buffer,size);
		DMA_Cmd(DMA2_Channel5, DISABLE);        // 禁用DMA （*）
		DMA2_Channel5->CNDTR = size;            // 设置发送长度
		DMA_Cmd(DMA2_Channel5, ENABLE);         // 启动DMA发送
	}

void UART4_IRQHandler(void)
{	
	INT8U temp = temp;
	if(USART_GetITStatus(UART4, USART_IT_IDLE) != RESET) //接收中断
	{
        // 读取SR+DR， 清除空闲中断标志
		temp = (UART4->SR);
		temp = (UART4->DR); 

		DMA_Cmd(DMA2_Channel3,DISABLE);
		
        // 计算本帧数据长度
		LEN4 =USART4_MAX_RECV_LEN-DMA_GetCurrDataCounter(DMA2_Channel3); 
		memcpy(UART4_RX_BUF,UART4_DMA_Rece_Buf, USART4_MAX_RECV_LEN);
			
		DMA2_Channel3->CNDTR = USART4_MAX_RECV_LEN;         // 重置DMA接收长度
		DMA_Cmd(DMA2_Channel3,ENABLE);                      // 重启DMA接收  

		USART4_RX_STA = 1;
		
		USART_ClearITPendingBit(UART4,USART_IT_IDLE);		  // 清除中断标志		
	}
}





/********************************** 串口5 *****************************************/
/**
 * @name    USART5
 * @brief   串口5 初始化函数
 * @param   -USART5_Gpio_Config(void);
 * @retval  
 * @note   
 */
/* RCC CONFIG */
void USART5_Rcc_Config(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD|RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5,ENABLE);	
}


/* GPIO CONFIG */
void USART5_Gpio_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // USART5_RX (PD2) -> floating input
 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;	
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    // USART5_TX （PC12）-> push-pull
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
} 


/* Nvic Configuration */
void USART5_Nvic_Config(void)	
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	/* Enable the USAR1 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;   // 抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		    // 子优先级1
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure);
}



void USART5_Init(u32 bound)
{
	USART_InitTypeDef USART_InitStructure;

	USART5_Rcc_Config();
	USART5_Gpio_Config();
	USART5_Nvic_Config();

	USART_InitStructure.USART_BaudRate = bound;			          
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;	 
	USART_InitStructure.USART_StopBits = USART_StopBits_1;	
	USART_InitStructure.USART_Parity = USART_Parity_No;	
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(UART5, &USART_InitStructure);


	USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);	   //接收中断使能

	USART_Cmd(UART5, ENABLE);

	USART5_RX_STA=0;

}


// 0x0D是回车的ASCLL码，0x0A是换行的ASCLL码  <- 接收到得数据必须以这两个结尾
void UART5_IRQHandler(void)
{
 	volatile char res5;    
	if(USART_GetITStatus(UART5, USART_IT_RXNE) != RESET)    // 接收到数据
	{	 
		res5=(char)USART_ReceiveData(UART5);
        USART_ClearITPendingBit(UART5, USART_IT_RXNE); 
        SubBoard_LinkOnRxByte((u8)res5);

		if(USART5_RX_STA<USART5_MAX_RECV_LEN)		 // 还可以接收数据
		{
			CntRx5=10;				                 // 一接收到串口中断，就将计数器的值置为10 
			USART5_RX_BUF[USART5_RX_STA++]=res5;	 // 存储接收到的值	
		}else 
		{
			USART5_RX_STA|=1<<15;					// 强制标记接收完成
		} 
	}
}


/********************************** 串口打印函数 *********************************************/
/***************************************************************************************************
 * @name     *itoa(int value, char *string, int radix)
 * @brief   将整形数据转换成字符串
 * @param   -radix =10 表示10进制，其他结果为0
 * 			-value 要转换的整形数
 * 			-buf 转换后的字符串
 * @retval  None 
 * @note    -radix = 10
 * @call	被USART1_printf()调用
 ***************************************************************************************************/
static char *itoa(int value, char *string, int radix)
{
    int     i, d;
    int     flag = 0;
    char    *ptr = string;
    unsigned int uvalue;
    int negative = 0;
    char temp[33];   /* 足够容纳32位二进制数（含符号） */

    if (radix < 2 || radix > 36)
    {
        *ptr = '\0';
        return string;
    }

    /* 处理0值 */
    if (value == 0)
    {
        *ptr++ = '0';
        *ptr ='\0';
        return string;
    }

    /* 处理负数：只对十进制支持负号，其他进制按无符号处理（兼容常见itoa行为） */
    if (radix != 10)
    {
        *ptr = 0;
        return string;
    }


    if (value < 0)
    {
        *ptr++ = '-';

        /* Make the value positive. */
        value *= -1;
    }

    for (i = 10000; i > 0; i /= 10)
    {
        d = value / i;

        if (d || flag)
        {
            *ptr++ = (char)(d + 0x30);
            value -= (d * i);
            flag = 1;
        }
    }

    /* Null terminate the string. */
    *ptr = 0;

    return string;

} /* NCL_Itoa */



/***************************************************************************************************
 * @name    *u8toh(u8 value, char *string)
 * @brief   将一个无符号8位二进制数转换成16进制字符串
 * @param   -value 要转换的8位二进制数 
 * @retval  None 
 * @note    被USART1_printf()调用
 ***************************************************************************************************/
static char *u8toh(u8 value, char *string)
{
    u8      i;
    int     d;
    char    *ptr = string;

    if (!value)
    {
        *ptr++ = 0x30;
        *ptr++ = 0x30;
        *ptr = 0;
        return string;
    }

    for(i=2;i>0;i--)
    {
        d = ( value >> ((i-1)*4) ) & 0x0F;
        if(d > 9){
            *ptr++ = (char)(d - 10 + 0x41);
        }
        else {
            *ptr++ = (char)(d + 0x30);   
        }
        }

    /* Null terminate the string. */
    *ptr = 0;

    return string;

} /* NCL_u8toh */



/***************************************************************************************************
 * @name	USART_printf(USART_TypeDef* USARTx, uint8_t *Data,...)
 * @brief   串口打印函数
 * @param   -USARTx 串口通道
 *		    -Data   要发送到串口的内容的指针
 *			-...    其他参数
 * @retval  None
 * @note   	外部调用
 *         	典型应用 USART_printf( USART1, "\r\n this is a demo \r\n" );
 *            		 USART_printf( USART1, "\r\n %d \r\n", i );
 *            		 USART_printf( USART1, "\r\n %s \r\n", j );
 ***************************************************************************************************/
void USART_printf(USART_TypeDef* USARTx, uint8_t *Data,...)
{
	const char *s;
    int d;   
    char buf[16];

    va_list ap;                 // 定义可变参数列表指针
    va_start(ap, Data);         // 初始化参数列表，绑定到最后一个固定参数（Data）


	while ( *Data != 0)         // 判断是否到达字符串结束符（'\0'）
	{				                          
		if ( *Data == 0x5c )    // 0x5c是'\'的ASCII码，检测到转义符
		{									  
			switch ( *++Data )
			{
				case 'r':							          //回车符
					USART_SendData(USARTx, 0x0d);
					Data ++;
					break;

				case 'n':							          //换行符
					USART_SendData(USARTx, 0x0a);	
					Data ++;
					break;
				
				default:
					Data ++;
				    break;
			}			 
		}
		else if ( *Data == '%')         // 检测到格式控制符起始符
		{									  
			switch ( *++Data )
			{				
				case 's':									// 字符串
					s = va_arg(ap, const char *);           // 从可变参数中提取字符串指针
                    for ( ; *s; s++) 
                    {
                        USART_SendData(USARTx,*s);
                        while( USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET );
                    }
                    Data++;
                    break;

                case 'd':									// 十进制
                    d = va_arg(ap, int);
                    itoa(d, buf, 10);
                    for (s = buf; *s; s++) 
					{
						USART_SendData(USARTx,*s);
						while( USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET );
                    }
					Data++;
                    break;

                case 'h':										//十六进制
                    d = va_arg(ap, unsigned int);
                    u8toh(d, buf);
                    for (s = buf; *s; s++) 
					{
						USART_SendData(USARTx,*s);
						while( USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET );
                    }
					Data++;
                    break;

				default:
					Data++;
				    break;
			}		 
		} 
		else USART_SendData(USARTx, *Data++);
		while( USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET );
	}
}



/***************************************************************************************************
 * @name	prtData(char *ary, INT8U len)
 * @brief   将字节数组以16进制格式打印出来
 * @param   ary：字节数组指针;
 * @note   
 ***************************************************************************************************/
void prtData(char *ary, INT8U len){
	INT8U i = 0;

	for(i=0; i<len; i++)
	{
		swgPrt("%02X ", ary[i]);
	}

	swgPrt("\n");
}


/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/
