#ifndef _BSP_LED_H
#define _BSP_LED_H


#define RCU_YN 1
#if RCU_YN == 1
        #define STATE_LED_ON()  GPIO_SetBits(GPIOA,GPIO_Pin_0)
        #define STATE_LED_OFF()  GPIO_ResetBits(GPIOA,GPIO_Pin_0)
#else
        #define STATE_LED_ON()  GPIO_SetBits(GPIOE,GPIO_Pin_15)
        #define STATE_LED_OFF()  GPIO_ResetBits(GPIOE,GPIO_Pin_15)
#endif

#define STATE_LED_TOGGLE() do{static u8 toggle = 0; if (toggle) {STATE_LED_ON(); toggle = 0;} else {STATE_LED_OFF();toggle = 1;}}while(0)
void led_init(void );

#endif
