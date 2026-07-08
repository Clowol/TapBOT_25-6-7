/******************** (C) COPYRIGHT 2026 *****************************************
 * @file        main.c
 * @brief       System entry point: hardware init, main loop with timer-driven scheduler.
 * 
 * @note        
 * @warning     
 * @license     This project is released under the MIT License.
 *********************************************************************************/
#include "stm32f10x.h"
#include "function.h"

/***************************************************************************************************
 * @name   main()
 * @brief  System entry point.
 * @param  None
 * @return Integer exit code (never returns under normal operation).
 * @note   Init order: SystemInit -> USART -> CAN -> BSP -> actuators/control -> Timer -> main loop.
 ***************************************************************************************************/
int main()
{
    static u16 cnt1s = 0U;
    static u16 cnt30ms = 0U;
    static u16 cnt50ms = 0U;
    static u16 cnt100ms = 0U;
    static u16 cnt200ms = 0U;
    static u16 cnt300ms = 0U;
    static u16 cnt500ms = 0U;

    /********************************** System & Core Init ********************************************/
    SystemInit();
    delay_init(72);

    LED_Init();
    NVIC_Configuration();

    /********************************** Communication Peripherals Init ********************************************/
    USART1_Init(APP_USART1_BAUD);       // USART1:4000000 & YuShuMotor & 485
    USART2_Init(APP_USART2_BAUD);       // USART2:115200  &	UpControl & 232
    USART3_Init(APP_USART3_BAUD);       // USART3:115200 & ReMote
    USART4_Init(APP_UART4_BAUD);        // UART4: 9600 & PTZ & 485
    SubBoard_LinkInit();
    USART5_Init(APP_UART5_BAUD);        // UART5: 115200 & deputy & 485

    USER_CAN1_Init();                  
    USER_CAN2_Init();

    /********************************** BSP / Board-Level Init ********************************************/
    Switch_Init();                        // I/O INIT()
    /* ADC1_Init(); */

    /********************************** Application Module Init ********************************************/
    ControlDispatcher_Init();             // UP_control

    Timer3_Init(10, 7199);                // 1ms Scheduled
    Timer4_Init(500, 7199);               // 50ms Scheduled

    SendSteer_SYNC_SetDefaultMode();      // Motor Syn Mode set_parameter
    Send_PTZ_Data();
    Encoder_Init();
    WitImu_Init();

#if APP_USART2_TEXT_DEBUG
    swgPrtUx(USART2, "\r\n Init Ok \r\n");
#endif

    while(1)
    {
        /********************************** Protocol Frame Processing ********************************************/
        ROS2_CommProc();
        FunctionProce();

        /********************************** 10ms Timer-tick Scheduler **************************************/
        if(flag10ms)
        {
            flag10ms = 0U;
            Function_10ms();

            if(++cnt30ms >= 3U)
            {
                cnt30ms = 0U;
                Function_30ms();
            }

            if(++cnt50ms >= 5U)
            {
                cnt50ms = 0U;
                Function_50ms();
            }

            if(++cnt100ms >= 10U)
            {
                cnt100ms = 0U;
                Function_100ms();
            }

            if(++cnt200ms >= 20U)
            {
                cnt200ms = 0U;
                Function_200ms();
            }

            if(++cnt300ms >= 30U)
            {
                cnt300ms = 0U;
                Function_300ms();
            }

            if(++cnt500ms >= 50U)
            {
                cnt500ms = 0U;
                Function_500ms();
            }

            if(++cnt1s >= 100U)
            {
                cnt1s = 0U;
                Function_1s();
            }
        }
    }
}




/******************* (C) COPYRIGHT 2026 END OF FILE *****************************/

