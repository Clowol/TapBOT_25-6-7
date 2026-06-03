/******************** (C) COPYRIGHT 2024 *****************************************
 * 文件�? ：can2.c
 * 描述    ：can2 接口的初始化
 * 硬件配置: -----------------
 *          |  PB5-CAN2-RX     |
 *          |  PB6-CAN2-TX     |
 *           -----------------      
 * 版本    �?
 * 修改日期�?                                                                                                                                                                                                                                                                                                                                                                                                                                  
 * 作�?   �?
 * 修改日志�?
*********************************************************************************/
#include "can2.h"
//#include "PTZ_Servo.h"
#include "function.h"
#include "wit_imu.h"

/************************** 初始化报�?****************************************/    
u8 CAN2_SendMsg1[8]={0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02};   /* CAN1 TEST Msg */  
u8 canrxbuf2[8];

//typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;


///* 在中断处理函数中返回 */
//__IO uint32_t ret = 0;

//volatile TestStatus TestRx;	

/*
 * 函数名：USER_CAN2_Init
 * 描述  ：CAN2初始化，包括端口初始化和中断优先级初始化
 * 输入  ：无
 * 输出  : �? 
 * 调用  ：外部调�?
 */
void USER_CAN2_Init(void)
{
	CAN2_NVIC_Configuration();
	CAN2_GPIO_Config();
	CAN2_Init_Config();
}


/*
 * 函数名：CAN2_NVIC_Configuration
 * 描述  ：CAN2 RX0 中断优先级配�?
 * 输入  ：无
 * 输出  : �?
 * 调用  ：内部调�?
 */
void CAN2_NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

  /* Enable CAN2 RX0 interrupt IRQ channel */
  NVIC_InitStructure.NVIC_IRQChannel = CAN2_RX0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;     // 主优先级�?
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;            // 次优先级�?
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}


/*
 * 函数名：CAN2_GPIO_Config
 * 描述  ：CAN2 GPIO 和时钟配�?
 * 输入  ：无
 * 输出  : �?
 * 调用  ：内部调�?
 */
void CAN2_GPIO_Config(void)
{ 
  GPIO_InitTypeDef GPIO_InitStructure; 
  	 
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOB, ENABLE);

  /* CAN Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, ENABLE);

  /***********Configure CAN2 pin ************/									
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;           			// PB5  CAN2 RX
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;           	// 上拉输入
  GPIO_Init(GPIOB, &GPIO_InitStructure); 
									
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;                	// PB6  CAN2 TX
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;       
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;          	//复用推挽输出  ！！
  GPIO_Init(GPIOB, &GPIO_InitStructure);  
  
  //#define GPIO_Remap2_CAN    GPIO_Remap2_CAN
  GPIO_PinRemapConfig(GPIO_Remap_CAN2,ENABLE);
}


/*
 * 函数名：CAN2_Init_Config
 * 描述  ：CAN2 参数初始�?
 * 输入  ：无
 * 输出  : �? 
 * 调用  ：外部调�?
 */
void CAN2_Init_Config(void)
{
  CAN_InitTypeDef        CAN_InitStructure;

  CAN_DeInit(CAN2);
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
  CAN_InitStructure.CAN_BS1=CAN_BS1_8tq;          //  1     8      7     9
  CAN_InitStructure.CAN_BS2=CAN_BS2_7tq;
  CAN_InitStructure.CAN_Prescaler=18;			  			// 分频系数�?8  36M/18/(1+8+7)=125k    36M/45/(1+8+7)=50k   36M/9/(1+5+2)=500k 

  CAN_Init(CAN2, &CAN_InitStructure);			  			// 初始化CAN2

  CAN2_Filter_Config();

  /* CAN FIFO0 message pending interrupt enable */        
  CAN_ITConfig(CAN2, CAN_IT_FMP0, ENABLE);        //CAN2中断使能,进入中断后读fifo的报文函数释放报文清中断标志
}


/*
 * 函数名：CAN2_Filter_Config
 * 描述  ：CAN2过滤�?
 * 输入  ：无
 * 输出  : �? 
 * 调用  �?       
 */
void CAN2_Filter_Config(void)
{
	//标识符列表模�?
	CAN_FilterInitTypeDef  CAN_FilterInitStructure;

//	CAN_FilterInit(&CAN_FilterInitStructure); //初始化CAN_FilterInitStructrue结构体变�?
																							//两个CAN使用的时候不要随便取消注�?

	//CAN2过滤器设�?        CAN2 要从过滤�?4开�?
	CAN_FilterInitStructure.CAN_FilterNumber=14;
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
 * 函数名：Can2_Send_Msg
 * 描述  ：使用can2发送一组数�?
 *         msg为发送的数据指针
 *         发送数据为标准 数据�?
 *         dlc 数据长度
 * 输入  ：无
 * 输出  :  	 
 * 调用  ：外部调�?
 */
u8 Can2_Send_Msg(u8 *msg, u32 id, u8 dlc)
{
	u8 mbox;
	u8 i=0;
	CanTxMsg TxMessage;
	TxMessage.StdId=id;					 			// 标准标识符为id
	TxMessage.ExtId=id;								// 扩展帧ID
	TxMessage.IDE=CAN_ID_STD;					// 标准�?
	TxMessage.RTR=CAN_RTR_DATA;		   	// 数据�?
	TxMessage.DLC=dlc;					    	// dlc 数据的长�?

	for(i=0;i<8;i++)
	  TxMessage.Data[i]=msg[i];	
    		          
	mbox= CAN_Transmit(CAN2, &TxMessage);   
	i=0;
	while((CAN_TransmitStatus(CAN2, mbox)!=CANTXOK)&&(i<0XFF))	i++;	//等待发送结�?
	if(i>=0XFF)	return 1;
	return 0;	
}


/*
 * 函数名：CAN2_RX0_IRQHandler
 * 描述  ：CAN2中断服务函数
 * 输入  ：无
 * 输出  :  	 
 * 调用  �?
 */
//static CanRxMsg RxMessage;
//void CAN2_Process(void)
//{
//    u8 i;
//    
//    CAN_Receive(CAN2, CAN_FIFO0, &RxMessage);
//    if(RxMessage.IDE == CAN_ID_STD && RxMessage.RTR == CAN_RTR_DATA)  /* 读取数据 */
//    {
//      if(RxMessage.StdId == 0x00 || 0x01)
//        {
//          for(i=0;i<RxMessage.DLC;i++)
//              RxMessage.Data[i]= canrxbuf2[i];
//        }
//      Move_Cmd(canrxbuf2);
//    }
//}
//
/*
 * 函数名：CAN2_RX0_IRQHandler
 * 描述  ：CAN2中断服务函数
 * 输入  ：无
 * 输出  :  	 
 * 调用  �?
 */
 CanRxMsg RxMessage;
void CAN2_RX0_IRQHandler(void)
{
    u8 i;

    CAN_Receive(CAN2, CAN_FIFO0, &RxMessage);
    if((RxMessage.IDE == CAN_ID_STD) && (RxMessage.RTR == CAN_RTR_DATA))
    {
        for(i = 0U; (i < RxMessage.DLC) && (i < 8U); i++)
        {
            canrxbuf2[i] = RxMessage.Data[i];
        }

        if(RxMessage.DLC >= 8U)
        {
            WitImu_OnCanFrame(RxMessage.StdId, canrxbuf2, RxMessage.DLC);
        }

        CntRxCAN2 = 80U;
        CAN2_OutCommunFlg = 0U;
    }

    CAN_ClearITPendingBit(CAN2, CAN_IT_FMP0);
}

#if 0
/*
 * 函数名：CAN2_Rx_Cam_Proce
 * 描述  ：CAN2接收报文处理函数
 * 输入  ：无
 * 输出  :  	 
 * 调用  ：外部调�?
 */
void CAN2_Rx_Cam_Proce(void)   
{
    
    if(canrxbuf[0] == 0xAA){
        for(u8 i = 0; i < 3; i++)
        {
            pos_rebuilt_array[i] = (s16)((s16)canrxbuf[2*i+2]<<8)| canrxbuf[2*i];
        }
    }else if(canrxbuf[0] == 0xCC){
        for(u8 i = 0; i < 3; i++)
        {
            rot_rebuilt_array[i] = (s16)((s16)canrxbuf[2*i+2]<<8)| canrxbuf[2*i];
        }
    }else if(canrxbuf[0] == 0xEE){
        for(u8 i = 0; i < 3; i++)
        {
            err_rebuilt_array[i] = (s16)((s16)canrxbuf[2*i+2]<<8)| canrxbuf[2*i];
        }
    }
}
#endif


/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/


