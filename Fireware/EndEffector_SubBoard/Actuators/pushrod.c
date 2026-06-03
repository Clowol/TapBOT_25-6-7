#include "pushrod.h"
#include "gpio.h"

void Pushrod_Extend(void)
{
    EndGpio_PushrodExtendOn();
}

void Pushrod_Retract(void)
{
    EndGpio_PushrodRetractOn();
}

void Pushrod_Stop(void)
{
    EndGpio_PushrodOff();
}
