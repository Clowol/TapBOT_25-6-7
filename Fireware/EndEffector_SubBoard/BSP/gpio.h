#ifndef __END_GPIO_H
#define __END_GPIO_H

#include "stm32f10x.h"

void EndGpio_Init(void);
u8 EndGpio_ObjectDetected(void);
void EndGpio_PushrodExtendOn(void);
void EndGpio_PushrodRetractOn(void);
void EndGpio_PushrodOff(void);


#endif
