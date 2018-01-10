#include "stm32f10x.h"
#include "bsp_delay.h"
#include "bsp_led_display.h"

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

#define NUM_A_()        do {LED_ON(A);LED_ON(B);LED_ON(C);LED_OFF(D);LED_ON(E);LED_ON(F);LED_ON(G);}while(0)
#define NUM_C_()        do {LED_ON(A);LED_OFF(B);LED_OFF(C);LED_ON(D);LED_ON(E);LED_ON(F);LED_OFF(G);}while(0)
#define NUM_E_()        do {LED_ON(A);LED_OFF(B);LED_OFF(C);LED_ON(D);LED_ON(E);LED_ON(F);LED_ON(G);}while(0)
#define NUM_F_()        do {LED_ON(A);LED_OFF(B);LED_OFF(C);LED_OFF(D);LED_ON(E);LED_ON(F);LED_ON(G);}while(0)
#define NUM_U_()        do {LED_OFF(A);LED_ON(B);LED_ON(C);LED_ON(D);LED_ON(E);LED_ON(F);LED_OFF(G);}while(0)
#define NUM_P_()        do {LED_ON(A);LED_ON(B);LED_OFF(C);LED_OFF(D);LED_ON(E);LED_ON(F);LED_ON(G);}while(0)

#define SHOW_NUMBER(x)  NUM_##x##_()
#define LED_1_CS()      do {GPIO_SetBits(GPIOC, GPIO_Pin_13); GPIO_ResetBits(GPIOA, GPIO_Pin_4 ); } while(0)
#define LED_2_CS()      do {GPIO_SetBits(GPIOA, GPIO_Pin_4); GPIO_ResetBits(GPIOC, GPIO_Pin_13 ); } while(0)

#define LED_POS(x)      LED_##x##_CS()

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

void time_cnt_down(void)
{
        SHOW_NUMBER(3);
        delay_nms(1000);
        SHOW_NUMBER(2);
        delay_nms(1000);
        SHOW_NUMBER(1);
        delay_nms(1000);
        SHOW_NUMBER(0);
}

void led_set_number(u8 num)
{
        switch (num) {
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
        case 'A':
                SHOW_NUMBER(A);
                break;
        case 'C':
                SHOW_NUMBER(C);
                break;
        case 'E':
                SHOW_NUMBER(E);
                break;
        case 'F':
                SHOW_NUMBER(F);
                break;
        case 'U':
                SHOW_NUMBER(U);
                break;
        case 'P':
                SHOW_NUMBER(P);
                break;
        default:
                break;
        }
}

static volatile u8 lsb = 0;
static volatile u8 msb = 0;

void set_led_content(u8 type, u8 value)
{
        if ((type == LED_TYPE_NUM) && (value < 100)) {
                lsb = value % 10;
                msb = value / 10;
        } else if (type == LED_TYPE_INFO) {
                switch (value) {
                case ERR_VALUE_0:
                case ERR_VALUE_1:
                case ERR_VALUE_2:
                case ERR_VALUE_3:
                case ERR_VALUE_4:
                case ERR_VALUE_5:
                case ERR_VALUE_6:
                case ERR_VALUE_7:
                case ERR_VALUE_8:
                        msb = 'E';
                        lsb = value;
                        break;
                case UPDATE_VALUE:
                        msb = 'U';
                        lsb = 'P';
                        break;
                case APP_VALUE:
                        msb = 'A';
                        lsb = 'P';
                        break;
                default:
                        break;
                }

        }
}

#define switch_interval         10

void led_display(void)
{
        static u16 time_ms_cnt = 0;

        time_ms_cnt++;
        if (time_ms_cnt == switch_interval) {
                LED_POS(1);
                led_set_number(msb);
        } else if (time_ms_cnt == switch_interval * 2){
                time_ms_cnt = 0;
                LED_POS(2);
                led_set_number(lsb);
        }
}

