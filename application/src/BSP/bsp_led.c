#include "stm32f10x.h"
#include "bsp_led.h"
void led_init(void )
{
        GPIO_InitTypeDef GPIO_InitStruct;
        #if RCU_YN == 1
        GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOA, &GPIO_InitStruct);
        #else
        GPIO_InitStruct.GPIO_Pin = GPIO_Pin_15;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOE, &GPIO_InitStruct);
        #endif
}
