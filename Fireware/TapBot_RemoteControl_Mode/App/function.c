/******************** (C) COPYRIGHT 2026 *****************************************
 * @file        function.c
 * @brief       Application periodic task scheduler and communication proc wrappers.
 * @note        
 * @warning     
 * @license     This project is released under the MIT License.
 *********************************************************************************/
#include "function.h"

/********************************** Control Mode Global Variables ********************************************/
u8 g_RmtUpManCtrlMode = RMT_MODE;
u8 g_RmtUpManRealMode = RMT_MODE;

/********************************** Communication Send Buffers ********************************************/
u8 USART2_SendBuf[60] = {0};
u8 USART4_SendBuf[60] = {0};

/********************************** Motor Control Data Structures ********************************************/
MOTOR_send cmd;
MOTOR_recv data;

static u8 s_diag_feedback_pending = 0U;         // wait for diag information

/***************************************************************************************************
 * @name   FunctionProce(void)
 * @brief  Non-timed per-loop processing: USART1/2/4/5 receive handling 
 *         and remote-controller protocol parsing (USART3 via Rmt_CommProc). 
 *         Upper-computer protocol (USART2) is handled via ROS2_CommProc in main.c.
 * @param  None
 * @note   Called every main-loop iteration, outside the timer-tick scheduler.
 ***************************************************************************************************/
void FunctionProce(void)
{
    USART1_Proc();
    USART2_Proc();
    Rmt_CommProc();
    USART4_Proc();
    USART5_Proc();
}

/***************************************************************************************************
 * @name   Function_10ms(void)
 * @brief  10ms periodic tasks.
 * @param  None
 * @note   Updates the control-source dispatcher (remote vs upper-computer arbitration)
 *         and the sub-board link online timer.
 ***************************************************************************************************/
void Function_10ms(void)
{
    static u8 subboard_heartbeat_ticks = 0U;

    ControlDispatcher_Update();             // Confirm control dispatcher
    ArmAutoTask_Proc10ms();
    SubBoard_LinkTick10ms();                // Timeout check
    if(++subboard_heartbeat_ticks >= 50U)
    {
        subboard_heartbeat_ticks = 0U;
        SubBoardProtocol_SendHeartbeat();
    }
}

/***************************************************************************************************
 * @name   Function_30ms(void)
 * @brief  30ms periodic task: M8010 actuator control processing.
 * @param  None
 * @note   Executes YushuMotor_ControlProc() which handles speed mapping, clutch control,
 *         limit-switch protection, and CAN command transmission for the Unitree M8010 motor.
 ***************************************************************************************************/
void Function_30ms(void)
{
    YushuMotor_ControlProc();
}

/***************************************************************************************************
 * @name   Function_50ms(void)
 * @brief  50ms periodic tasks: PTZ control and upper-computer feedback.
 * @param  None
 * @note   Sends ARM_STATE (0x80) and ACTUATOR_ECHO (0x81) feedback frames to the
 *         upper computer, and processes PTZ gimbal control.
 ***************************************************************************************************/
void Function_50ms(void)
{
    static u8 feedback_slot = 0U;

    PTZ_ControlProc();
    if(s_diag_feedback_pending != 0U)
    {
        s_diag_feedback_pending = 0U;
        UpperFeedback_SendDiag();
    }
    else if(feedback_slot == 0U)
    {
        UpperFeedback_SendArmState();
    }
    else
    {
        UpperFeedback_SendActuatorEcho();
    }
    feedback_slot ^= 1U;
}

/***************************************************************************************************
 * @name   Function_100ms(void)
 * @brief  100ms periodic task: SMS servo control (position servos ID0-ID2, speed servo ID3).
 * @param  None
 * @note   Only active in RMT_MODE. Calls Steer_ControlProc() for servo speed/position
 *         updates and sub-board end-effector auto-task triggering.
 ***************************************************************************************************/
void Function_100ms(void)
{
    if(g_RmtUpManRealMode == RMT_MODE)
    {
        Steer_ControlProc();
    }
}

/***************************************************************************************************
 * @name   Function_200ms(void)
 * @brief  200ms periodic tasks (reserved).
 * @param  None
 * @note   Currently unused. Reserved for future low-frequency state reporting.
 ***************************************************************************************************/
void Function_200ms(void)
{
}

/***************************************************************************************************
 * @name   Function_300ms(void)
 * @brief  300ms periodic tasks (reserved).
 * @param  None
 * @note   Currently unused. Reserved for future low-frequency tasks.
 ***************************************************************************************************/
void Function_300ms(void)
{
}

/***************************************************************************************************
 * @name   Function_500ms(void)
 * @brief  500ms periodic task: diagnostic feedback to upper computer.
 * @param  None
 * @note   Sends DIAG frame (0x82) with remote/upper online status, encoder/IMU validity,
 *         and CAN communication state flags.
 ***************************************************************************************************/
void Function_500ms(void)
{
    s_diag_feedback_pending = 1U;
}

/***************************************************************************************************
 * @name   Function_1s(void)
 * @brief  1s periodic tasks: LED heartbeat toggle and debug print.
 * @param  None
 * @note   Toggles the work LED and BOOT LED, and optionally prints debug info if
 *         APP_USART2_TEXT_DEBUG is enabled.
 ***************************************************************************************************/
void Function_1s(void)
{
    LED_WorkCtrlFun();
    function_prtf();
}

/***************************************************************************************************
 * @name   LED_WorkCtrlFun(void)
 * @brief  Toggle the work LED and BOOT LED at 1 Hz.
 * @param  None
 * @note   LED1 and SW_BOOT_LED toggle together, providing a visual heartbeat
 *         indication that the main loop is running.
 ***************************************************************************************************/
void LED_WorkCtrlFun(void)
{
    static u8 led_toggle = 0U;

    if((led_toggle & 0x01U) == 0U)
    {
        LED1(ON);
        SW_BOOT_LED(ON);
    }
    else
    {
        LED1(OFF);
        SW_BOOT_LED(OFF);
    }

    led_toggle++;
}

/***************************************************************************************************
 * @name   USART1_Proc(void)
 * @brief  USART1 receive completion handler.
 * @param  None
 * @note   USART1 is currently reserved for RS-485 auxiliary communication.
 *         Clears the RX status flag when a frame has been received (DMA/IDLE complete).
 ***************************************************************************************************/
void USART1_Proc(void)
{
    if(USART1_RX_STA)
    {
        USART1_RX_STA = 0U;
    }
}

/***************************************************************************************************
 * @name   USART2_Proc(void)
 * @brief  USART2 receive completion flag handler.
 * @param  None
 * @note   USART2 carries the upper-computer binary protocol. The actual frame
 *         parsing is done by ROS2_CommProc/Upper_CommProc; this function only
 *         clears the DMA/IDLE completion status flag.
 ***************************************************************************************************/
void USART2_Proc(void)
{
    if(USART2_RX_STA)
    {
        USART2_RX_STA = 0U;
    }
}

/***************************************************************************************************
 * @name   USART4_Proc(void)
 * @brief  UART4 receive completion flag handler.
 * @param  None
 * @note   UART4 is used for RS-485 PTZ gimbal communication. PTZ commands are
 *         sent via the ptz_data module; this function handles RX completion.
 ***************************************************************************************************/
void USART4_Proc(void)
{
    if(USART4_RX_STA)
    {
        USART4_RX_STA = 0U;
    }
}

/***************************************************************************************************
 * @name   USART5_Proc(void)
 * @brief  UART5 receive frame processing.
 * @param  None
 * @note   UART5 serves as the RS-485 link to the end-effector sub-board (STM32F103).
 *         Frame parsing and protocol handling are delegated to SubBoard_LinkProc().
 ***************************************************************************************************/
void USART5_Proc(void)
{
    SubBoard_LinkProc();
}

/***************************************************************************************************
 * @name   function_prtf(void)
 * @brief  Low-frequency debug-print function (1 Hz).
 * @param  None
 * @note   Only active when APP_USART2_TEXT_DEBUG is set to 1. Prints control mode,
 *         M8010 speed, servo speed, and PTZ movement flags to USART2.
 *         Keep disabled during normal operation to avoid corrupting the binary protocol.
 ***************************************************************************************************/
void function_prtf(void)
{
#if APP_USART2_TEXT_DEBUG
    swgPrtUx(USART2, "ctrl=%d yushu=%.1f steer=%d ptz=%d/%d\r\n",
            (int)g_RmtUpManCtrlMode,
            YushuSpeed,
            SteerSendMsgArr[STEER_ROTATE_SERVO_INDEX].SpeedData,
            PTZ_UpDownMoveFlg,
            PTZ_LftRgtMoveFlg);
#endif
}


/******************* (C) COPYRIGHT 2026 END OF FILE ***************************/


