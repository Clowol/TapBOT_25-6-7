/******************** (C) COPYRIGHT 2026 *****************************************
   * @author      Clomol
   * @date        2026-2027
   * @brief       can2 接口的初始化
   *                -----------------
   *              |  PB5-CAN2-RX     |
   *              |  PB6-CAN2-TX     |
   *               -----------------     
   * @license     [z]本代码仅用于教学与科研目的，未经作者书面许可，不得用于商业用途
   *              This project is released under the MIT License.
   * @note        
   * @warning     
*********************************************************************************/
#include "can2.h"


/**************************       全局变量定义       ****************************/
// CAN1接收数据缓冲区（8字节）
static u8 canrxbuf2[CAN2_MAX_DLC] = {0};


/************************** 初始化报文 ****************************************/ 
// CAN2测试发送报文8（8字节） 
u8 CAN2_SendMsg1[CAN2_MAX_DLC]={0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01}; 


/******************************************************************************
 * @name 	USER_CAN2_Init(void)
 * @brief   CAN2初始化，包括端口初始化和中断优先级初始化
 * @param   无
 * @retval  无
 * @note    
 *          [外部调用]
 ******************************************************************************/
void USER_CAN2_Init(void)
{
	CAN2_NVIC_Config();
	CAN2_GPIO_Config();
	CAN2_Init_Config();
}


/********************************************************************************
 * @name 	CAN2_NVIC_Config(void)
 * @brief   CAN2 RX0 中断优先级配置
 * @param   无
 * @retval  无
 * @note    配置CAN2_RX0_IRQn为优先级组2，主优先级1，次优先级0
 *          [内部调用]
 ********************************************************************************/
void CAN2_NVIC_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    NVIC_InitStructure.NVIC_IRQChannel = CAN2_RX0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;     // 主优先级为1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;            // 次优先级为0
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStructure);
}


/********************************************************************************
 * @name 	CAN2_GPIO_Config(void)
 * @brief   CAN2 GPIO 和时钟配置
 * @param   无
 * @retval  无
 * @note    
 *          [内部调用]
 ********************************************************************************/
void CAN2_GPIO_Config(void)
{ 
    GPIO_InitTypeDef GPIO_InitStructure; 

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOB, ENABLE);

    /* CAN Periph clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, ENABLE);

    /***********Configure CAN2 pin ************/									
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;           		// PB5  CAN2 RX
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;           	// 上拉输入
    GPIO_Init(GPIOB, &GPIO_InitStructure); 
                                        
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;                	// PB6  CAN2 TX
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;       
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;          	//复用推挽输出  ！！
    GPIO_Init(GPIOB, &GPIO_InitStructure);  
    
    GPIO_PinRemapConfig(GPIO_Remap_CAN2,ENABLE);
}


/********************************************************************************
 * @name 	CAN2_Init_Config(void)
 * @brief   CAN2 参数初始化
 * @param   无
 * @retval  无
 * @note    
 *          [外部调用]
 ********************************************************************************/
void CAN2_Init_Config(void)
{
    CAN_InitTypeDef        CAN_InitStructure;

    CAN_DeInit(CAN2);
    /* CAN register init */
    CAN_StructInit(&CAN_InitStructure);

    /* CAN cell init */
    CAN_InitStructure.CAN_TTCM=DISABLE;           	// 禁用时间触发通信模式
    CAN_InitStructure.CAN_ABOM=DISABLE;	          	// 离线退出是在中断置位清0后退出
    CAN_InitStructure.CAN_AWUM=DISABLE;	          	// 自动唤醒模式：清零sleep
    CAN_InitStructure.CAN_NART=ENABLE;	          	// 自动重新传送，直到发送成功
    CAN_InitStructure.CAN_RFLM=DISABLE;	          	// FIFO没有锁定，新报文覆盖旧报文
    CAN_InitStructure.CAN_TXFP=DISABLE;             // 发送报文优先级确定：标志符
    CAN_InitStructure.CAN_Mode=CAN_Mode_Normal;     // 回环模式
    CAN_InitStructure.CAN_SJW=CAN_SJW_1tq;          // 1tq、BS1、BS2的值跟波特率有关
    CAN_InitStructure.CAN_BS1=CAN_BS1_8tq;          //  1     8      7     9
    CAN_InitStructure.CAN_BS2=CAN_BS2_7tq;
    CAN_InitStructure.CAN_Prescaler=18;			  	// 分频系数为18  36M/18/(1+8+7)=125k    36M/45/(1+8+7)=50k   36M/9/(1+5+2)=500k 

    CAN_Init(CAN2, &CAN_InitStructure);			  	// 初始化CAN2

    CAN2_Filter_Config();

    /* CAN FIFO0 message pending interrupt enable */        
    CAN_ITConfig(CAN2, CAN_IT_FMP0, ENABLE);        //CAN2中断使能,进入中断后读fifo的报文函数释放报文清中断标志
}


/********************************************************************************
 * @name 	CAN2_Filter_Config(void)
 * @brief   CAN2过滤器
 * @param   无
 * @retval  无
 * @note    
 *          [内部调用]
 ********************************************************************************/
void CAN2_Filter_Config(void)
{
	//标识符列表模式
	CAN_FilterInitTypeDef  CAN_FilterInitStructure;

//	CAN_FilterInit(&CAN_FilterInitStructure); //初始化CAN_FilterInitStructrue结构体变量
												//两个CAN使用的时候不要随便取消注释

	//CAN2过滤器设置         CAN2 要从过滤组14开始
	CAN_FilterInitStructure.CAN_FilterNumber    = 14;
	CAN_FilterInitStructure.CAN_FilterMode      = CAN_FilterMode_IdMask;
	CAN_FilterInitStructure.CAN_FilterScale     = CAN_FilterScale_32bit;
	CAN_FilterInitStructure.CAN_FilterIdHigh    = 0x0000;
	CAN_FilterInitStructure.CAN_FilterIdLow     = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh= 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment= CAN_FIFO0;
	CAN_FilterInitStructure.CAN_FilterActivation    = ENABLE;

	CAN_FilterInit(&CAN_FilterInitStructure);
}


/***************************************************************************************************
 * @name    Can2_Send_Msg()
 * @brief   使用can2发送一组数据
 * @param   msg：待发送数据缓冲区（长度≤8字节）
 *          id：标准帧ID（0~0x7FF）
 *          dlc：数据长度（0~8字节）
 * @retval 发送结果：0-成功，1-失败（超时）
 * @note   1. 仅支持标准数据帧发送
 *          2. 超时时间：0xFFF次循环，防止死等
 *        [外部调用]
 ***************************************************************************************************/
u8 Can2_Send_Msg(u8 *msg, u32 id, u8 dlc)
{
	u8 mbox;
	u8 timeout=0;
	CanTxMsg TxMessage;

	TxMessage.StdId = id;					// 标准标识符为id
	TxMessage.ExtId = id;					// 扩展帧ID
	TxMessage.IDE   = CAN_ID_STD;			// 标准帧
	TxMessage.RTR   = CAN_RTR_DATA;		   	// 数据帧
	TxMessage.DLC   = dlc;					// dlc 数据的长度

	for(u8 i=0; i<8;i++){
        TxMessage.Data[i]=msg[i];	
    }

	mbox= CAN_Transmit(CAN2, &TxMessage);   

	timeout=0;
	while((CAN_TransmitStatus(CAN2, mbox)!=CANTXOK)&&(timeout<0XFF))
    {
        timeout++;	//等待发送结束
    }

	if(timeout>=0XFF)	return 1;
	return 0;	
}


/**********************************************************************************************
 * 函数名：CAN2_RX0_IRQHandler
 * 描述  ：CAN2中断服务函数
 * 输入  ：无
 * 输出  :  	 
 * 调用  ：
 ***********************************************************************************************/
static CanRxMsg RxMessage;
void CAN2_RX0_IRQHandler(void)
{
    u8 i;
    
    CAN_Receive(CAN2, CAN_FIFO0, &RxMessage);
    if(RxMessage.StdId == 0x00 || RxMessage.StdId == 0x01)  /* 读取数据 */
    {
        if(RxMessage.IDE == CAN_ID_STD && RxMessage.RTR == CAN_RTR_DATA)
        {
            for(i=0;i<RxMessage.DLC;i++)
            {
                canrxbuf2[i] = RxMessage.Data[i];
            }
        }
    }
    
	CAN_ClearITPendingBit(CAN2,CAN_IT_FMP0);  /*清除挂起中断 */
    //CAN2_RxProce();
}

/***************************************************************************************************
 * @name    CAN2_RxProce(void)
 * @brief   CAN2接收报文处理函数
 * @param   无
 * @retval  无
 * @note     
 *        [外部调用]
 ***************************************************************************************************/
void CAN2_RxProce(void)
{
    // 预留接收数据处理逻辑
    // 示例：解析g_can2_rx_buf中的数据，更新g_can2_io_temp
}

/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/
