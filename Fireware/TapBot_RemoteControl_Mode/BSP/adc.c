/******************** (C) COPYRIGHT 2026 *****************************************
 * @file        adc.c
 * @brief       ADC1 DMA-based multi-channel readout functions.
 * @note        
 * @warning     
 * @license     This project is released under the MIT License. 
 *********************************************************************************/
/**
 *  Hardware specifications：------------------------------ totak channels ---
 *            ADC1	STM32_AD1： PA0   ADC_IN0		  重标：J46		AD2
 *        后面要改	 STM32_AD2： PA2   ADC_IN2				 J50		AD1
 *               	  STM32_AD3： PA3   ADC_IN3				  J54		AD8
 *               	  STM32_AD4： PA6   ADC_IN6				  J56		AD7
 *               	  STM32_AD5： PA7   ADC_IN7				  J47		AD10
 *               	  STM32_AD6： PB0   ADC_IN8				  J51		AD9
 *               	  STM32_AD7： PB1   ADC_IN9				  J55		AD4
 *                	STM32_AD8： PC0   ADC_IN10				J57		AD3
 *               	  STM32_AD9： PC1   ADC_IN11				J48		AD6
 *               	  STM32_AD10：PC2   ADC_IN12				J52		AD5
 * 
 *  电阻测量 0-190欧	  AIN1		PC3		ADC_IN13	      J49
 *					           AIN2		 PC4		ADC_IN14	      J53
 *              ---------------------------------------------------------
 */ 

#include "adc.h"


/*************************** global variables *******************************/
volatile u16 ADC_ConvertedValue[SCAN_TIMES][CHANNEL_NUM];
volatile u16 ADC_FilteredValue[CHANNEL_NUM];



/*************************** Function definition *******************************/

/**********************************************************************************************
 * @name 	  ADC1_GPIO_Config(void)
 * @brief   Set the GPIO mode of the pins required for ADC1 to analogue input
 * @param   无
 * @retval  无
 * @note    Enable the clocks for ADC1 and DMA1, and initialise PA, PB and PC
 *          -> Analogue input mode（AIN）
 *          [Internal call]
 **********************************************************************************************/
static void ADC1_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable ADC1 and GPIOA GPIOB GPIOC clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA   \
	| RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);

  /* Configure PA PB  PC as analog input */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_6|GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOA, &GPIO_InitStructure);				// no need to set the baud rate when entering data
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOB, &GPIO_InitStructure);				// no need to set the baud rate when entering data

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOC, &GPIO_InitStructure);				// no need to set the baud rate when entering data
}



/**********************************************************************************************
 * @name 	  ADC1_Mode_Config(void)
 * @brief   Configure the ADC1’s operating mode, rule channel sequence, sampling time and DMA enable
 * @param   无   
 * DC1,ADC通道x,规则采样顺序值为y,采样时间为239.5周期
 * [  12  channels  ]
 * ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5 );
 * ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 2, ADC_SampleTime_239Cycles5 );
 * ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 3, ADC_SampleTime_239Cycles5 );
 * ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 4, ADC_SampleTime_239Cycles5 );
 * ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 5, ADC_SampleTime_239Cycles5 );
 * ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 6, ADC_SampleTime_239Cycles5 );
 * ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 7, ADC_SampleTime_239Cycles5 );
 * ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 8, ADC_SampleTime_239Cycles5 );
 * ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 9, ADC_SampleTime_239Cycles5 );
 * ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 10, ADC_SampleTime_239Cycles5 );
 * ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 11, ADC_SampleTime_239Cycles5 );
 * ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 12, ADC_SampleTime_239Cycles5 );
 * @retval  无
 * @note    配置ADC1  3.3V  4096  12位； 采样时间239.5个周期
 *          [ Internal call ]
 **********************************************************************************************/
static void ADC1_Mode_Config(void)
{
	ADC_InitTypeDef ADC_InitStructure;
  
  ADC_DeInit(ADC1);                               // 复位ADC1寄存器至默认值
    
  /* ADC1 configuration */
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;      // 独立模式
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;            // 扫描模式
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;      // 连续转换模式
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;   // 外部触发转换关闭
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;  // ADC数据右对齐
  ADC_InitStructure.ADC_NbrOfChannel = CHANNEL_NUM;       // 顺序进行规则转换的ADC通道的数目
  ADC_Init(ADC1, &ADC_InitStructure);                     // 根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器

  /* 配置规则通道 */
  // 设置指定ADC的规则组通道，设置它们的转化顺序和采样时间
  ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 1, ADC_SampleTime_239Cycles5 );
  ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 2, ADC_SampleTime_239Cycles5 );
  ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 3, ADC_SampleTime_239Cycles5 );
  ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 4, ADC_SampleTime_239Cycles5 );

  /* 使能ADC1 DMA功能 */
  ADC_DMACmd(ADC1, ENABLE);    // 开启ADC的DMA支持（要实现DMA功能，还需独立配置DMA通道等参数）
  
  /* 使能ADC1并执行校准 */
  ADC_Cmd(ADC1, ENABLE);     

  ADC_ResetCalibration(ADC1);                     // 复位校准寄存器
  while(ADC_GetResetCalibrationStatus(ADC1));     // 获取ADC1复位校准寄存器的状态,设置状态则等待
  ADC_StartCalibration(ADC1);                     // 开始指定ADC1的校准状态
  while(ADC_GetCalibrationStatus(ADC1));          // 获取指定ADC1的校准程序,设置状态则等待
}



/**********************************************************************************************
 * @name 	  DMA_ADC1_Configuration(void)
 * @brief   Configure DMA1 channel 1 to automatically transfer ADC1 data to memory
 * @param   无
 * @retval  无
 * @note    配置ADC1的DMA通道，用于自动采集ADC数据
 *          [内部调用]
 **********************************************************************************************/
static void DMA_ADC1_Configuration(void)
{
	DMA_InitTypeDef DMA_InitStructure;

	/* 使能DMA1时钟 */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	/* DMA channel1 configuration */
  DMA_DeInit(DMA1_Channel1);     // 将DMA的通道1寄存器重设为缺省值

  /* DMA核心配置 */
  DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&ADC1->DR;         // 外设基地址（ADC1数据寄存器）
  DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&ADC_ConvertedValue;   // 内存基地址（原始采样值）
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;                 // 外设→内存
  DMA_InitStructure.DMA_BufferSize = CHANNEL_NUM * SCAN_TIMES;       // 缓存大小（4*20=80）
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;   // 外设地址不递增
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;            // 内存地址递增
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;     //数据宽度为16位
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;             //数据宽度为16位 
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                    // 循环模式（持续采集）
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;                // DMA通道x 拥有高优先级
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                       // DMA通道x 禁止内存→内存

  DMA_Init(DMA1_Channel1, &DMA_InitStructure);                       //根据DMA_InitStruct中指定的参数初始化DMA的通道
}



/**********************************************************************************************
 * @name 	  ADC1_Init(void)
 * @brief   初始化ADC1
 * @param   无
 * @retval  无
 * @note    初始化ADC1，包括GPIO配置、模式配置和DMA配置
 *          [外部调用]
 **********************************************************************************************/
void ADC1_Init(void)
{
  ADC1_GPIO_Config();
  ADC1_Mode_Config();
  DMA_ADC1_Configuration();
	
  /* 启动ADC软件转换 + DMA通道 */
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);     // 启动AD1转换  
  DMA_Cmd(DMA1_Channel1, ENABLE);             // 启动DMA通道  
}


/*============================= Physical quantity conversion function  ================*/
/**********************************************************************************************
 * @name 	  GetVolt(u16 advalue)
 * @brief   获得电压值
 * @param   advalue ADC采集值
 * @retval  电压值
 * @note    采集值经过运算放大器变化0-5V范围内，方便下面求出小数
 *          [外部调用]
 **********************************************************************************************/
float GetVolt(u16 advalue)  
{
  return (5.0 * advalue / 4096); 
  //采集值经过运算放大器变化0-5V范围内，方便下面求出小数
}



/**********************************************************************************************
 * @name 	  GetRes(u16 advalue)
 * @brief   获得电阻值 0-190欧
 * @param   advalue ADC采集值
 * @retval  电阻值
 * @note    采集值经过运算放大器变化0-5V范围内，方便下面求出小数
 *          [外部调用]
 **********************************************************************************************/
float GetRes(u16 advalue)  
{
  return (5.0 * advalue / 4096); //
}

/**********************************************************************************************
 * @name 	  ADC1_GetCurrent(u16 adc_value) 
 * @brief   ADC值转换为电流值（4-20mA）
 * @param   adc_value
 * @retval  电流值（单位：mA）
 * @note    电流 = ((ADC值 - 最小ADC值) / ADC范围) * 电流范围 + 最小电流
 *          [外部-调用]
 **********************************************************************************************/
float GetCurrent(u16 advalue)  
{
  return ((float)(advalue-750)/(3735-750)*16+4); //
}


/*============================= Software filter function  ================*/
/**********************************************************************************************
 * @name 	  AD_filter(void)
 * @brief   ADC数据均值滤波函数
 * @param   无
 * @retval  无
 * @note    
 **********************************************************************************************/
void AD_filter(void)
{
  int sum = 0;
  u8 sample_idx,i;

  for(i=0;i<CHANNEL_NUM;i++)
  { 
    for(sample_idx=0;sample_idx<SCAN_TIMES;sample_idx++)
    {                       
      sum += ADC_ConvertedValue[sample_idx][i];  
    }  

    ADC_FilteredValue[i]=sum/SCAN_TIMES;   
    sum=0;
  }   
}


/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/
