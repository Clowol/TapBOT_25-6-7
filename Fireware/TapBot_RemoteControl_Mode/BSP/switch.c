/******************** (C) COPYRIGHT 2026 *****************************************
   * @author      Clomol
   * @date        2026-2027
   * @brief       
   * @license     [z]本代码仅用于教学与科研目的，未经作者书面许可，不得用于商业用途
   *              This project is released under the MIT License.
   * @note        
   * @warning     
*********************************************************************************/
#include "switch.h"

/***************************************************************************************************
 * @name	 Switch_Init(void)
 * @brief  开关量输入输出用到的I/O口定义
 * @param     
 *		SW1  SW2  SW3  SW4  	SW5  SW6  SW7  	SW8	 SW9  	SW10  SW11  SW12  SW13  SW14  	SW15  SW16
 *		PD12 PD13 PD14 PD15 	PC6  PC8  PC9  	PE13 PE14 	PE2   PE3   PE4   PE5   PE6   	PA11  PA12
 * @else	
 *		IN1  	IN2  IN3  IN4  	IN5  	IN6  	IN7  	IN8	 	IN9  IN10  	IN11  IN12
 *		PE11 	PB3  PB4  PB7  	PA1  	PE9  	PA15 	PC7   	PD10 PD11  	PD7   PD4
 *
 * @note 	SW1~SW16 对应输出引脚; IN1~IN12 对应输入引脚
 ***************************************************************************************************/
void Switch_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
                                    
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|  \
	RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOE, ENABLE);
	
	/********************************** I/O输出 ********************************************/
	// 配置PA11、PA12为推挽输出
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11|GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		         //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_ResetBits(GPIOA,GPIO_Pin_11|GPIO_Pin_12);              // 初始化为低电平（关闭状态）
	
	// 配置PC6、PC8、PC9为推挽输出
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_8|GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		    
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	GPIO_ResetBits(GPIOC,GPIO_Pin_6|GPIO_Pin_8|GPIO_Pin_9);
	
	// 配置PD12、PD13、PD14、PD15为推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		      //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	GPIO_ResetBits(GPIOD,GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);
	
	// 配置PE2、PE3、PE4、PE5、PE6、PE13、PE14为推挽输出
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5| \
	                            GPIO_Pin_6|GPIO_Pin_13|GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		        //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	GPIO_ResetBits(GPIOE, GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5| \
	                    GPIO_Pin_6|GPIO_Pin_13|GPIO_Pin_14);

/***************************************************************************************************
 * @brief  开关量输入输出用到的I/O口定义
 * @param     
 *		IN1  	IN2  IN3  IN4  	IN5  	IN6  	IN7  	IN8	 	IN9  IN10  	IN11  IN12
 *		PE11 	PB3  PB4  PB7  	PA1  	PE9  	PA15 	PC7   PD10 PD11  	PD7   PD4
 *
 * @note 	IN1~IN12 对应输入引脚
 ***************************************************************************************************/
	// 设置PA1、PA15为上拉输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_15;	  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		    	//上拉输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// 调试接口信号被映射到GPIO端口 -> 禁用JTAG功能，释放PB3、PB4等引脚
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);      // 使能复用功能模块时钟 
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable , ENABLE); 

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		    	//上拉输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	//PC
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		    	//上拉输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	//PD
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_7|GPIO_Pin_10|GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		    	//上拉输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	//PE
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9|GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		    	//上拉输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	
	//继电器输出初始化
	SW_CLUTCH(OFF);

}



/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/
