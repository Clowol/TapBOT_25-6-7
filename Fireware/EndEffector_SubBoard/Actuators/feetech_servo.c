#include "feetech_servo.h"

void FeetechServo_Init(void)
{
    FeetechServo_StopAll();
}

void FeetechServo_SetModeSpeed123(void)
{
    /* TODO: Send SMS mode-write frame to set ID0-2 speed mode. */
}

void FeetechServo_SetModePosition123(void)
{
    /* TODO: Send SMS mode-write frame to set ID0-2 position mode. */
}

void FeetechServo_SetSpeed123(s16 speed0, s16 speed1, s16 speed2)
{
    (void)speed0;
    (void)speed1;
    (void)speed2;
    /* TODO: SyncWrite ID0-2 speed command. */
}

void FeetechServo_SetPosition123(s16 pos0, s16 pos1, s16 pos2, u16 run_time, u16 speed)
{
    (void)pos0;
    (void)pos1;
    (void)pos2;
    (void)run_time;
    (void)speed;
    /* TODO: SyncWrite ID0-2 position/time/speed command. */
}

void FeetechServo_SetRotateSpeed(s16 speed)
{
    (void)speed;
    /* TODO: Set ID3 continuous-rotation speed. */
}

void FeetechServo_Stop123(void)
{
    FeetechServo_SetSpeed123(0, 0, 0);
}

void FeetechServo_StopAll(void)
{
    FeetechServo_Stop123();
    FeetechServo_SetRotateSpeed(0);
}
