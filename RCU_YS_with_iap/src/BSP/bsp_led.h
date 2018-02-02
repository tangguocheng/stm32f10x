#ifndef _BSP_LED_H
#define _BSP_LED_H

void led_init(void );

#define STATE_LED_ON()  GPIO_SetBits(GPIOA,GPIO_Pin_0)
#define STATE_LED_OFF()  GPIO_ResetBits(GPIOA,GPIO_Pin_0)
#define rs485_LED_ON()     GPIO_SetBits(GPIOA,GPIO_Pin_0)
#define rs485_LED_OFF()    GPIO_ResetBits(GPIOA,GPIO_Pin_0)

#define STATE_LED_TOGGLE() do{static u8 toggle = 0; if (toggle) {STATE_LED_ON(); toggle = 0;} else {STATE_LED_OFF();toggle = 1;}}while(0)
#define rs485_LED_TOGGLE() do{static u8 toggle_485 = 0; if (toggle_485) {rs485_LED_ON(); toggle_485 = 0;} else {rs485_LED_OFF();toggle_485 = 1;}}while(0)

#endif
