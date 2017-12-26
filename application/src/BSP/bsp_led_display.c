#include "stm32f10x.h"
#include "delay.h"

#define LED_A_ON()      GPIOC->BRR = GPIO_Pin_2
#define LED_B_ON()      GPIOA->BRR = GPIO_Pin_0
#define LED_C_ON()      GPIOA->BRR = GPIO_Pin_1
#define LED_D_ON()      GPIOA->BRR = GPIO_Pin_3
#define LED_E_ON()      GPIOA->BRR = GPIO_Pin_2
#define LED_F_ON()      GPIOC->BRR = GPIO_Pin_1
#define LED_G_ON()      GPIOC->BRR = GPIO_Pin_3

#define LED_A_OFF()     GPIOC->BSRR = GPIO_Pin_2
#define LED_B_OFF()     GPIOA->BSRR = GPIO_Pin_0
#define LED_C_OFF()     GPIOA->BSRR = GPIO_Pin_1
#define LED_D_OFF()     GPIOA->BSRR = GPIO_Pin_3
#define LED_E_OFF()     GPIOA->BSRR = GPIO_Pin_2
#define LED_F_OFF()     GPIOC->BSRR = GPIO_Pin_1
#define LED_G_OFF()     GPIOC->BSRR = GPIO_Pin_3

#define LED_ON(x)        LED_##x##_ON()
#define LED_OFF(x)       LED_##x##_OFF()

#define NUM_0_()        do {LED_ON(A);LED_ON(B);LED_ON(C);LED_ON(D);LED_ON(E);LED_ON(F);LED_OFF(G);}while(0)
#define NUM_1_()        do {LED_OFF(A);LED_ON(B);LED_ON(C);LED_OFF(D);LED_OFF(E);LED_OFF(F);LED_OFF(G);}while(0)
#define NUM_2_()        do {LED_ON(A);LED_ON(B);LED_OFF(C);LED_ON(D);LED_ON(E);LED_OFF(F);LED_ON(G);}while(0)
#define NUM_3_()        do {LED_ON(A);LED_ON(B);LED_ON(C);LED_ON(D);LED_OFF(E);LED_OFF(F);LED_ON(G);}while(0)
#define NUM_4_()        do {LED_OFF(A);LED_ON(B);LED_ON(C);LED_OFF(D);LED_OFF(E);LED_ON(F);LED_ON(G);}while(0)
#define NUM_5_()        do {LED_ON(A);LED_OFF(B);LED_ON(C);LED_ON(D);LED_OFF(E);LED_ON(F);LED_ON(G);}while(0)
#define NUM_6_()        do {LED_ON(A);LED_OFF(B);LED_ON(C);LED_ON(D);LED_ON(E);LED_ON(F);LED_ON(G);}while(0)
#define NUM_7_()        do {LED_ON(A);LED_ON(B);LED_ON(C);LED_OFF(D);LED_OFF(E);LED_OFF(F);LED_OFF(G);}while(0)
#define NUM_8_()        do {LED_ON(A);LED_ON(B);LED_ON(C);LED_ON(D);LED_ON(E);LED_ON(F);LED_ON(G);}while(0)
#define NUM_9_()        do {LED_ON(A);LED_ON(B);LED_ON(C);LED_ON(D);LED_OFF(E);LED_ON(F);LED_ON(G);}while(0)

#define SHOW_NUMBER(x) NUM_##x##_()

void led_display_init(void)
{
        GPIO_InitTypeDef GPIO_InitStruct;
        GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;

        GPIO_Init(GPIOC, &GPIO_InitStruct);     //DIG2

        GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;

        GPIO_Init(GPIOA, &GPIO_InitStruct);     // DIG1

        GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOC, &GPIO_InitStruct);

        GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOA, &GPIO_InitStruct);

        GPIO_ResetBits(GPIOA, GPIO_Pin_4 );
        GPIO_ResetBits(GPIOA, GPIO_Pin_13 );
        GPIO_ResetBits(GPIOA, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 );
        GPIO_ResetBits(GPIOC, GPIO_Pin_13 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3);
}

void led_display_test(void)
{
        SHOW_NUMBER(0);
        delay_nms(1000);
        SHOW_NUMBER(1);
        delay_nms(1000);
        SHOW_NUMBER(2);
        delay_nms(1000);
        SHOW_NUMBER(3);
        delay_nms(1000);
        SHOW_NUMBER(4);
        delay_nms(1000);
        SHOW_NUMBER(5);
        delay_nms(1000);
        SHOW_NUMBER(6);
        delay_nms(1000);
        SHOW_NUMBER(7);
        delay_nms(1000);
        SHOW_NUMBER(8);
        delay_nms(1000);
        SHOW_NUMBER(9);
        delay_nms(1000);
}

void led_cnt(void)
{
        static u8 num_cnt = 9;

        switch (num_cnt) {
        case 0:
                SHOW_NUMBER(0);
                break;
        case 1:
                SHOW_NUMBER(1);
                break;
        case 2:
                SHOW_NUMBER(2);
                break;
        case 3:
                SHOW_NUMBER(3);
                break;
        case 4:
                SHOW_NUMBER(4);
                break;
        case 5:
                SHOW_NUMBER(5);
                break;
        case 6:
                SHOW_NUMBER(6);
                break;
        case 7:
                SHOW_NUMBER(7);
                break;
        case 8:
                SHOW_NUMBER(8);
                break;
        case 9:
                SHOW_NUMBER(9);
                break;
        default:
                SHOW_NUMBER(9);
                num_cnt = 9;
                break;
        }
        num_cnt--;
}
