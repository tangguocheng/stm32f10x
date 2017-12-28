#include "stm32f10x.h"
#include "bsp_led_display.h"

/**
 * @brief System Timer Initialization
 * @param NONE
 * @return NONE
 * @note System Clock: 72MHz , Period: 1ms
 */
void sys_timer_init(u8 pre_priority,u8 sub_priority)
{
        NVIC_InitTypeDef nvic_struct;
        TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

        /* Enable the TIM7 global Interrupt */
        nvic_struct.NVIC_IRQChannel = TIM2_IRQn;
        nvic_struct.NVIC_IRQChannelPreemptionPriority = pre_priority;
        nvic_struct.NVIC_IRQChannelSubPriority = sub_priority;
        nvic_struct.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&nvic_struct);

        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
        TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1;
        TIM_TimeBaseStructure.TIM_Period = 1000 - 1;
        TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
        TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
        TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
        TIM_ClearFlag(TIM2, TIM_FLAG_Update);
        TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
        TIM_Cmd(TIM2, ENABLE);
}

void TIM2_IRQHandler(void)
{
        if ( TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET ) {
                led_display();
                TIM_ClearITPendingBit(TIM2, TIM_FLAG_Update);
        }
}
