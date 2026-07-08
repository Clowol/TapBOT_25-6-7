/******************** (C) COPYRIGHT 2026 *****************************************
 * @file        can1.c
 * @brief       CAN1 interface initialization and RX interrupt (PD0/PD1).
 *********************************************************************************/
#include "can1.h"
#include "encoder.h"


/**************************       全局变量定义       ****************************/
// CAN1接收数据缓冲区（8字节）
u8 canrxbuf1[CAN1_MAX_DLC] = {0};

// 预留IO临时变量（标注未使用，便于后续扩展）
static s16 io_temp[4] = {0};

/************************** 初始化报文 ****************************************/   
// CAN1测试发送报文8（8字节） 
u8 CAN1_SendMsg1[CAN1_MAX_DLC]={0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01}; 
// CAN1测试发送报文2（2字节）
u8 CAN1_SendMsg2[CAN1_MIN_DLC]={0x01,0x01};    


/* typedef enum {
    CAN_STATE_IDLE = 0,    // 空闲
    CAN_STATE_DATA_RECV,   // 已接收位置/转速数据
    CAN_STATE_DATA_CONVERT,// 数据转换完成
    CAN_STATE_SEND_OK,     // 串口DMA发送完成
    CAN_STATE_ERROR        // 通信错误
} CAN_Work_State;

volatile CAN_Work_State g_can_work_state = CAN_STATE_IDLE; // 全局业务状态 */


/********************************************************************************
 * @name 	USER_CAN1_Init(void)
 * @brief   CAN1初始化，包括端口初始化和中断优先级初始化
 * @param   无
 * @retval  无
 * @note    
 *          [外部调用]
 ********************************************************************************/
void USER_CAN1_Init(void)
{
	CAN1_NVIC_Config();
	CAN1_GPIO_Config();
	CAN1_Init_Config();
}


/********************************************************************************
 * @name 	CAN1_NVIC_Configuration(void)
 * @brief   CAN1 RX0 中断优先级配置
 * @param   无
 * @retval  无
 * @note    配置CAN1_RX0_IRQn为优先级组2，主优先级0，次优先级1
 *          [内部调用]
 ********************************************************************************/
void CAN1_NVIC_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;     // 主优先级为0
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;            // 次优先级为1
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStructure);
}

/********************************************************************************
 * @name 	CAN1_GPIO_Config(void)
 * @brief   CAN1 GPIO 和时钟配置
 * @param   无
 * @retval  无
 * @note    
 *          [内部调用]
 ********************************************************************************/
void CAN1_GPIO_Config(void)
{ 
    GPIO_InitTypeDef GPIO_InitStructure; 

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOD, ENABLE);	                        											 
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
	
    // 配置PD0（CAN1_RX）为上拉输入						
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	      //上拉输入
	GPIO_Init(GPIOD, &GPIO_InitStructure);
    
    // 配置PD1（CAN1_TX）为复用推挽输出，50MHz								
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	    //复用推挽
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_PinRemapConfig(GPIO_Remap2_CAN1, ENABLE);  
}


/********************************************************************************
 * @name 	CAN1_Init_Config(void)
 * @brief   CAN1 参数初始化
 * @param   无
 * @retval  无
 * @note    
 *          [外部调用]
 ********************************************************************************/
void CAN1_Init_Config(void)
{
    CAN_InitTypeDef        CAN_InitStructure;

    CAN_DeInit(CAN1);

    CAN_StructInit(&CAN_InitStructure);

    CAN_InitStructure.CAN_TTCM = DISABLE;           	// 禁用时间触发通信模式
    CAN_InitStructure.CAN_ABOM = DISABLE;	          	// 禁用自动离线管理（离线退出是在中断置位清0后退出）
    CAN_InitStructure.CAN_AWUM = DISABLE;	          	// 禁用自动唤醒模式（自动唤醒模式：清零sleep）
    CAN_InitStructure.CAN_NART = ENABLE;	          	// 启用自动重传（直到发送成功）
    CAN_InitStructure.CAN_RFLM = DISABLE;	          	// 禁用FIFO锁定（新报文覆盖旧报文）
    CAN_InitStructure.CAN_TXFP = DISABLE;             // 发送报文优先级确定：标志符
    CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;     // 正常模式
    CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;          // 1tq、BS1、BS2的值跟波特率有关
    CAN_InitStructure.CAN_BS1 = CAN_BS1_9tq;          //  1     5      2     9
    CAN_InitStructure.CAN_BS2 = CAN_BS2_8tq;
    CAN_InitStructure.CAN_Prescaler = 20;			  			// 分频系数为20 36M/20/(1+9+8)=100k    36M/9/(1+8+7)=250k    36M/45/(1+8+7)=50k   36M/9/(1+5+2)=500k 

    CAN_Init(CAN1, &CAN_InitStructure);			  			  // 初始化CAN1

    CAN1_Filter_Config();
    
    CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE); 		      // CAN1中断使能,进入中断后读fifo的报文函数释放报文清中断标志
}


/********************************************************************************
 * @name 	CAN1_Filter_Config(void)
 * @brief   CAN1过滤器
 * @param   无
 * @retval  无
 * @note    
 *          [内部调用]
 ********************************************************************************/
void CAN1_Filter_Config(void)
{
	CAN_FilterInitTypeDef  CAN_FilterInitStructure;

	CAN_FilterInit(&CAN_FilterInitStructure);       //初始化CAN_FilterInitStructrue结构体变量
													//两个CAN使用的时候不要随便取消注释

	//CAN1过滤器设置
	CAN_FilterInitStructure.CAN_FilterNumber  = 0;                      // 使用滤波器0
	CAN_FilterInitStructure.CAN_FilterMode    = CAN_FilterMode_IdMask;  // 掩码模式
	CAN_FilterInitStructure.CAN_FilterScale   = CAN_FilterScale_32bit;  // 32位宽度
	CAN_FilterInitStructure.CAN_FilterIdHigh  = 0x0000;                 // 标识符高16位     
	CAN_FilterInitStructure.CAN_FilterIdLow   = 0x0000;                 // 标识符低16位
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;              // 掩码高16位
	CAN_FilterInitStructure.CAN_FilterMaskIdLow   = 0x0000;             // 掩码低16位
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FIFO0;       // 报文过滤器0关联到FIFO0
	CAN_FilterInitStructure.CAN_FilterActivation     = ENABLE;

	CAN_FilterInit(&CAN_FilterInitStructure);
}


/***************************************************************************************************
 * @name    Can1_Send_Msg()
 * @brief   使用can1发送一组数据
 * @param   msg：待发送数据缓冲区（长度≤8字节）
 *          id：标准帧ID（0~0x7FF）
 *          dlc：数据长度（0~8字节）
 * @retval 发送结果：0-成功，1-失败（超时）
 * @note   1. 仅支持标准数据帧发送
 *          2. 超时时间：0xFFF次循环，防止死等
 *        [外部调用]
 ***************************************************************************************************/
u8 CAN1_Send_Msg(u8 *msg, u32 id, u8 dlc)
{
	u8  mbox;
	u16 timeout=0;
	CanTxMsg TxMessage;
	u8 i;

    if((msg == 0) || (dlc > CAN1_MAX_DLC))
    {
        return 1U;
    }

    TxMessage.StdId = id;					 		// 标准标识符为id
    TxMessage.ExtId = 0x00;						    // 扩展帧ID
    TxMessage.IDE   = CAN_ID_STD;				    // 标准帧
    TxMessage.RTR   = CAN_RTR_DATA;		            // 数据帧
    TxMessage.DLC   = dlc;					        // dlc 数据的长度

    // 拷贝发送数据
    for(i=0 ; i<dlc ; i++)
    {
        TxMessage.Data[i] = msg[i];	
    } 	

    mbox= CAN_Transmit(CAN1, &TxMessage); 

    timeout=0;

    while((CAN_TransmitStatus(CAN1, mbox) != CANTXOK) && (timeout < 0XFFF))	
    {
        timeout++;
    }	

    if(timeout>=0XFFF)	
    {
        return 1;
    }

	return 0;	
}



/**********************************************************************************************
 * 函数名：CAN1_RX0_IRQHandler
 * 描述  ：CAN1中断服务函数
 * 输入  ：无
 * 输出  :  	 
 * 调用  ：
 ***********************************************************************************************/
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
            Encoder_UpdateFromCanFrame(canrxbuf1, RxMessage.DLC);
#if CAN1_RX_TEXT_DEBUG
            USART2_DMA_send(canrxbuf1,RxMessage.DLC);
#endif
        }
    }

    // 清除挂起中断
	CntRxCAN1 = 80U;
    CAN1_OutCommunFlg = 0U;

    // 清除挂起中断
	CAN_ClearITPendingBit(CAN1,CAN_IT_FMP0); 
}


/***************************************************************************************************
 * @name    CAN1_RxProce(void)
 * @brief   CAN1接收报文处理函数
 * @param   无
 * @retval  无
 * @note     
 *        [外部调用]
 ***************************************************************************************************/
void CAN1_RxProce(void)
{
    // 预留接收数据处理逻辑
    // 示例：解析g_can1_rx_buf中的数据，更新g_can1_io_temp
}




/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/
