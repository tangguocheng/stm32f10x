#include "stm32f10x.h"

/**
 * @brief soft delay N * 1us
 * @param u16 us
 * @return NONE
 * @note System Clock: 72MHz
 */
void delay_nus(u16 us)
{
        u16 i;
        while (us--) {
                i = 5;
                while(i--);
        }
}

/**
 * @brief soft delay 20us
 * @param NONE
 * @return NONE
 * @note System Clock: 72MHz
 */
void delay_20us(void)
{
        u8 i;
        i = 142;
        while (--i);
}

/**
 * @brief soft delay 5us
 * @param NONE
 * @return NONE
 * @note System Clock: 72MHz
 */
void delay_5us(void)
{
        u8 i = 0;
        for(i=0; i<33; i++);
}

/**
 * @brief soft delay N * 1ms
 * @param u16 ms
 * @return NONE
 * @note System Clock: 72MHz
 */
void delay_nms(u16 ms)
{
        u16 i = 0;
        while(ms--) {
                i = 8000;
                while(i--) ;
        }
}

