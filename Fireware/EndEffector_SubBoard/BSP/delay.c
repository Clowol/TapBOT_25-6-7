#include "delay.h"

void Delay_Init(void)
{
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
}

void Delay_ms(u32 ms)
{
    u32 i;
    for(i = 0U; i < ms; i++)
    {
        SysTick->LOAD = 9000U - 1U;
        SysTick->VAL = 0U;
        SysTick->CTRL = SysTick_CTRL_ENABLE_Msk;
        while((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0U)
        {
        }
        SysTick->CTRL = 0U;
    }
}
