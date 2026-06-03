/******************** (C) COPYRIGHT 2024 *****************************************
 * 文件��? ：usart.c
 * 描述    ��?个串口的初始��? 以及中断函数 
 *		                                         中断优先级排��?
 * 硬件配置��?-----------------------                          
 *          | PA9  - USART1(TX)      |      DMA      3  3
 *          | PA10 - USART1(RX)      |			DMA			 
 *          |                        |
 *          | PD5  - USART2(TX)  ��? |     	DMA      3  1
 *          | PD6  - USART2(RX)  ��? |			DMA
 *          |                        |
 *          | PB10 - USART3(TX)      |      DMA      2  3
 *          | PB11 - USART3(RX)      |			DMA
 *          |                        |
 *          | PC10 - USART4(TX)      |      DMA      0  0
 *          | PC11 - USART4(RX)      |			DMA
 *          |                        |
 *          | PC12 - USART5(TX)      |               2  0
 *          | PD2  - USART5(RX)      |
 *           ------------------------       
 * 版本    ��?
 * 修改日期��?
 * 作�?   ��?
 * 修改日志：串2  ��?   ��?  已改为DMA接收及发��?
*********************************************************************************/
#include "usart.h"
#include "subboard_link.h"


////串口1变量定义
//char USART1_RX_BUF[USART1_MAX_RECV_LEN]; 						//接收缓冲,最大USART1_MAX_RECV_LEN个字��?
//u16 USART1_RX_STA=0;																//串口1标志��?
//int LEN1=0;																					//串口1数据长度

//串口1变量定义  	DMA
INT8U USART1_DMA_Tx_Buf[USART1_DMA_TX_BSIZE]={0};	 	//USART1 发送数��?
char USART1_DMA_Rece_Buf[USART1_MAX_RECV_LEN]; 			//USART1 接收数组

char USART1_RX_BUF[USART1_MAX_RECV_LEN]; 						//接收缓冲,最大USART1_MAX_RECV_LEN个字��?
u16 USART1_RX_STA=0;                         				//串口1标志��?
int LEN1=0;																					//串口1数据长度
												

//串口2变量定义  	DMA
INT8U USART2_DMA_Tx_Buf[USART2_DMA_TX_BSIZE]={0};	 	//USART2 发送数��?
char USART2_DMA_Rece_Buf[USART2_MAX_RECV_LEN]; 			//USART2 接收数组

char USART2_RX_BUF[USART2_MAX_RECV_LEN]; 						//接收缓冲,最大USART2_MAX_RECV_LEN个字��?
u16 USART2_RX_STA=0;                         				//串口2标志��?
int LEN2=0;																					//串口2数据长度

 
//串口3变量定义		DMA
INT8U USART3_DMA_Tx_Buf[USART3_DMA_TX_BSIZE]={0};	 	//USART3 发送数��?
char USART3_DMA_Rece_Buf[USART3_MAX_RECV_LEN];			//USART3 接收数组

int LEN3=0;											   									//数据长度


//串口4变量定义		DMA
INT8U UART4_DMA_Tx_Buf[UART4_DMA_TX_BSIZE]={0};	   	//UART4 发送数��?
char UART4_DMA_Rece_Buf[USART4_MAX_RECV_LEN];     	//UART4 接收数组

char USART4_RX_BUF[USART4_MAX_RECV_LEN]; 						//接收缓冲,最大USART4_MAX_RECV_LEN个字��?
u16 USART4_RX_STA=0;                             		//串口4标志��?
int LEN4=0;											   									//数据长度


//串口5变量定义
char USART5_RX_BUF[USART5_MAX_RECV_LEN];           	//串口5接收缓存数组
u16 USART5_RX_STA=0;                             	 	//串口5标志��?
int LEN5=0;



///********************************** 串口1 *****************************************/
///*
// * 函数名：USART1_Rcc_Config
// * 描述  ��?
// * 输入  ：无
// * 输出  : ��?
// * 调用  ：外部调��?
// */
//void USART1_Rcc_Config(void)
//{
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO, ENABLE);	
//}


///*
// * 函数名：USART1_Gpio_Config
// * 描述  ��?
// * 输入  ：无
// * 输出  : ��?
// * 调用  ：外部调��?
// */
//void USART1_Gpio_Config(void)
//{
// 	GPIO_InitTypeDef GPIO_InitStructure;

//  /* Configure USART1 Rx as input floating */
// 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
// 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
// 	GPIO_Init(GPIOA, &GPIO_InitStructure);
// 
//  /* Configure USART1 Tx as alternate function push-pull */
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
// 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//	GPIO_Init(GPIOA, &GPIO_InitStructure);
//} 


///*
// * 函数名：USART1_Nvic_Config
// * 描述  ��?
// * 输入  ：无
// * 输出  : ��?
// * 调用  ：外部调��?
// */
//void USART1_Nvic_Config(void)				         
//{
//  NVIC_InitTypeDef NVIC_InitStructure;
// 
//  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
//  
//  /* Enable the USAR1 Interrupt */
//  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
//  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
//  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//  NVIC_Init(&NVIC_InitStructure);
//}



///*
// * 函数名：USART1_Init
// * 描述  ：串��?初始��?
// * 输入  ：bound  波特��?
// * 输出  :  
// * 调用  ：外部调��?
// */
//void USART1_Init(u32 bound)
//{
//	USART_InitTypeDef USART_InitStructure;

//	USART1_Rcc_Config();
//	USART1_Gpio_Config();
//	USART1_Nvic_Config();

//	USART_InitStructure.USART_BaudRate = bound;
//	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
//	USART_InitStructure.USART_StopBits = USART_StopBits_1;
//	USART_InitStructure.USART_Parity = USART_Parity_No;
//	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
//	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

//  /* Configure USART1 */
//	USART_Init(USART1, &USART_InitStructure);
//  /* Enable USART1 Receive and Transmit interrupts */
//	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);	   //接收中断使能
//  /* Enable the USARTy */
//	USART_Cmd(USART1, ENABLE);

//	USART1_RX_STA=0;  
//}

//	



///*
// * 函数名：USART1_IRQHandler
// * 描述  ��?
// * 输入  ：无
// * 输出  : ��?
// * 调用  ：外部调��?
// */
//void USART1_IRQHandler(void)
//{
// 	char res;	    
//	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)//接收到数��?
//	{	 
//		res=USART_ReceiveData(USART1); 			 
//		if(USART1_RX_STA<USART1_MAX_RECV_LEN)		//还可以接收数��?
//		{
//		 	CntRx1=7;
//			USART1_RX_BUF[USART1_RX_STA++]=res;		//记录接收到的��? 
//		}else 
//		{
//			USART1_RX_STA|=1<<15;					//强制标记接收完成
//		}
//	}
//}    


///*
// * 函数名：fputc
// * 描述  ：重定向c库函数printf到USART1
// *         将printf函数重定向到USART1
// *         这样就可以用printf函数将单片机的数据打印到PC上的超级终端或串口调试助手�?
// * 输入  ：无
// * 输出  ：无
// * 调用  ：由printf调用
// */
//int fputc(int ch, FILE *f)
//{
///* 将Printf内容发往串口 */
//  USART_SendData(USART1, (unsigned char) ch);
//  while (!(USART1->SR & USART_FLAG_TXE));
// 
//  return (ch);
//}


/********************************** 串口1 *****************************************/
/*
 * 函数名：USART1_Init
 * 描述  ��?
 * 输入  ：无
 * 输出  : ��?
 * 调用  ：外部调��?
 */
void USART1_Init(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);	//注意USART1是APB2时钟树！��?
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);		//开启DMA时钟
	
	USART_DeInit(USART1);	//复位USART1

  /* Configure USART1 Rx as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
 
  /* Configure USART1 Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_Init(&NVIC_InitStructure);
	
	//USART1 初始化配��?
	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART1, &USART_InitStructure);					//初始化串��?

	USART_ClearITPendingBit(USART1, USART_IT_RXNE); 			//难道上来就进了中��?
	USART_ClearITPendingBit(USART1, USART_IT_TXE);				//难道上来就进了中��?
	USART_ClearITPendingBit(USART1, USART_IT_TC);					//难道上来就进了中��?

	USART_ITConfig(USART1, USART_IT_TC, DISABLE);					//禁用发送中��?
	USART_ITConfig(USART1, USART_IT_RXNE, DISABLE); 			//禁用接收中断
	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);	   		//开启空闲中��?
	USART_DMACmd(USART1,USART_DMAReq_Tx|USART_DMAReq_Rx,ENABLE);	   //使能USART1 DMA发送和接收
	USART_Cmd(USART1, ENABLE);		 //使能串口
	
	 //Tx DMA CONFIG	USART1 TX 对应DAM1通道4
	DMA_Cmd(DMA1_Channel4,DISABLE); 									//close DMA Channel
	DMA_DeInit(DMA1_Channel4);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART1->DR;   // 设置DMA源地址：串口数据寄存器地址*/	
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)USART1_DMA_Tx_Buf;	 // 内存地址(要传输的变量的指��? 
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;	 // 方向：从内存到外��?			
	DMA_InitStructure.DMA_BufferSize = 0;	// 传输大小 	
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;   // 外设地址不增		
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;   // 内存地址自增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; 	// 外设数据单位		
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;   // 内存数据单位 	
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal	;// DMA模式，一次或者循环模��?	
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;	 // 优先级：��?	
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;	// 禁止内存到内存的传输	   
	
	DMA_Init(DMA1_Channel4, &DMA_InitStructure);		// 配置DMA通道		
	DMA_Cmd (DMA1_Channel4,DISABLE);	// 初始化为禁止，否则使能后就开始发送了！！！！！！��?
	
	//Rx DMA CONFIG   USART1 RX 对应DAM通道5
	DMA_DeInit(DMA1_Channel5);	 //串口1对应DMA1通道5
	
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART1->DR; //外设地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)USART1_DMA_Rece_Buf;  //内存地址
	
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;	//数据传输方向从外设读取发送到内存
	DMA_InitStructure.DMA_BufferSize = USART1_MAX_RECV_LEN;	//DMA通道的DMA缓存大小
	
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //外设地址寄存器不��?
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //内存地址寄存器递增
	
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //外设数据字长
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;  //内存数据子字��?
	
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  //工作在正常缓冲模��?
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMA通道x有中优先��?
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMA通道x没有设置内存到内存传��?
	DMA_Init(DMA1_Channel5, &DMA_InitStructure);  
	
	DMA_Cmd(DMA1_Channel5, ENABLE);  //使能通道5 开始接��?
	USART1_RX_STA=0;
}


void USART1_DMA_send(INT8U* buffer, u8 size)
{
	memcpy(USART1_DMA_Tx_Buf, buffer,size);
	DMA_Cmd(DMA1_Channel4, DISABLE);
	DMA1_Channel4->CNDTR = size;// 设置发送长��?
	DMA_Cmd(DMA1_Channel4, ENABLE);  // 启动DMA发�?
}


void USART1_IRQHandler(void)
{	
	INT8U temp = temp;
	if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET) //接收中断
		{
		temp = (USART1->SR);
		temp = (USART1->DR);	
		DMA_Cmd(DMA1_Channel5,DISABLE);
			
		LEN1 =USART1_MAX_RECV_LEN-DMA_GetCurrDataCounter(DMA1_Channel5); //算出本帧数据长度
		memcpy(USART1_RX_BUF,USART1_DMA_Rece_Buf, USART1_MAX_RECV_LEN);
			
		DMA1_Channel5->CNDTR = USART1_MAX_RECV_LEN;
		DMA_Cmd(DMA1_Channel5,ENABLE);
			
		USART1_RX_STA=1;
	
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
/*
 * 函数名：USART2_Init
 * 描述  ��?
 * 输入  ：无
 * 输出  : ��?
 * 调用  ：外部调��?
 */
void USART2_Init(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD|RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);		//开启DMA时钟
	
	USART_DeInit(USART2);	//复位USART2
	
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


	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel7_IRQn;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel6_IRQn;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_Init(&NVIC_InitStructure);
	
	  //USART2 初始化配��?
	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART2, &USART_InitStructure);					//初始化串��?

	USART_ClearITPendingBit(USART2, USART_IT_RXNE); 			//难道上来就进了中��?
	USART_ClearITPendingBit(USART2, USART_IT_TXE);				//难道上来就进了中��?
	USART_ClearITPendingBit(USART2, USART_IT_TC);				//难道上来就进了中��?

	USART_ITConfig(USART2, USART_IT_TC, DISABLE);				//禁用发送中��?
	USART_ITConfig(USART2, USART_IT_RXNE, DISABLE); 			//禁用接收中断
	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);	   //开启空闲中��?
	USART_DMACmd(USART2,USART_DMAReq_Tx|USART_DMAReq_Rx,ENABLE);	   //使能USART2 DMA发送和接收
	USART_Cmd(USART2, ENABLE);		 //使能串口
	
	 //Tx DMA CONFIG	USART2 TX 对应DAM通道7
	DMA_Cmd(DMA1_Channel7,DISABLE); 									//close DMA Channel
	DMA_DeInit(DMA1_Channel7);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART2->DR;   // 设置DMA源地址：串口数据寄存器地址*/	
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)USART2_DMA_Tx_Buf;	 // 内存地址(要传输的变量的指��? 
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;	 // 方向：从内存到外��?			
	DMA_InitStructure.DMA_BufferSize = 0;	// 传输大小 	
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;   // 外设地址不增		
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;   // 内存地址自增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; 	// 外设数据单位		
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;   // 内存数据单位 	
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal	;// DMA模式，一次或者循环模��?	
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;	 // 优先级：��?	
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;	// 禁止内存到内存的传输	   
	
	DMA_Init(DMA1_Channel7, &DMA_InitStructure);		// 配置DMA通道		
	DMA_Cmd (DMA1_Channel7,DISABLE);	// 初始化为禁止，否则使能后就开始发送了！！！！！！��?
	
	//Rx DMA CONFIG   USART2 RX 对应DAM通道6
	DMA_DeInit(DMA1_Channel6);	 //串口2对应DMA1通道6
	
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART2->DR; //外设地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)USART2_DMA_Rece_Buf;  //内存地址
	
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;	//数据传输方向从外设读取发送到内存
	DMA_InitStructure.DMA_BufferSize = USART2_MAX_RECV_LEN;	//DMA通道的DMA缓存大小
	
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //外设地址寄存器不��?
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //内存地址寄存器递增
	
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //外设数据字长
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;  //内存数据子字��?
	
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  //工作在正常缓冲模��?
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMA通道x有中优先��?
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMA通道x没有设置内存到内存传��?
	DMA_Init(DMA1_Channel6, &DMA_InitStructure);  
	DMA_ITConfig(DMA1_Channel6, DMA_IT_TC, ENABLE);
	
	uart_rx_driver_init(UART_RX_PORT_USART2);
	DMA_Cmd(DMA1_Channel6, ENABLE);  //使能通道6 开始接��?	USART2_RX_STA=0;
}


void USART2_DMA_send(INT8U* buffer, u8 size)
{
	u32 timeout = 0x000FFFFFU;

	if((buffer == 0) || (size == 0U) || (size > USART2_DMA_TX_BSIZE))
	{
		return;
	}

	while(((DMA1_Channel7->CCR & 0x0001U) != 0U) &&
	      (DMA_GetFlagStatus(DMA1_FLAG_TC7) == RESET) &&
	      (timeout > 0U))
	{
		timeout--;
	}

	DMA_Cmd(DMA1_Channel7, DISABLE);
	DMA_ClearFlag(DMA1_FLAG_TC7);
	memcpy(USART2_DMA_Tx_Buf, buffer, size);
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
			
		LEN2 =USART2_MAX_RECV_LEN-DMA_GetCurrDataCounter(DMA1_Channel6); //算出本帧数据长度
		if(LEN2 > 0)
		{
			uart_rx_driver_put(UART_RX_PORT_USART2, (uint8_t *)USART2_DMA_Rece_Buf, (uint32_t)LEN2);
		}
			
		DMA1_Channel6->CNDTR = USART2_MAX_RECV_LEN;
		DMA_Cmd(DMA1_Channel6,ENABLE);
			
		USART2_RX_STA=0;
	
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

/*
 * 函数名：USART3_Init(void)
 * 描述  ��?
 * 输入  ：无
 * 输出  : ��?
 * 调用  ：外部调��?
 */
void USART3_Init(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);		//开启DMA时钟
	
	USART_DeInit(USART3);	//复位USART3
  //USART3_TX	 PB.10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure); 

	//USART3_RX  PB.11
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure); 


	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel2_IRQn;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel3_IRQn;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_Init(&NVIC_InitStructure);


  //USART3 初始化配��?
	USART_InitStructure.USART_BaudRate = bound; 				  //波特率设��?
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;   //数据长度 8个字��?
	USART_InitStructure.USART_StopBits = USART_StopBits_1;		  //1位停止位
	USART_InitStructure.USART_Parity = USART_Parity_No; 		   //无奇偶校��?
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART3, &USART_InitStructure);					//初始化串��?

	USART_ClearITPendingBit(USART3, USART_IT_RXNE); 			//难道上来就进了中��?
	USART_ClearITPendingBit(USART3, USART_IT_TXE);				//难道上来就进了中��?
	USART_ClearITPendingBit(USART3, USART_IT_TC);				//难道上来就进了中��?

	USART_ITConfig(USART3, USART_IT_TC, DISABLE);				//禁用发送中��?
	USART_ITConfig(USART3, USART_IT_RXNE, DISABLE); 			//禁用接收中断
	USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);	   //开启空闲中��?
	USART_DMACmd(USART3,USART_DMAReq_Tx|USART_DMAReq_Rx,ENABLE);	   //使能USART3 DMA发送和接收
	USART_Cmd(USART3, ENABLE);		 //使能串口

 //Tx DMA CONFIG	USART3 TX 对应DAM通道2
	DMA_Cmd(DMA1_Channel2,DISABLE); 									//close DMA Channel
	DMA_DeInit(DMA1_Channel2);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART3->DR;   // 设置DMA源地址：串口数据寄存器地址*/	
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)USART3_DMA_Tx_Buf;	 // 内存地址(要传输的变量的指��? 
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;	 // 方向：从内存到外��?			
	DMA_InitStructure.DMA_BufferSize = 0;	// 传输大小 	
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;   // 外设地址不增		
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;   // 内存地址自增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; 	// 外设数据单位		
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;   // 内存数据单位 	
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal	;// DMA模式，一次或者循环模��?	
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;	 // 优先级：��?	
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;	// 禁止内存到内存的传输	   
	
	DMA_Init(DMA1_Channel2, &DMA_InitStructure);		// 配置DMA通道		
	DMA_Cmd (DMA1_Channel2,DISABLE);	// 初始化为禁止，否则使能后就开始发送了！！！！！！��?
	
	//Rx DMA CONFIG   USART3 RX 对应DAM通道3
	DMA_DeInit(DMA1_Channel3);	 //串口3对应DMA通道3
	
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART3->DR; //外设地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)USART3_DMA_Rece_Buf;  //内存地址
	
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;	//数据传输方向从外设读取发送到内存
	DMA_InitStructure.DMA_BufferSize = USART3_MAX_RECV_LEN;	//DMA通道的DMA缓存大小
	
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //外设地址寄存器不��?
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //内存地址寄存器递增
	
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //外设数据字长
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;  //内存数据子字��?
	
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  //工作在正常缓冲模��?
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMA通道x有中优先��?
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMA通道x没有设置内存到内存传��?
	DMA_Init(DMA1_Channel3, &DMA_InitStructure);  
	DMA_ITConfig(DMA1_Channel3, DMA_IT_TC, ENABLE);
	
	uart_rx_driver_init(UART_RX_PORT_USART3);
	DMA_Cmd(DMA1_Channel3, ENABLE);  //使能通道3 开始接��?

} 
   


void prtData(char *ary, INT8U len){
	INT8U i = 0;
	for(i=0;i<len;i++){
		swgPrt("%02X ",ary[i]);
	}
	swgPrt("\n");
}


void USART3_DMA_send(INT8U* buffer, u8 size)
	{
		memcpy(USART3_DMA_Tx_Buf, buffer,size);
		DMA_Cmd(DMA1_Channel2, DISABLE);
		DMA1_Channel2->CNDTR = size;// 设置发送长��?
		DMA_Cmd(DMA1_Channel2, ENABLE);  // 启动DMA发�?
	}

void USART3_IRQHandler(void)
{	
	INT8U temp = temp;
//	INT8U i=0;
	if(USART_GetITStatus(USART3, USART_IT_IDLE) != RESET) //接收中断
		{
		temp = (USART3->SR);
		temp = (USART3->DR);	
		DMA_Cmd(DMA1_Channel3,DISABLE);
		DMA_ClearITPendingBit(DMA1_IT_TC3);
			
		LEN3 =USART3_MAX_RECV_LEN-DMA_GetCurrDataCounter(DMA1_Channel3); //算出本帧数据长度
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
/*
 * 函数名：USART4_Init
 * 描述  ��?
 * 输入  ：无
 * 输出  : ��?
 * 调用  ：外部调��?
 */
void USART4_Init(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4,ENABLE);	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);		//开启DMA2时钟
	
	USART_DeInit(UART4);   //复位UART4
  /* Configure USART4 Rx as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
 
  /* Configure USART4 Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure); 


	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Channel4_IRQn;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;;
	NVIC_Init(&NVIC_InitStructure);


  //UART4 初始化配��?
	USART_InitStructure.USART_BaudRate = bound; 				  //波特率设��?
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;   //数据长度 8个字��?
	USART_InitStructure.USART_StopBits = USART_StopBits_1;		  	//1位停止位
	USART_InitStructure.USART_Parity = USART_Parity_No; 		   	//无校��?
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(UART4, &USART_InitStructure);					//初始化串��?

	USART_ClearITPendingBit(UART4, USART_IT_RXNE);			//难道上来就进了中��?
	USART_ClearITPendingBit(UART4, USART_IT_TXE);				//难道上来就进了中��?
	USART_ClearITPendingBit(UART4, USART_IT_TC);				//难道上来就进了中��?

	USART_ITConfig(UART4, USART_IT_TC, DISABLE);				//禁用发送中��?
	USART_ITConfig(UART4, USART_IT_RXNE, DISABLE);				//禁用接收中断
	USART_ITConfig(UART4, USART_IT_IDLE, ENABLE);	  //开启空闲中��?
	
	USART_DMACmd(UART4,USART_DMAReq_Tx|USART_DMAReq_Rx,ENABLE); 	  //使能UART4 DMA发送和接收
	USART_Cmd(UART4, ENABLE);		//使能串口

 //Tx DMA CONFIG	UART4 TX 对应DAM通道5
	DMA_Cmd(DMA2_Channel5,DISABLE); 									//close DMA Channe5
	DMA_DeInit(DMA2_Channel5);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&UART4->DR;   // 设置DMA源地址：串口数据寄存器地址*/ 
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)UART4_DMA_Tx_Buf;	// 内存地址(要传输的变量的指��? 
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;	 // 方向：从内存到外��?			
	DMA_InitStructure.DMA_BufferSize = 0;	// 传输大小 	
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;   // 外设地址不增		
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;   // 内存地址自增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; 	// 外设数据单位		
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;   // 内存数据单位 	
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal	;// DMA模式，一次或者循环模��?	
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;	 // 优先级：��?	
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;	// 禁止内存到内存的传输	   
	
	DMA_Init(DMA2_Channel5, &DMA_InitStructure);		// 配置DMA通道		
	DMA_Cmd (DMA2_Channel5,DISABLE);	// 初始化为禁止，否则使能后就开始发送了！！！！！！��?
	

	/*******Rx DMA CONFIG	UART4 RX 对应DAM2通道3************/
	DMA_DeInit(DMA2_Channel3);	 //串口4对应DMA2通道3
	
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&UART4->DR; //外设地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)UART4_DMA_Rece_Buf;  //内存地址
	
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;	//数据传输方向从外设读取发送到内存
	DMA_InitStructure.DMA_BufferSize = USART4_MAX_RECV_LEN;  //DMA通道的DMA缓存大小
	
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //外设地址寄存器不��?
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //内存地址寄存器递增
	
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //外设数据字长
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;  //内存数据子字��?
	
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  //工作在正常缓冲模��?
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMA通道x有中优先��?
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMA通道x没有设置内存到内存传��?
	DMA_Init(DMA2_Channel3, &DMA_InitStructure);  
	
	DMA_Cmd(DMA2_Channel3, ENABLE);  //使能DMA2通道3 开始接��?
	USART4_RX_STA = 0;

} 

void UART4_DMA_send(INT8U* buffer, u8 size)
	{
		memcpy(UART4_DMA_Tx_Buf, buffer,size);
		DMA_Cmd(DMA2_Channel5, DISABLE);
		DMA2_Channel5->CNDTR = size;// 设置发送长��?
		DMA_Cmd(DMA2_Channel5, ENABLE);  // 启动DMA发�?
	}
 
void UART4_IRQHandler(void)
{	
	INT8U temp = temp;
	if(USART_GetITStatus(UART4, USART_IT_IDLE) != RESET) //接收中断
		{
		temp = (UART4->SR);
		temp = (UART4->DR); 
		DMA_Cmd(DMA2_Channel3,DISABLE);
			
		LEN4 =USART4_MAX_RECV_LEN-DMA_GetCurrDataCounter(DMA2_Channel3); //算出本帧数据长度
		memcpy(USART4_RX_BUF,UART4_DMA_Rece_Buf, USART4_MAX_RECV_LEN);
			
		DMA2_Channel3->CNDTR = USART4_MAX_RECV_LEN;
		DMA_Cmd(DMA2_Channel3,ENABLE);
		USART4_RX_STA = 1;
	
			/*******************test***********************/
//			swgPrt ("LEN4:%d\r\n",LEN4);
//			prtData(USART4_RX_BUF,LEN4);
			/*********************************************/
			
		USART_ClearITPendingBit(UART4,USART_IT_IDLE);		  //清除中断标志		
		}
}





/********************************** 串口5 *****************************************/
/*
 * 函数名：USART5_Rcc_Config
 * 描述  ��?
 * 输入  ：无
 * 输出  : ��?
 * 调用  ：外部调��?
 */
void USART5_Rcc_Config(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD|RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5,ENABLE);	
}


/*
 * 函数名：USART5_Gpio_Config
 * 描述  ��?
 * 输入  ：无
 * 输出  : ��?
 * 调用  ：外部调��?
 */
void USART5_Gpio_Config(void)
{
 	GPIO_InitTypeDef GPIO_InitStructure;

  /* Configure USART5 Rx as input floating */
 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;			 //PD2 串口5输入��?
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
 	GPIO_Init(GPIOD, &GPIO_InitStructure);
 
  /* Configure USART5 Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;		     //PC12 串口5输出��?
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
} 


/*
 * 函数名：USART5_Nvic_Config
 * 描述  ��?
 * 输入  ：无
 * 输出  : ��?
 * 调用  ：外部调��?
 */
void USART5_Nvic_Config(void)				         // nvic configuration
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	/* Enable the USAR1 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;  //抢占优先��? 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;			//子优先级1
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}



/*
 * 函数名：USART5_Init
 * 描述  ��?
 * 输入  ：无
 * 输出  : ��?
 * 调用  ：外部调��?
 */
void USART5_Init(u32 bound)
{
	USART_InitTypeDef USART_InitStructure;

	USART5_Rcc_Config();
	USART5_Gpio_Config();
	USART5_Nvic_Config();

	USART_InitStructure.USART_BaudRate = bound;			          //波特率设��?
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;	  //数据长度 8个字��?
	USART_InitStructure.USART_StopBits = USART_StopBits_1;		  //1位停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;			   //无奇偶校��?
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

  /* Configure USART5 */
	USART_Init(UART5, &USART_InitStructure);
  /* Enable USART5 Receive and Transmit interrupts */
	USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);	   //接收中断使能
  /* Enable the USARTy */
	USART_Cmd(UART5, ENABLE);			   //开启中��?

	USART5_RX_STA=0;
}
 
 



/*
 * 函数名：UART5_IRQHandler
 * 描述  ��?
 * 输入  ：无
 * 输出  : ��?
 * 调用  ：外部调��?
 */
void UART5_IRQHandler(void)
{
    u8 res5;

    if(USART_GetITStatus(UART5, USART_IT_RXNE) != RESET)
    {
        res5 = (u8)USART_ReceiveData(UART5);
        SubBoard_LinkOnRxByte(res5);
    }
}


/********************************** 串口打印函数 *********************************************/
/*
 * 函数名：itoa
 * 描述  ：将整形数据转换成字符串
 * 输入  ��?radix =10 表示10进制，其他结果为0
 *         -value 要转换的整形��?
 *         -buf 转换后的字符��?
 *         -radix = 10
 * 输出  ：无
 * 返回  ：无
 * 调用  ：被USART1_printf()调用
 */
static char *itoa(int value, char *string, int radix)
{
    int     i, d;
    int     flag = 0;
    char    *ptr = string;

    /* This implementation only works for decimal numbers. */
    if (radix != 10)
    {
        *ptr = 0;
        return string;
    }

    if (!value)
    {
        *ptr++ = 0x30;
        *ptr = 0;
        return string;
    }

    /* if this is a negative value insert the minus sign. */
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



/*
 * 函数名：u8toh
 * 描述  ：将一个无符号8位二进制数转换成16进制字符��?
 * 输入  ��?value 要转换的8位二进制��?
 * 输出  ：无
 * 返回  ：无
 * 调用  ：被USART1_printf()调用
 */
static char *u8toh(u8 value, char *string)
{
    u8 i;
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
      if(d>9)
        *ptr++ = (char)(d - 10 + 0x41);
      else  
        *ptr++ = (char)(d + 0x30);    
    }

    /* Null terminate the string. */
    *ptr = 0;

    return string;

} /* NCL_u8toh */



/*
 * 函数名：USART_printf
 * 描述  ��?
 * 输入  ��?USARTx 串口通道
 *		     -Data   要发送到串口的内容的指针
 *			   -...    其他参数
 * 输出  ：无
 * 返回  ：无 
 * 调用  ：外部调��?
 *         典型应用USART_printf( USART1, "\r\n this is a demo \r\n" );
 *            		 USART_printf( USART1, "\r\n %d \r\n", i );
 *            		 USART_printf( USART1, "\r\n %s \r\n", j );
 */
void USART_printf(USART_TypeDef* USARTx, uint8_t *Data,...)
{
	const char *s;
  int d;   
  char buf[16];

  va_list ap;
  va_start(ap, Data);

	while ( *Data != 0)     // 判断是否到达字符串结束符
	{				                          
		if ( *Data == 0x5c )  //'\'
		{									  
			switch ( *++Data )
			{
				case 'r':							          //回车��?
					USART_SendData(USARTx, 0x0d);
					Data ++;
					break;

				case 'n':							          //换行��?
					USART_SendData(USARTx, 0x0a);	
					Data ++;
					break;
				
				default:
					Data ++;
				    break;
			}			 
		}
		else if ( *Data == '%')
		{									  //
			switch ( *++Data )
			{				
				case 's':										  //字符��?
					s = va_arg(ap, const char *);
          for ( ; *s; s++) 
					{
						USART_SendData(USARTx,*s);
						while( USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET );
          }
					Data++;
          break;

        case 'd':										//十进��?
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
		} /* end of else if */
		else USART_SendData(USARTx, *Data++);
		while( USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET );
	}
}





/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/



