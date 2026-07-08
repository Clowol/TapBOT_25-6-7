#ifndef __FEETECH_SERVO_H
#define __FEETECH_SERVO_H

#include "stm32f10x.h"

void FeetechServo_Init(void);
void FeetechServo_SetModeSpeed123(void);
void FeetechServo_SetModePosition123(void);
void FeetechServo_SetSpeed123(s16 speed0, s16 speed1, s16 speed2);
void FeetechServo_SetPosition123(s16 pos0, s16 pos1, s16 pos2, u16 run_time, u16 speed);
void FeetechServo_SetRotateSpeed(s16 speed);
void FeetechServo_Stop123(void);
void FeetechServo_StopAll(void);


#endif
