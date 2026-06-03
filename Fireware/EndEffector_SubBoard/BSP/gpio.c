#include "gpio.h"

#define PHOTO_GPIO_RCC       RCC_APB2Periph_GPIOA
#define PHOTO_GPIO_PORT      GPIOA
#define PHOTO_GPIO_PIN       GPIO_Pin_0
#define PUSH_GPIO_RCC        RCC_APB2Periph_GPIOB
#define PUSH_GPIO_PORT       GPIOB
#define PUSH_EXTEND_PIN      GPIO_Pin_0
#define PUSH_RETRACT_PIN     GPIO_Pin_1

void EndGpio_Init(void)
{
    GPIO_InitTypeDef gpio;

    RCC_APB2PeriphClockCmd(PHOTO_GPIO_RCC | PUSH_GPIO_RCC, ENABLE);

    gpio.GPIO_Pin = PHOTO_GPIO_PIN;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(PHOTO_GPIO_PORT, &gpio);

    gpio.GPIO_Pin = PUSH_EXTEND_PIN | PUSH_RETRACT_PIN;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(PUSH_GPIO_PORT, &gpio);
    EndGpio_PushrodOff();
}

u8 EndGpio_ObjectDetected(void)
{
    return (GPIO_ReadInputDataBit(PHOTO_GPIO_PORT, PHOTO_GPIO_PIN) == Bit_RESET) ? 1U : 0U;
}

void EndGpio_PushrodExtendOn(void)
{
    GPIO_SetBits(PUSH_GPIO_PORT, PUSH_EXTEND_PIN);
    GPIO_ResetBits(PUSH_GPIO_PORT, PUSH_RETRACT_PIN);
}

void EndGpio_PushrodRetractOn(void)
{
    GPIO_ResetBits(PUSH_GPIO_PORT, PUSH_EXTEND_PIN);
    GPIO_SetBits(PUSH_GPIO_PORT, PUSH_RETRACT_PIN);
}

void EndGpio_PushrodOff(void)
{
    GPIO_ResetBits(PUSH_GPIO_PORT, PUSH_EXTEND_PIN | PUSH_RETRACT_PIN);
}
