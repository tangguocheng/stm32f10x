#include "stm32f10x.h"
#include "bsp_board_gpio.h"
#include "task_485.h"

void board_gpio_init(void)
{
        // IO-INPUT22\23\24
        GPIO_InitTypeDef GPIO_InitStruct;
        GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOA, &GPIO_InitStruct);
        
        //RL1--RL17
        GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_15;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOA, &GPIO_InitStruct);
        
        GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_8 | GPIO_Pin_9;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOB, &GPIO_InitStruct);
        
        GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOB, &GPIO_InitStruct);

        GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
        
        GPIO_InitStruct.GPIO_Pin = 0xFF;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOD, &GPIO_InitStruct);

        GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOC, &GPIO_InitStruct);

        RL_OFF(1);RL_OFF(6);RL_OFF(10);RL_OFF(14);
        RL_OFF(2);RL_OFF(7);RL_OFF(11);RL_OFF(15);
        RL_OFF(3);RL_OFF(8);RL_OFF(12);RL_OFF(16);
        RL_OFF(4);RL_OFF(9);RL_OFF(13);RL_OFF(17);
        RL_OFF(5);
        
        GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOE, &GPIO_InitStruct);
        GPIO_ResetBits(GPIOE,GPIO_Pin_3);
}

extern volatile u8 data_02;
extern volatile u8 data_04;
extern void uart_sent_data(u8* data,u8 len);
static u8 first = 1;
void gpio_input_process(void)
{
        static u8 last_PA3 = 3,last_PA4 = 3,last_PA5 = 3;
        u8 PA3 = GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_3);
        u8 PA4 = GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_4);
        u8 PA5 = GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_5);
        
        if (first) {
                first = 0;
                last_PA3 = PA3;
                last_PA4 = PA4;
                last_PA5 = PA5;
                return;
        }
        
        static u16 delay_off_time_A3 = 0;
        static u8 start_delay_A3 = 0;
        
        if (PA3 != last_PA3) {
                last_PA3 = PA3;
                if ( PA3 == Bit_RESET ) {
                        start_delay_A3 = 0;
                        delay_off_time_A3 = 0;
                        RL_ON(1);RL_ON(2);RL_ON(3);
                        RL_ON(7);RL_ON(8);RL_ON(11);
                        IO_OUTPUT9_ON();
                        // Todo:02地址RS485:01、02开关量打开
                        u8 protocol_data[8];
                        protocol_data[0] = 0xd8;
                        protocol_data[1] = 0;
                        protocol_data[2] = data_02 | BIT1 | BIT2;
                        protocol_data[3] = 0;
                        protocol_data[4] = 0;
                        protocol_data[5] = 0;
                        protocol_data[6] = 0;
                        protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^
                                           protocol_data[2] ^ protocol_data[3] ^
                                           protocol_data[4] ^ protocol_data[5] ^
                                           protocol_data[6] ;
                        uart_sent_data(protocol_data,8);
                } else if (PA3 == Bit_SET) {
                        start_delay_A3 = 1;
                        delay_off_time_A3 = 60000;
                }
        }
        
        if (delay_off_time_A3)
                delay_off_time_A3--;
        
        if ((start_delay_A3 == 1) && (delay_off_time_A3 == 0)) {
                start_delay_A3 = 0;
                RL_OFF(1);RL_OFF(6);RL_OFF(10);RL_OFF(14);
                RL_OFF(2);RL_OFF(7);RL_OFF(11);RL_OFF(15);
                RL_OFF(3);RL_OFF(8);RL_OFF(12);RL_OFF(16);
                RL_OFF(4);RL_OFF(9);RL_OFF(13);RL_OFF(17);
                RL_OFF(5);IO_OUTPUT9_OFF();
        }
        
        
        static u16 delay_off_time_A4 = 0;
        static u8 start_delay_A4 = 0;
        if (PA4 != last_PA4) {
                last_PA4 = PA4;
                if ( (PA3 == Bit_SET) && (PA4 == Bit_RESET)) {
                        start_delay_A4 = 1;
                        delay_off_time_A4 = 60000;
                        RL_ON(2);
                } 
        }
        
        if (delay_off_time_A4)
                delay_off_time_A4--;
        
        if ((start_delay_A4 == 1) && (delay_off_time_A4 == 0)) {
                start_delay_A4 = 0;
                RL_OFF(2);
        }
        
        static u8 start_delay_A5 = 0;
        static u16 delay_off_time_A5 = 0;
        
        if (PA5 != last_PA5) {
                last_PA5 = PA5;
                if ( PA5 == Bit_RESET ) {
                        start_delay_A5 = 1;
                        delay_off_time_A5 = 2000;
                        //IO_OUTPUT10_ON();
                        GPIO_SetBits(GPIOE,GPIO_Pin_3);
                } 
        }
        
        if (delay_off_time_A5)
                delay_off_time_A5--;
        
        if ((start_delay_A5 == 1) && (delay_off_time_A5 == 0)) {
                start_delay_A5 = 0;
                //IO_OUTPUT10_OFF();
                GPIO_ResetBits(GPIOE,GPIO_Pin_3);
        }
        
}
