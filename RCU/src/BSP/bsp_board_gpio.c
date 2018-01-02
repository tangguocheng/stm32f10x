#include "stm32f10x.h"
#include "bsp_board_gpio.h"
#include "task_485.h"

void board_gpio_init(void)
{
        GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
        
        GPIO_InitTypeDef GPIO_InitStruct;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
                
        //RL1--RL17
        GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_15;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOA, &GPIO_InitStruct);
        
        GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_8 | GPIO_Pin_9;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOB, &GPIO_InitStruct);
        
        GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOB, &GPIO_InitStruct);
        
        GPIO_InitStruct.GPIO_Pin = 0xFF & (~GPIO_Pin_12);
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOD, &GPIO_InitStruct);
       
        GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOC, &GPIO_InitStruct);

        RL_OFF(1);RL_OFF(6);RL_OFF(10);RL_OFF(14);
        RL_OFF(2);RL_OFF(7);RL_OFF(11);RL_OFF(15);
        RL_OFF(3);RL_OFF(8);RL_OFF(12);RL_OFF(16);
        RL_OFF(4);RL_OFF(9);RL_OFF(13);RL_OFF(17);
        RL_OFF(5);
        
        GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOD, &GPIO_InitStruct);
        
        GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_7;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOA, &GPIO_InitStruct);
        
        GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOE, &GPIO_InitStruct);
        
        GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOE, &GPIO_InitStruct);

}


