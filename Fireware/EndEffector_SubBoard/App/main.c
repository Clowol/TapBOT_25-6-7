/******************** (C) COPYRIGHT 2026 *****************************************
 * @file    main.c
 * @brief   STM32F103 end-effector sub-board application entry.
 * @note        
 * @warning     
 * @license     This project is released under the MIT License.
 *********************************************************************************/
#include "stm32f10x.h"

#include "app_config.h"

#include "delay.h"
#include "gpio.h"
#include "usart.h"
#include "rs485_link.h"
#include "end_effector_task.h"



int main(void)
{
    Delay_Init(); 

    // Initialise the GPIOs associated with the end effector
    EndGpio_Init();             

    SubUart_Init(APP_SUBBOARD_UART_BAUD);   

    Rs485Link_Init();

    EndEffectorTask_Init();

    while(1)
    {
        /* 1.Handling RS485 communication*/
        Rs485Link_Proc();

        /* 2. Control of end-effectors */
        EndEffectorTask_Proc(APP_CONTROL_PERIOD_MS);

        /* 3.Fixed cycle period */
        Delay_ms(APP_CONTROL_PERIOD_MS);
    }
}
