#ifndef _BSP_BOARD_GPIO_H
#define _BSP_BOARD_GPIO_H

#define RL_1_ON()       GPIO_SetBits(GPIOB,GPIO_Pin_9)
#define RL_1_OFF()      GPIO_ResetBits(GPIOB,GPIO_Pin_9)

#define RL_2_ON()       GPIO_SetBits(GPIOB,GPIO_Pin_8)
#define RL_2_OFF()      GPIO_ResetBits(GPIOB,GPIO_Pin_8)

#define RL_3_ON()       GPIO_SetBits(GPIOB,GPIO_Pin_5)
#define RL_3_OFF()      GPIO_ResetBits(GPIOB,GPIO_Pin_5)

#define RL_4_ON()       GPIO_SetBits(GPIOB,GPIO_Pin_4)
#define RL_4_OFF()      GPIO_ResetBits(GPIOB,GPIO_Pin_4)

#define RL_5_ON()       GPIO_SetBits(GPIOB,GPIO_Pin_3)
#define RL_5_OFF()      GPIO_ResetBits(GPIOB,GPIO_Pin_3)

#define RL_6_ON()       GPIO_SetBits(GPIOD,GPIO_Pin_7)
#define RL_6_OFF()      GPIO_ResetBits(GPIOD,GPIO_Pin_7)

#define RL_7_ON()       GPIO_SetBits(GPIOD,GPIO_Pin_6)
#define RL_7_OFF()      GPIO_ResetBits(GPIOD,GPIO_Pin_6)

#define RL_8_ON()       GPIO_SetBits(GPIOD,GPIO_Pin_5)
#define RL_8_OFF()      GPIO_ResetBits(GPIOD,GPIO_Pin_5)

#define RL_9_ON()       GPIO_SetBits(GPIOD,GPIO_Pin_4)
#define RL_9_OFF()      GPIO_ResetBits(GPIOD,GPIO_Pin_4)

#define RL_10_ON()       GPIO_SetBits(GPIOD,GPIO_Pin_3)
#define RL_10_OFF()      GPIO_ResetBits(GPIOD,GPIO_Pin_3)

#define RL_11_ON()       GPIO_SetBits(GPIOD,GPIO_Pin_2)
#define RL_11_OFF()      GPIO_ResetBits(GPIOD,GPIO_Pin_2)

#define RL_12_ON()       GPIO_SetBits(GPIOD,GPIO_Pin_1)
#define RL_12_OFF()      GPIO_ResetBits(GPIOD,GPIO_Pin_1)

#define RL_13_ON()       GPIO_SetBits(GPIOD,GPIO_Pin_0)
#define RL_13_OFF()      GPIO_ResetBits(GPIOD,GPIO_Pin_0)

#define RL_14_ON()       GPIO_SetBits(GPIOC,GPIO_Pin_12)
#define RL_14_OFF()      GPIO_ResetBits(GPIOC,GPIO_Pin_12)

#define RL_14_ON()       GPIO_SetBits(GPIOC,GPIO_Pin_12)
#define RL_14_OFF()      GPIO_ResetBits(GPIOC,GPIO_Pin_12)

#define RL_15_ON()       GPIO_SetBits(GPIOC,GPIO_Pin_11)
#define RL_15_OFF()      GPIO_ResetBits(GPIOC,GPIO_Pin_11)

#define RL_16_ON()       GPIO_SetBits(GPIOC,GPIO_Pin_10)
#define RL_16_OFF()      GPIO_ResetBits(GPIOC,GPIO_Pin_10)

#define RL_17_ON()       GPIO_SetBits(GPIOA,GPIO_Pin_15)
#define RL_17_OFF()      GPIO_ResetBits(GPIOA,GPIO_Pin_15)

#define IO_OUTPUT10_ON()        GPIO_SetBits(GPIOC,GPIO_Pin_2)    
#define IO_OUTPUT9_ON()         GPIO_SetBits(GPIOC,GPIO_Pin_1)

#define IO_OUTPUT10_OFF()       GPIO_ResetBits(GPIOC,GPIO_Pin_2)     
#define IO_OUTPUT9_OFF()        GPIO_ResetBits(GPIOC,GPIO_Pin_1)

#define RL_ON(x)         RL_##x##_ON()
#define RL_OFF(x)        RL_##x##_OFF()

void board_gpio_init(void);
void gpio_input_process(void);        
#endif


