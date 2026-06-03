/******************** (C) COPYRIGHT 2024 *****************************************
 * 文件�? ：can1.c
 * 描述    ：can1 接口的初始化
 * 硬件配置: -----------------
 *          |  PD0-CAN1-RX     |
 *          |  PD1-CAN1-TX     |
 *           -----------------      
 * 版本    �?
 * 修改日期�?                                                                                                                                                                                                                                                                                                                                                                                                                                  
 * 作�?   �?
 * 修改日志�?
*********************************************************************************/
#include "can1.h"
#include "usart.h"

u8 canrxbuf1[8];
s16 io_temp[4] = {0};

/************************** 初始化报�?****************************************/    

u8 CAN1_SendMsg1[8]={0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01};   /* CAN1 TEST Msg */  
u8 CAN1_SendMsg2[2]={0x01,0x01};   /* CAN1 TEST Msg */  


/**************************************************************************************/
//位置�? 要开机发送？
//初始化（一次）->CAN中断接收位置、转�?>（中断里？）转换为线速度与长�?>通过串口DMA发送线速度�?）与长度�?）数据，控制电机与离合器
/**************************************************************************************/

//typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;


///* 在中断处理函数中返回 */
//__IO uint32_t ret = 0;

//volatile TestStatus TestRx;	

/*
 * 函数名：USER_CAN1_Init
 * 描述  ：CAN1初始化，包括端口初始化和中断优先级初始化
 * 输入  ：无
 * 输出  : �? 
 * 调用  ：外部调�?
 */
void USER_CAN1_Init(void)
{
	CAN1_NVIC_Configuration();
	CAN1_GPIO_Config();
	CAN1_Init_Config();
}


/*
 * 函数名：CAN1_NVIC_Configuration
 * 描述  ：CAN1 RX0 中断优先级配�?
 * 输入  ：无
 * 输出  : �?
 * 调用  ：内部调�?
 */
void CAN1_NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  /* Enable CAN1 RX0 interrupt IRQ channel */
  NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;     // 主优先级�?
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;            // 次优先级�?
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}


/*
 * 函数名：CAN1_GPIO_Config
 * 描述  ：CAN1 GPIO 和时钟配�?
 * 输入  ：无
 * 输出  : �?
 * 调用  ：内部调�?
 */
void CAN1_GPIO_Config(void)
{ 
  GPIO_InitTypeDef GPIO_InitStructure; 
  	 
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOD, ENABLE);	                        											 

  /* CAN Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);

  /***********Configure CAN1 pin ************/									
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	//上拉输入
	GPIO_Init(GPIOD, &GPIO_InitStructure);
  								
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_PinRemapConfig(GPIO_Remap2_CAN1, ENABLE);  
}


/*
 * 函数名：CAN1_Init_Config
 * 描述  ：CAN1 参数初始�?
 * 输入  ：无
 * 输出  : �? 
 * 调用  ：外部调�?
 */
void CAN1_Init_Config(void)
{
  CAN_InitTypeDef        CAN_InitStructure;

  CAN_DeInit(CAN1);

  /* CAN register init */
  CAN_StructInit(&CAN_InitStructure);

  /* CAN cell init */
  CAN_InitStructure.CAN_TTCM=DISABLE;           	// 时间触发通信禁止
  CAN_InitStructure.CAN_ABOM=DISABLE;	          	// 离线退出是在中断置位清0后退�?
  CAN_InitStructure.CAN_AWUM=DISABLE;	          	// 自动唤醒模式：清零sleep
  CAN_InitStructure.CAN_NART=ENABLE;	          	// 自动重新传送豹纹，知道发送成�?
  CAN_InitStructure.CAN_RFLM=DISABLE;	          	// FIFO没有锁定，新报文覆盖旧报�?
  CAN_InitStructure.CAN_TXFP=DISABLE;             // 发送报文优先级确定：标志符
  CAN_InitStructure.CAN_Mode=CAN_Mode_Normal;     // 回环模式
  CAN_InitStructure.CAN_SJW=CAN_SJW_1tq;          // 1tq、BS1、BS2的值跟波特率有�?
  CAN_InitStructure.CAN_BS1=CAN_BS1_9tq;          //  1     5      2     9
  CAN_InitStructure.CAN_BS2=CAN_BS2_8tq;
  CAN_InitStructure.CAN_Prescaler=20;			  			// 分频系数�?0 36M/20/(1+9+8)=100k    36M/9/(1+8+7)=250k    36M/45/(1+8+7)=50k   36M/9/(1+5+2)=500k 

  CAN_Init(CAN1, &CAN_InitStructure);			  			// 初始化CAN1

  CAN1_Filter_Config();

  /* CAN FIFO0 message pending interrupt enable */        
  CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE); 		  //CAN1中断使能,进入中断后读fifo的报文函数释放报文清中断标志
}


/*
 * 函数名：CAN1_Filter_Config
 * 描述  ：CAN1过滤�?
 * 输入  ：无
 * 输出  : �? 
 * 调用  �?       
 */
void CAN1_Filter_Config(void)
{
	//标识符列表模�?
	CAN_FilterInitTypeDef  CAN_FilterInitStructure;

	CAN_FilterInit(&CAN_FilterInitStructure); //初始化CAN_FilterInitStructrue结构体变�?
																							//两个CAN使用的时候不要随便取消注�?

	//CAN1过滤器设�?
	CAN_FilterInitStructure.CAN_FilterNumber=0;
	CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask;
	CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit;
	CAN_FilterInitStructure.CAN_FilterIdHigh=0x0000;
	CAN_FilterInitStructure.CAN_FilterIdLow=0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh=0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow=0x0000;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_FIFO0;
	CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;
	CAN_FilterInit(&CAN_FilterInitStructure);
}

	
/*
 * 函数名：Can1_Send_Msg
 * 描述  ：使用can1发送一组数�?
 *         msg为发送的数据指针
 *         发送数据为标准 数据�?
 *         dlc 数据长度
 * 输入  ：无
 * 输出  :  	 
 * 调用  ：外部调�?
 */
u8 Can1_Send_Msg(u8 *msg, u32 id, u8 dlc)
{
	u8 mbox;
	u16 i=0;
	CanTxMsg TxMessage;
	TxMessage.StdId=id;					 			// 标准标识符为id
	TxMessage.ExtId=0x00;								// 扩展帧ID
	TxMessage.IDE=CAN_ID_STD;					// 标准�?
	TxMessage.RTR=CAN_RTR_DATA;		          	// 数据�?
	TxMessage.DLC=dlc;					    	// dlc 数据的长�?

	for(i=0;i<8;i++)
	  TxMessage.Data[i]=msg[i];	
    		          
	mbox= CAN_Transmit(CAN1, &TxMessage);   
	i=0;
	while((CAN_TransmitStatus(CAN1, mbox)!=CANTXOK)&&(i<0XFFF))	i++;	//等待发送结�?
	if(i>=0XFFF)	return 1;
	return 0;	
}

/*
 * 函数名：CAN1_RX0_IRQHandler
 * 描述  ：CAN1中断服务函数
 * 输入  ：无
 * 输出  :  	 
 * 调用  �?
 */
static CanRxMsg RxMessage;
void CAN1_RX0_IRQHandler(void)
{    
    CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);
    if(RxMessage.StdId == 0x01)  /* 读取数据 */
    {
      if(RxMessage.IDE == CAN_ID_STD && RxMessage.RTR == CAN_RTR_DATA)
        {
          for(u8 i=0;i<RxMessage.DLC;i++)
            {
             canrxbuf1[i] = RxMessage.Data[i];

            }
            Encoder_OnCanFrame(RxMessage.StdId, canrxbuf1, RxMessage.DLC);
        }
    }
	CAN_ClearITPendingBit(CAN1,CAN_IT_FMP0);  /*清除挂起中断 */
}

#if 0
/*
 * 函数名：CAN1_RX0_IRQHandler
 * 描述  ：CAN1中断服务函数
 * 输入  ：无
 * 输出  :  	 
 * 调用  �?
 */
static CanRxMsg RxMessage;
void CAN1_RX0_IRQHandler(void)
{	
    u8 i;

    RxMessage.StdId=0x00;
    RxMessage.ExtId=0x00;
    RxMessage.IDE=0;
    RxMessage.DLC=0;
    RxMessage.FMI=0;
	
    for(i=0;i<8;i++)
		RxMessage.Data[i]=0x00;
    
    CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);
		CAN_ClearITPendingBit(CAN1,CAN_IT_FMP0);  /*清除挂起中断 */
	
//    CAN1_RxProce();

    USART_printf( USART2, (uchar *)"\r\n Rx CAN1" );
    USART_printf( USART2, (uchar *)"\r\n Id %d \r",(RxMessage.StdId<<18 | RxMessage.ExtId) );
    //标准�?
    USART_printf( USART2, (uchar *)"\r\n Standard Id" );    		
    USART_printf( USART2, (uchar *)" %h", RxMessage.StdId >> 8 & 0x00ff );    
    USART_printf( USART2, (uchar *)"%hh ", RxMessage.StdId & 0x00ff );  
		//扩展�?
    USART_printf( USART2, (uchar *)"\r\n Extended Id %h ", RxMessage.ExtId >> 24 & 0x00ff );    
    USART_printf( USART2, (uchar *)"%h ", RxMessage.ExtId >> 16 & 0x00ff );    
    USART_printf( USART2, (uchar *)"%h ", RxMessage.ExtId >> 8 & 0x00ff );    
    USART_printf( USART2, (uchar *)"%hh\r\n", RxMessage.ExtId & 0x00ff );  
    for(i=0;i<8;i++)
        USART_printf( USART2,(uchar *)"%h ", RxMessage.Data[i] );  
    USART_printf( USART2,(uchar *)"\r\n");  
}
#endif


/* 函数名：CAN1_RxProce
 * 描述  ：CAN1接收报文处理函数  
 * 输入  ：无
 * 输出  : �? 
 * 调用  ：外部调�?
 */
void CAN1_RxProce(void)
{

}


/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/
