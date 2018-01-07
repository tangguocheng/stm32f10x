#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "bsp_uart1.h"
#include "CRC16.h"
#include "bsp_board_gpio.h"
#include "task_485.h"
#include "bsp_delay.h"
#include "bsp_uart3.h"

static u8 current_protocol = WK8001_PROTOCOL;           // –≠“È«–ªª
static u8 data_idx = 0;
static QueueHandle_t  send_485_queue = NULL;
static QueueHandle_t  receive_485_queue = NULL;
static u8 time_scheduler_start = 0;
static u16 time_scheduler_time = 0;
static u8 wk8001_time_start = 0;
static u16 wk8001_time_cnt = 0;
static u8 wk8001_time_start_2 = 0;
static u16 wk8001_time_cnt_2 = 0;
static u8 light_state_word = 0;
static u8 read_state_word = 0;
static u8 sleep_state_word = 0;
static u8 cozy_state_word = 0;
volatile u8 rs485_data_in = 0;
volatile u8 data_01 = 0;
volatile u8 data_03 = 0;
volatile u8 data_05 = 0;
volatile u8 data_02 = 0;
volatile u8 data_04 = 0;
volatile u8 data_06 = 0;
static u8 PA5_enable = 1;
struct portocol_data_frame {
        u8 use;
        u8 pro_id;
        u8 data_len;
        u8 data[25];
};

#define SEND_FRAME_LEN 30

static struct portocol_data_frame protocol_data_wait_sent[SEND_FRAME_LEN];

#define RECV_FRAME_LEN 20

static struct portocol_data_frame protocol_data_receive[RECV_FRAME_LEN];

static inline u8 get_avalid_frame(struct portocol_data_frame* frame,u8 frame_len)
{
        u8 i;
        for (i = 0; i < frame_len; i++ )
                if (!frame[i].use)
                        break;

        return (( i < frame_len) ?  i : 0);
}

BaseType_t send_queue_item(u8 from_isr,struct portocol_data_frame **protocol_data)
{
        void *data = protocol_data;
        if (send_485_queue == NULL)
                return pdFALSE;

        BaseType_t xHigherPriorityTaskWoken;

        if (from_isr)
                xQueueSendFromISR(send_485_queue, &data, &xHigherPriorityTaskWoken);
        else
                xQueueSend(send_485_queue, &data, 0);

        return( xHigherPriorityTaskWoken);
}


void send_protocol_data(struct portocol_data_frame * protocol_data)
{
        delay_nms(10);
        GPIO_SetBits(GPIOA,GPIO_Pin_8);
        delay_nms(2);
        for (u8 i = 0; i < protocol_data->data_len; i++)
                Uart1_send_data(protocol_data->data[i]);

        delay_nms(1);
        GPIO_ResetBits(GPIOA, GPIO_Pin_8);

        protocol_data->use = 0;

}

void key_0x13_delay_proc(void);
void key_0x14_delay_proc(void);
void key_0x16_delay_proc(void);

static u8 time_scheduler_flag = 0;
static u8 wk8001_flag_1 = 0;
static u8 wk8001_flag_2 = 0;
void time_scheduler(void)
{
        if (time_scheduler_time) {
                time_scheduler_flag = 1;
                time_scheduler_time--;
        }

        if ((time_scheduler_start == 1) && (time_scheduler_time == 0) && (time_scheduler_flag == 1)) {
                time_scheduler_flag = 0;
                time_scheduler_start = 0;
                u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                protocol_data_wait_sent[idx].use = 1;;
                u8* protocol_data = protocol_data_wait_sent[idx].data;
                protocol_data[0] = 0xd8;
                data_04 = data_04 & (~BIT2);
                protocol_data[3] = data_02;
                protocol_data[5] = data_04;
                protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^ protocol_data[2] ^ 
                                   protocol_data[3] ^ protocol_data[4] ^ protocol_data[5] ^
                                   protocol_data[6] ;

                protocol_data_wait_sent[idx].data_len = 8;
                void* ptr = &protocol_data_wait_sent[idx];
                send_queue_item(1,ptr);
        }
        
        if (wk8001_time_cnt) {
                wk8001_flag_1 = 1;
                wk8001_time_cnt--;
        }
        
        if (wk8001_time_cnt == 0 && (wk8001_flag_1 == 1)) {
                wk8001_flag_1 = 0;
                if (wk8001_time_start & BIT1) {
                        wk8001_time_start &= ~BIT1;
                        RL_OFF(11);RL_OFF(12);RL_OFF(13);
                } else if (wk8001_time_start & BIT2){
                        wk8001_time_start &= ~BIT2;
                        RL_ON(11);RL_OFF(12);RL_OFF(13);
                } else if (wk8001_time_start & BIT3){
                        wk8001_time_start &= ~BIT3;
                        RL_OFF(11);RL_ON(12);RL_OFF(13);
                } else if (wk8001_time_start & BIT4){
                        wk8001_time_start &= ~BIT4;
                        RL_OFF(11);RL_OFF(12);RL_ON(13);
                }
        }
        
        if (wk8001_time_cnt_2) {
                wk8001_flag_2 = 1;
                wk8001_time_cnt_2--;
        }
        
        if (wk8001_time_cnt_2 == 0 && (wk8001_flag_2 == 1)) {
                wk8001_flag_2 = 0;
                if (wk8001_time_start_2 & BIT1) {
                        RL_ON(11);RL_OFF(12);RL_OFF(13);
                } else if (wk8001_time_start_2 & BIT2){
                        RL_OFF(11);RL_OFF(12);RL_ON(13);
                } else if (wk8001_time_start_2 & BIT3){
                        RL_OFF(11);RL_OFF(12);RL_ON(13);
                }
        }        
        
        key_0x13_delay_proc();
        key_0x14_delay_proc();
        key_0x16_delay_proc();
}

static u8 wk_data[14] = {0x01,0x01,0x02,0x00,0x01,0x19,0x19,0x10,0x10,0x10,0x10,0x10,0x10,0x10};
void copy_wk_data(u8* des)
{
        for (u8 i = 0; i < 14; i ++)
                des[i] = wk_data[i];
}

void update_wk_data(u8* src)
{
        for (u8 i = 0; i < 14; i ++)
                wk_data[i] = src[i];
}

void device_poll(void)
{
        if (current_protocol == WK8001_PROTOCOL) {
                u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                protocol_data_wait_sent[idx].use = 1;;
                u8* protocol_data = protocol_data_wait_sent[idx].data;

                protocol_data[0] = 0x41;protocol_data[1] = 0x43;protocol_data[2] = 0x54;
                protocol_data[3] = 0x11;
                
                copy_wk_data(&protocol_data[4]);
                
                crc16_cal(&protocol_data[19], &protocol_data[18], &protocol_data[3], 15);

                protocol_data_wait_sent[idx].data_len = 20;
                void* ptr = &protocol_data_wait_sent[idx];
                send_queue_item(0,ptr);

        } else if (current_protocol == SHANGHUI_PROTOCOL) {
                u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                protocol_data_wait_sent[idx].use = 1;;
                u8* protocol_data = protocol_data_wait_sent[idx].data;

                protocol_data[0] = 0x74;protocol_data[1] = 0xAA;protocol_data[2] = 0xDE;

                protocol_data_wait_sent[idx].data_len = 3;
                void* ptr = &protocol_data_wait_sent[idx];
                send_queue_item(0,ptr);
        }
}


static u8 receive_idx = 0;

void receive_protocol_data(u8 data)
{
        rs485_data_in = 1;
        if (current_protocol == WK8001_PROTOCOL) {
                protocol_data_receive[receive_idx].data[data_idx] = data;
                data_idx++;
                if (protocol_data_receive[receive_idx].data[0] != 0x41) {
                        data_idx = 0;
                        return;
                } else if (data_idx >= 17) {
                        u8 high_byte,low_byte;
                        data_idx = 0;
                        crc16_cal(&high_byte, &low_byte, &protocol_data_receive[receive_idx].data[3], 12);
//                        if ((high_byte == protocol_data_receive[receive_idx].data[16]) && (low_byte == protocol_data_receive[receive_idx].data[15])) {
                        if ((0x43 == protocol_data_receive[receive_idx].data[1]) && (0x54 == protocol_data_receive[receive_idx].data[2])) {

                                protocol_data_receive[receive_idx].use = 1;
                                protocol_data_receive[receive_idx].pro_id = WK8001_PROTOCOL;
                                void* ptr = &protocol_data_receive[receive_idx];
                                BaseType_t task_woken = pdFALSE;
                                xQueueSendFromISR( receive_485_queue, &ptr, &task_woken );
                                portYIELD_FROM_ISR(task_woken);
                        }
                        receive_idx = get_avalid_frame(protocol_data_receive,RECV_FRAME_LEN);
                }
        } else if (current_protocol == SHANGHUI_PROTOCOL) {
                protocol_data_receive[receive_idx].data[data_idx] = data;
                data_idx++;
                if ((protocol_data_receive[receive_idx].data[0] < 0x10) || (protocol_data_receive[receive_idx].data[0] > 0x73)) {
                        data_idx = 0;
                        return ;
                }

                if (data_idx >= 3) {
                        data_idx = 0;
                        if (protocol_data_receive[receive_idx].data[2] == (protocol_data_receive[receive_idx].data[0] ^ protocol_data_receive[receive_idx].data[1])) {
                                protocol_data_receive[receive_idx].use = 1;
                                protocol_data_receive[receive_idx].pro_id = SHANGHUI_PROTOCOL;
                                void* ptr = &protocol_data_receive[receive_idx];
                                BaseType_t task_woken = pdFALSE;
                                xQueueSendFromISR( receive_485_queue, &ptr, &task_woken );
                                portYIELD_FROM_ISR(task_woken);
                        }

                        receive_idx = get_avalid_frame(protocol_data_receive,RECV_FRAME_LEN);
                }

        }
}

static u8 key_disable_0x11 = 0;
static u8 key_disable_0x12 = 0;
static u8 key_disable_0x13 = 0;
static u8 key_disable_0x14 = 0;
static u8 key_disable_0x15 = 0;
static u8 key_disable_0x16 = 0;

#define KEY_0x12    BIT1
#define KEY_0x14    BIT2
#define KEY_wkq     BIT3
void gpio_input_process(void)
{
        static u8 first = 1;
        static u8 last_PD12 = 3,last_PA4 = 3,last_PA5 = 3;
        u8 PD12 = GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_12);
        u8 PA4 = GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_4);
        u8 PA5 = GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_5);
        u8 PA7 = GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_7);
        BaseType_t rtl = pdFALSE;

        if (first) {
                first = 0;
                last_PD12 = PD12;last_PA4 = PA4;last_PA5 = PA5;
                return;
        }

        static u16 delay_off_time_D12 = 0;
        static u8 start_delay_D12 = 0;

        if (PD12 != last_PD12) {
                last_PD12 = PD12;
                if ( PD12 == Bit_RESET ) {
                        RL_ON(1);RL_ON(2);RL_ON(9);RL_ON(10);
                        GPIO_ResetBits(GPIOE,GPIO_Pin_0);
                        RL_ON(14);RL_ON(16);
                        start_delay_D12 = BIT1;
                        delay_off_time_D12 = 2000;
                        GPIO_SetBits(GPIOE,GPIO_Pin_0);
                        
                        u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                        protocol_data_wait_sent[idx].use = 1;;
                        u8* protocol_data = protocol_data_wait_sent[idx].data;
                        protocol_data[0] = 0xd8;
                        data_01 |= BIT2;
                        data_03 |= (BIT1 | BIT2);
                        data_05 |= BIT2;
                        protocol_data[2] = data_01;
                        protocol_data[4] = data_03;
                        protocol_data[6] = data_05;
                        protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^ protocol_data[2] ^ 
                                           protocol_data[3] ^ protocol_data[4] ^ protocol_data[5] ^
                                           protocol_data[6] ;

                        protocol_data_wait_sent[idx].data_len = 8;
                        void* ptr = &protocol_data_wait_sent[idx];
                        rtl = send_queue_item(1,ptr);
                } else if (PD12 == Bit_SET) {
                        start_delay_D12 = BIT2;
                        delay_off_time_D12 = 60000;
                        RL_ON(15);RL_ON(17);
                }
        }

        if (delay_off_time_D12)
                delay_off_time_D12--;
        
        if ((delay_off_time_D12 == 58000) && (start_delay_D12 == BIT2)) {
                RL_OFF(15);RL_OFF(17);
        }
                
        if ((start_delay_D12 != 0) && (delay_off_time_D12 == 0)) {
                if (start_delay_D12 == BIT1) {
                        RL_OFF(14);RL_OFF(16);
                        start_delay_D12 = 0;
                } else if (start_delay_D12 == BIT2){
                        RL_OFF(1);RL_OFF(6);RL_OFF(10);
                        RL_OFF(2);RL_OFF(7);RL_OFF(11);
                        RL_OFF(3);RL_OFF(8);RL_OFF(12);
                        RL_OFF(4);RL_OFF(9);RL_OFF(13);
                        RL_OFF(5);GPIO_ResetBits(GPIOE,GPIO_Pin_0);

                        
                        u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                        protocol_data_wait_sent[idx].use = 1;;
                        u8* protocol_data = protocol_data_wait_sent[idx].data;
                        protocol_data[0] = 0xd8;
                        data_01 &= ~BIT2;
                        data_02 &= ~(BIT1 | BIT2 | BIT3);
                        data_03 &= ~(BIT1 | BIT2 | BIT3);
                        data_04 &= ~(BIT1 | BIT2 | BIT3);
                        data_05 &= ~(BIT1 | BIT2 | BIT3);
                        
                        protocol_data[2] = data_01;
                        protocol_data[3] = data_02;
                        protocol_data[4] = data_03;
                        protocol_data[5] = data_04;
                        protocol_data[6] = data_05;
                        protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^ protocol_data[2] ^ 
                                           protocol_data[3] ^ protocol_data[4] ^ protocol_data[5] ^
                                           protocol_data[6] ;

                        protocol_data_wait_sent[idx].data_len = 8;
                        void* ptr = &protocol_data_wait_sent[idx];
                        rtl = send_queue_item(1,ptr);
                        
                        idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                        protocol_data_wait_sent[idx].use = 1;;
                        protocol_data = protocol_data_wait_sent[idx].data;
                        protocol_data[0] = 0xd9;
                        data_06 &= ~(BIT1 | BIT2);                      
                        protocol_data[1] = data_06;
                        protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^ protocol_data[2] ^ 
                                           protocol_data[3] ^ protocol_data[4] ^ protocol_data[5] ^
                                           protocol_data[6] ;

                        protocol_data_wait_sent[idx].data_len = 8;
                        ptr = &protocol_data_wait_sent[idx];
                        rtl = send_queue_item(1,ptr);
                        
                        key_disable_0x12 |= (BIT1 | BIT2 | BIT3);
                        key_disable_0x13 |= (BIT1 | BIT2 | BIT3);
                        key_disable_0x14 |= (BIT1 | BIT2 | BIT3);
                        key_disable_0x15 |= (BIT1 | BIT2 | BIT3);
                        key_disable_0x16 |= (BIT1 | BIT2);
                }              
        }


        static u16 delay_off_time_A4 = 0;
        static u8 start_delay_A4 = 0;
        if ((PA4 != last_PA4) && (PD12 != Bit_RESET)) {
                last_PA4 = PA4;
                if ( (PD12 == Bit_SET) && (PA4 == Bit_RESET)) {
                        start_delay_A4 = 1;
                        delay_off_time_A4 = 60000;
                        RL_ON(2);
                        u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                        protocol_data_wait_sent[idx].use = 1;;
                        u8* protocol_data = protocol_data_wait_sent[idx].data;
                        protocol_data[0] = 0xd8;
                        data_01 |= BIT2;
                        protocol_data[2] = data_01;
                        protocol_data[3] = data_02;
                        protocol_data[4] = data_03;
                        protocol_data[5] = data_04;
                        protocol_data[6] = data_05;
                        protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^ protocol_data[2] ^ 
                        protocol_data[3] ^ protocol_data[4] ^ protocol_data[5] ^
                        protocol_data[6] ;
                        protocol_data_wait_sent[idx].data_len = 8;
                        void* ptr = &protocol_data_wait_sent[idx];
                        rtl = send_queue_item(1,ptr);
                }
        }

        if (delay_off_time_A4)
                delay_off_time_A4--;

        if ((start_delay_A4 == 1) && (delay_off_time_A4 == 0)) {
                start_delay_A4 = 0;
                RL_OFF(2);
                u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                protocol_data_wait_sent[idx].use = 1;;
                u8* protocol_data = protocol_data_wait_sent[idx].data;
                protocol_data[0] = 0xd8;
                data_01 &= (~BIT2);
                protocol_data[2] = data_01;
                protocol_data[3] = data_02;
                protocol_data[4] = data_03;
                protocol_data[5] = data_04;
                protocol_data[6] = data_05;
                protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^ protocol_data[2] ^ 
                protocol_data[3] ^ protocol_data[4] ^ protocol_data[5] ^
                protocol_data[6] ;
                protocol_data_wait_sent[idx].data_len = 8;
                void* ptr = &protocol_data_wait_sent[idx];
                rtl = send_queue_item(1,ptr);
        }

        static u8 start_delay_A5 = 0;
        static u16 delay_off_time_A5 = 0;

        if (PD12 == Bit_RESET)
                PA5_enable = 1;
        else if (PD12 == Bit_SET)
                PA5_enable = 0;
                
        if ((PA5 != last_PA5) && PA5_enable) {
                last_PA5 = PA5;
                if ( PA5 == Bit_RESET ) {
                        start_delay_A5 = 1;
                        delay_off_time_A5 = 2000;
                        IO_OUTPUT9_ON();
                }
        }

        if (delay_off_time_A5)
                delay_off_time_A5--;

        if ((start_delay_A5 == 1) && (delay_off_time_A5 == 0)) {
                start_delay_A5 = 0;
                IO_OUTPUT9_OFF();
        }

        portYIELD_FROM_ISR(rtl);
}

static u16 delay_cnt = 0;
void task_485_poll(void* param)
{
        (void)param;
        while(1) {
                vTaskDelay(10 / portTICK_PERIOD_MS);
                delay_cnt++;
                
                if ((delay_cnt % 3) == 0) {
                        current_protocol = SHANGHUI_PROTOCOL;
                        device_poll();
                } 
//                else
//                if (delay_cnt == 200) {
//                        current_protocol = WK8001_PROTOCOL;
//                        device_poll();
//                        vTaskDelay(90 / portTICK_PERIOD_MS);
//                        delay_cnt = 0;
//                }      
        }
}

void key_0x11(u8 rx_data)
{
        if (rx_data & BIT1) { 
                ((GPIO_ReadOutputDataBit(GPIOE,GPIO_Pin_1 ) == Bit_SET) ? GPIO_ResetBits(GPIOE,GPIO_Pin_1) : GPIO_SetBits(GPIOE,GPIO_Pin_1));
                ((GPIO_ReadOutputDataBit(GPIOE,GPIO_Pin_2 ) == Bit_SET) ? GPIO_ResetBits(GPIOE,GPIO_Pin_2) : GPIO_SetBits(GPIOE,GPIO_Pin_2));

                u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                protocol_data_wait_sent[idx].use = 1;;
                u8* protocol_data = protocol_data_wait_sent[idx].data;
                protocol_data[0] = 0xd8;
                if (GPIO_ReadOutputDataBit(GPIOE,GPIO_Pin_1 )  == Bit_RESET) {
                        data_01 &= (~BIT1);
                } else {
                        GPIO_ResetBits(GPIOE,GPIO_Pin_3);
                        GPIO_ResetBits(GPIOE,GPIO_Pin_4);        
                        data_01 &= (~BIT3);
                        data_01 |= BIT1;
                }
                protocol_data[2] = data_01;
                protocol_data[3] = data_02;
                protocol_data[4] = data_03;
                protocol_data[5] = data_04;
                protocol_data[6] = data_05;
                protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^ protocol_data[2] ^ 
                                   protocol_data[3] ^ protocol_data[4] ^ protocol_data[5] ^
                                   protocol_data[6] ;
                protocol_data_wait_sent[idx].data_len = 8;
                void* ptr = &protocol_data_wait_sent[idx];
                send_queue_item(0,ptr);
        }

        if (rx_data & BIT2) {
                RL_TOGGLE(2);
                u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                protocol_data_wait_sent[idx].use = 1;
                u8* protocol_data = protocol_data_wait_sent[idx].data;
                protocol_data[0] = 0xd8;
                data_01 = (RL_STATE(2) ? data_01 | BIT2 : data_01 & (~BIT2));
                protocol_data[2] = data_01;
                protocol_data[3] = data_02;
                protocol_data[4] = data_03;
                protocol_data[5] = data_04;
                protocol_data[6] = data_05;
                protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^ protocol_data[2] ^ 
                                   protocol_data[3] ^ protocol_data[4] ^ protocol_data[5] ^
                                   protocol_data[6] ;
                protocol_data_wait_sent[idx].data_len = 8;                                                
                void* ptr = &protocol_data_wait_sent[idx];
                send_queue_item(0,ptr);
        }

        if (rx_data & BIT3) { 
                ((GPIO_ReadOutputDataBit(GPIOE,GPIO_Pin_3 ) == Bit_SET) ? GPIO_ResetBits(GPIOE,GPIO_Pin_3) : GPIO_SetBits(GPIOE,GPIO_Pin_3));
                ((GPIO_ReadOutputDataBit(GPIOE,GPIO_Pin_4 ) == Bit_SET) ? GPIO_ResetBits(GPIOE,GPIO_Pin_4) : GPIO_SetBits(GPIOE,GPIO_Pin_4));
                if (GPIO_ReadOutputDataBit(GPIOE,GPIO_Pin_3 ) == Bit_SET) {
                        PA5_enable = 0;
                        data_01 |= BIT3;
                        data_01 &= (~BIT1);
                } else {
                        PA5_enable = 1;
                        data_01 &= (~BIT3);
                }

                u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                protocol_data_wait_sent[idx].use = 1;;
                u8* protocol_data = protocol_data_wait_sent[idx].data;
                protocol_data[0] = 0xd8;
                protocol_data[2] = data_01;
                protocol_data[3] = data_02;
                protocol_data[4] = data_03;
                protocol_data[5] = data_04;
                protocol_data[6] = data_05;
                protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^ protocol_data[2] ^ 
                                   protocol_data[3] ^ protocol_data[4] ^ protocol_data[5] ^
                                   protocol_data[6] ;
                protocol_data_wait_sent[idx].data_len = 8;                                                
                RL_TOGGLE(3);                                                
                void* ptr = &protocol_data_wait_sent[idx];
                send_queue_item(0,ptr);
        }
}

void key_0x12(u8 rx_data)
{
        if (rx_data & BIT1) {
                RL_TOGGLE(3);
                u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                protocol_data_wait_sent[idx].use = 1;;
                u8* protocol_data = protocol_data_wait_sent[idx].data;
                protocol_data[0] = 0xd8;
              
                if (RL_STATE(3)) {
                        data_02 |= BIT1;
                } else {
                        data_02 &= (~BIT1);
                }
                
                protocol_data[2] = data_01;
                protocol_data[3] = data_02;
                protocol_data[4] = data_03;
                protocol_data[5] = data_04;
                protocol_data[6] = data_05;
                protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^ protocol_data[2] ^ 
                                   protocol_data[3] ^ protocol_data[4] ^ protocol_data[5] ^
                                   protocol_data[6] ;
                protocol_data_wait_sent[idx].data_len = 8;                                                   

                void* ptr = &protocol_data_wait_sent[idx];
                send_queue_item(0,ptr);
        }
        
        if (rx_data & BIT2) {
                RL_TOGGLE(5);
                u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                protocol_data_wait_sent[idx].use = 1;;
                u8* protocol_data = protocol_data_wait_sent[idx].data;
                protocol_data[0] = 0xd8;
     
                if (RL_STATE(5)) {
                        data_02 |= BIT2;
                } else {
                        data_02 &= (~BIT2);
                }
                
                protocol_data[2] = data_01;
                protocol_data[3] = data_02;
                protocol_data[4] = data_03;
                protocol_data[5] = data_04;
                protocol_data[6] = data_05;
                protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^ protocol_data[2] ^ 
                                   protocol_data[3] ^ protocol_data[4] ^ protocol_data[5] ^
                                   protocol_data[6] ;
                protocol_data_wait_sent[idx].data_len = 8;                                                   

                void* ptr = &protocol_data_wait_sent[idx];
                send_queue_item(0,ptr);
        }
        
        if (rx_data & BIT3) {
                RL_TOGGLE(4);
                u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                protocol_data_wait_sent[idx].use = 1;;
                u8* protocol_data = protocol_data_wait_sent[idx].data;
                protocol_data[0] = 0xd8;
               
              
                if (RL_STATE(4)) {
                        data_02 |= BIT3;
                } else {
                        data_02 &= (~BIT3);
                }
                
                protocol_data[2] = data_01;
                protocol_data[3] = data_02;
                protocol_data[4] = data_03;
                protocol_data[5] = data_04;
                protocol_data[6] = data_05;
                protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^ protocol_data[2] ^ 
                                   protocol_data[3] ^ protocol_data[4] ^ protocol_data[5] ^
                                   protocol_data[6] ;
                protocol_data_wait_sent[idx].data_len = 8;                                                   

                void* ptr = &protocol_data_wait_sent[idx];
                send_queue_item(0,ptr);
        }
}

static u8 key_0x13_delay_flag = 0;
static u16 key_0x13_delay_time = 0;
static u8 key_0x13_delay_start = 0;
void key_0x13_delay_proc(void)
{
        if (key_0x13_delay_time) {
                key_0x13_delay_start = 1;
                key_0x13_delay_time--;
        }
        
        if (key_0x13_delay_time == 0 && key_0x13_delay_start == 1) {
                key_0x13_delay_start = 0;
                if (key_0x13_delay_flag & BIT1) {
                        key_0x13_delay_flag &= ~BIT1;
                        RL_OFF(16);
                }
                
                if (key_0x13_delay_flag & BIT2) {
                        key_0x13_delay_flag &= ~BIT2;
                        RL_OFF(17);
                }
        }
}

void key_0x13(u8 rx_data)
{
        if (rx_data & BIT1) {
                RL_TOGGLE(16);
                u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                protocol_data_wait_sent[idx].use = 1;;
                u8* protocol_data = protocol_data_wait_sent[idx].data;
                protocol_data[0] = 0xd8;
              
                if (RL_STATE(16)) {
                        data_03 |= BIT1;
                        data_03 &= (~BIT3);

                } else {
                        data_03 &= (~BIT1);
                }
                key_0x13_delay_flag |= BIT1;
                key_0x13_delay_time = 2000;
                
                protocol_data[2] = data_01;
                protocol_data[3] = data_02;
                protocol_data[4] = data_03;
                protocol_data[5] = data_04;
                protocol_data[6] = data_05;
                protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^ protocol_data[2] ^ 
                                   protocol_data[3] ^ protocol_data[4] ^ protocol_data[5] ^
                                   protocol_data[6] ;
                protocol_data_wait_sent[idx].data_len = 8;                                                   

                void* ptr = &protocol_data_wait_sent[idx];
                send_queue_item(0,ptr);
        }
        
        if (rx_data & BIT2) {
                RL_TOGGLE(10);
                u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                protocol_data_wait_sent[idx].use = 1;;
                u8* protocol_data = protocol_data_wait_sent[idx].data;
                protocol_data[0] = 0xd8;
     
                if (RL_STATE(10)) {
                        data_03 |= BIT2;
                } else {
                        data_03 &= (~BIT2);
                }
                
                protocol_data[2] = data_01;
                protocol_data[3] = data_02;
                protocol_data[4] = data_03;
                protocol_data[5] = data_04;
                protocol_data[6] = data_05;
                protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^ protocol_data[2] ^ 
                                   protocol_data[3] ^ protocol_data[4] ^ protocol_data[5] ^
                                   protocol_data[6] ;
                protocol_data_wait_sent[idx].data_len = 8;                                                   

                void* ptr = &protocol_data_wait_sent[idx];
                send_queue_item(0,ptr);
        }
        
        if (rx_data & BIT3) {
                RL_TOGGLE(17);
                u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                protocol_data_wait_sent[idx].use = 1;;
                u8* protocol_data = protocol_data_wait_sent[idx].data;
                protocol_data[0] = 0xd8;

                if (RL_STATE(17)) {
                        data_03 |= BIT3;

                } 
                data_03 &= (~BIT1);
                key_0x13_delay_flag |= BIT2;
                key_0x13_delay_time = 2000;
                
                protocol_data[2] = data_01;
                protocol_data[3] = data_02;
                protocol_data[4] = data_03;
                protocol_data[5] = data_04;
                protocol_data[6] = data_05;
                protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^ protocol_data[2] ^ 
                                   protocol_data[3] ^ protocol_data[4] ^ protocol_data[5] ^
                                   protocol_data[6] ;
                protocol_data_wait_sent[idx].data_len = 8;                                                   

                void* ptr = &protocol_data_wait_sent[idx];
                send_queue_item(0,ptr);
        }
}

static u8 key_0x14_delay_flag = 0;
static u16 key_0x14_delay_time = 0;
static u8 key_0x14_delay_start = 0;
void key_0x14_delay_proc(void)
{
        if (key_0x14_delay_time) {
                key_0x14_delay_start = 1;
                key_0x14_delay_time--;
        }
        
        if ((key_0x14_delay_flag & BIT1) && (key_0x14_delay_time == 8000)) {
                        key_0x13_delay_flag &= ~BIT1;
                        RL_OFF(15); RL_OFF(17);
        } 
                
        if ((key_0x14_delay_time == 0) && (key_0x14_delay_start == 1)) {
                key_0x14_delay_start = 0;
                if (key_0x14_delay_flag & BIT2) {
                        key_0x13_delay_flag &= ~BIT2;
                        u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                        protocol_data_wait_sent[idx].use = 1;;
                        u8* protocol_data = protocol_data_wait_sent[idx].data;
                        protocol_data[0] = 0xd8;
                        
                        data_04 &= (~BIT2);
                      
                        protocol_data[2] = data_01;
                        protocol_data[3] = data_02;
                        protocol_data[4] = data_03;
                        protocol_data[5] = data_04;
                        protocol_data[6] = data_05;
                        protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^ protocol_data[2] ^ 
                                           protocol_data[3] ^ protocol_data[4] ^ protocol_data[5] ^
                                           protocol_data[6] ;
                        protocol_data_wait_sent[idx].data_len = 8;                                                   

                        void* ptr = &protocol_data_wait_sent[idx];
                        send_queue_item(1,ptr);
                }
        }
}

void key_0x14(u8 rx_data)
{
        if (rx_data & BIT1) {
                RL_TOGGLE(6);
                u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                protocol_data_wait_sent[idx].use = 1;;
                u8* protocol_data = protocol_data_wait_sent[idx].data;
                protocol_data[0] = 0xd8;
              
                if (RL_STATE(6)) {
                        data_04 |= BIT1;
                } else {
                        data_04 &= (~BIT1);
                }
                
                protocol_data[2] = data_01;
                protocol_data[3] = data_02;
                protocol_data[4] = data_03;
                protocol_data[5] = data_04;
                protocol_data[6] = data_05;
                protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^ protocol_data[2] ^ 
                                   protocol_data[3] ^ protocol_data[4] ^ protocol_data[5] ^
                                   protocol_data[6] ;
                protocol_data_wait_sent[idx].data_len = 8;                                                   

                void* ptr = &protocol_data_wait_sent[idx];
                send_queue_item(0,ptr);
        }
        
        if (rx_data & BIT2) {
                RL_OFF(2);RL_OFF(3);RL_OFF(4);RL_OFF(5);
                RL_OFF(7);RL_OFF(8);RL_OFF(9);RL_OFF(10);
                RL_OFF(11);RL_OFF(12);RL_OFF(13);
                RL_ON(6);
                cozy_state_word = 0;
                RL_ON(15);RL_ON(17);
                key_0x14_delay_flag |= BIT1;
                u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                protocol_data_wait_sent[idx].use = 1;;
                u8* protocol_data = protocol_data_wait_sent[idx].data;
                protocol_data[0] = 0xd8;
                
                data_01 &= (~BIT2);
                data_02 &= (~(BIT1 | BIT3));
                data_03 &= (~(BIT1 | BIT3));
                data_04 |= BIT2;
                key_0x14_delay_flag |= BIT2;
                key_0x14_delay_time = 10000;
                data_04 &= (~(BIT1 | BIT3));
                data_05 &= (~(BIT1 | BIT3));
                protocol_data[2] = data_01;
                protocol_data[3] = data_02;
                protocol_data[4] = data_03;
                protocol_data[5] = data_04;
                protocol_data[6] = data_05;
                protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^ protocol_data[2] ^ 
                                   protocol_data[3] ^ protocol_data[4] ^ protocol_data[5] ^
                                   protocol_data[6] ;
                protocol_data_wait_sent[idx].data_len = 8;                                                   

                void* ptr = &protocol_data_wait_sent[idx];
                send_queue_item(0,ptr);
                
                idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                protocol_data_wait_sent[idx].use = 1;;
                protocol_data = protocol_data_wait_sent[idx].data;
                protocol_data[0] = 0xd9;
                data_06 &= (~(BIT1 | BIT2));                    
                protocol_data[1] = data_06;
                protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^ protocol_data[2] ^ 
                protocol_data[3] ^ protocol_data[4] ^ protocol_data[5] ^
                protocol_data[6] ;

                protocol_data_wait_sent[idx].data_len = 8;
                ptr = &protocol_data_wait_sent[idx];
                send_queue_item(0,ptr);
        }
        
        if (rx_data & BIT3) {
               
                u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                protocol_data_wait_sent[idx].use = 1;;
                u8* protocol_data = protocol_data_wait_sent[idx].data;
                protocol_data[0] = 0xd8;

                if (cozy_state_word == 0) {
                        RL_ON(9);RL_ON(10);
                        RL_OFF(2);RL_OFF(7);RL_OFF(8);
                        RL_OFF(10);RL_OFF(11);RL_OFF(12);
                        RL_OFF(13);
                        cozy_state_word = 1;
                        data_01 &= (~BIT2);
                        data_02 &= (~(BIT1 | BIT3));
                        data_03 &= (~(BIT1 | BIT3));
                        data_04 &= (~(BIT1 | BIT2));
                        data_04 |= BIT3;
                        data_05 &= (~(BIT1 | BIT3));
                        data_06 &= (~(BIT1 | BIT2));
                        u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                        protocol_data_wait_sent[idx].use = 1;;
                        protocol_data = protocol_data_wait_sent[idx].data;
                        protocol_data[0] = 0xd9;
                        protocol_data[1] = data_06;
                        protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^ protocol_data[2] ^ 
                                           protocol_data[3] ^ protocol_data[4] ^ protocol_data[5] ^
                                           protocol_data[6] ;
                        protocol_data_wait_sent[idx].data_len = 8;                                                
                        RL_TOGGLE(3);                                                
                        void* ptr = &protocol_data_wait_sent[idx];
                        send_queue_item(0,ptr);
                } else {
                        cozy_state_word = 0;
                        RL_OFF(8);RL_OFF(9);
                        data_04 &= (~BIT3);
                }
                
                protocol_data[2] = data_01;
                protocol_data[3] = data_02;
                protocol_data[4] = data_03;
                protocol_data[5] = data_04;
                protocol_data[6] = data_05;
                protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^ protocol_data[2] ^ 
                                   protocol_data[3] ^ protocol_data[4] ^ protocol_data[5] ^
                                   protocol_data[6] ;
                protocol_data_wait_sent[idx].data_len = 8;                                                   

                void* ptr = &protocol_data_wait_sent[idx];
                send_queue_item(0,ptr);   
        }
}

void key_0x15(u8 rx_data)
{
        if (rx_data & BIT1) {
                RL_TOGGLE(7);
                u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                protocol_data_wait_sent[idx].use = 1;;
                u8* protocol_data = protocol_data_wait_sent[idx].data;
                protocol_data[0] = 0xd8;
              
                if (RL_STATE(7)) {
                        data_05 |= BIT1;
                } else {
                        data_05 &= (~BIT1);
                }
                
                protocol_data[2] = data_01;
                protocol_data[3] = data_02;
                protocol_data[4] = data_03;
                protocol_data[5] = data_04;
                protocol_data[6] = data_05;
                protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^ protocol_data[2] ^ 
                                   protocol_data[3] ^ protocol_data[4] ^ protocol_data[5] ^
                                   protocol_data[6] ;
                protocol_data_wait_sent[idx].data_len = 8;                                                   

                void* ptr = &protocol_data_wait_sent[idx];
                send_queue_item(0,ptr);
        }
        
        if (rx_data & BIT2) {
                RL_TOGGLE(9);
                u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                protocol_data_wait_sent[idx].use = 1;;
                u8* protocol_data = protocol_data_wait_sent[idx].data;
                protocol_data[0] = 0xd8;
              
                if (RL_STATE(9)) {
                        data_05 |= BIT2;
                } else {
                        data_05 &= (~BIT2);
                }
                
                protocol_data[2] = data_01;
                protocol_data[3] = data_02;
                protocol_data[4] = data_03;
                protocol_data[5] = data_04;
                protocol_data[6] = data_05;
                protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^ protocol_data[2] ^ 
                                   protocol_data[3] ^ protocol_data[4] ^ protocol_data[5] ^
                                   protocol_data[6] ;
                protocol_data_wait_sent[idx].data_len = 8;                                                   

                void* ptr = &protocol_data_wait_sent[idx];
                send_queue_item(0,ptr);
        }
        
        if (rx_data & BIT3) {
                RL_TOGGLE(8);
                u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                protocol_data_wait_sent[idx].use = 1;;
                u8* protocol_data = protocol_data_wait_sent[idx].data;
                protocol_data[0] = 0xd8;
              
                if (RL_STATE(8)) {
                        data_05 |= BIT3;
                } else {
                        data_05 &= (~BIT3);
                }
                
                protocol_data[2] = data_01;
                protocol_data[3] = data_02;
                protocol_data[4] = data_03;
                protocol_data[5] = data_04;
                protocol_data[6] = data_05;
                protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^ protocol_data[2] ^ 
                                   protocol_data[3] ^ protocol_data[4] ^ protocol_data[5] ^
                                   protocol_data[6] ;
                protocol_data_wait_sent[idx].data_len = 8;                                                   

                void* ptr = &protocol_data_wait_sent[idx];
                send_queue_item(0,ptr);
        }
}


static u8 key_0x16_delay_flag = 0;
static u16 key_0x16_delay_time = 0;
static u8 key_0x16_delay_start = 0;
void key_0x16_delay_proc(void)
{
        if (key_0x16_delay_time) {
                key_0x16_delay_start = 1;
                key_0x16_delay_time--;
        }
        
        if (key_0x16_delay_time == 0 && key_0x16_delay_start == 1) {
                key_0x16_delay_start = 0;
                if (key_0x16_delay_flag & BIT1) {
                        key_0x16_delay_flag &= ~BIT1;
                        RL_OFF(14);
                }
                
                if (key_0x16_delay_flag & BIT2) {
                        key_0x16_delay_flag &= ~BIT2;
                        RL_OFF(15);
                }
        }
}

void key_0x16(u8 rx_data)
{
        if (rx_data & BIT1) {
                RL_TOGGLE(14);
                u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                protocol_data_wait_sent[idx].use = 1;;
                u8* protocol_data = protocol_data_wait_sent[idx].data;
                protocol_data[0] = 0xd9;
              
                if (RL_STATE(14)) {
                        data_06 |= BIT1;
                        data_06 &= (~BIT2);
                        key_0x16_delay_flag |= BIT1;
                } else {
                        data_06 &= (~BIT1);
                }
                
                protocol_data[1] = data_06;
                protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^ protocol_data[2] ^ 
                                   protocol_data[3] ^ protocol_data[4] ^ protocol_data[5] ^
                                   protocol_data[6] ;
                protocol_data_wait_sent[idx].data_len = 8;                                                   

                void* ptr = &protocol_data_wait_sent[idx];
                send_queue_item(0,ptr);
        }
        
        if (rx_data & BIT2) {
                RL_TOGGLE(15);
                u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                protocol_data_wait_sent[idx].use = 1;;
                u8* protocol_data = protocol_data_wait_sent[idx].data;
                protocol_data[0] = 0xd9;
              
                if (RL_STATE(15)) {
                        data_06 |= BIT2;
                        data_06 &= (~BIT1);
                        key_0x16_delay_flag |= BIT2;
                } else {
                        data_06 &= (~BIT2);
                }
                
                protocol_data[1] = data_06;
                protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^ protocol_data[2] ^ 
                                   protocol_data[3] ^ protocol_data[4] ^ protocol_data[5] ^
                                   protocol_data[6] ;
                protocol_data_wait_sent[idx].data_len = 8;                                                   

                void* ptr = &protocol_data_wait_sent[idx];
                send_queue_item(0,ptr);
        }        
}

static u8 last_data3 = 4;
static u8 last_data45 = 3;
void wk8001_proc(u8* wk8001_data)
{
        if (wk8001_data[0] == 1)
                RL_ON(3);
        else if (wk8001_data[0] == 0) {
                RL_OFF(11);RL_OFF(12);RL_OFF(13);
        }

        if (last_data3 == 4)
                last_data3 = wk8001_data[3];
        else if (last_data3 != wk8001_data[3])
                wk8001_time_start = 0;

        if (wk8001_data[3] == 0) {                                           
                wk8001_time_cnt = 2000;
                wk8001_time_start |= BIT1;
        } else if (wk8001_data[3] == 1) {                                        
                wk8001_time_cnt = 2000;
                wk8001_time_start |= BIT2;
        } else if (wk8001_data[3] == 2) {                                        
                wk8001_time_cnt = 2000;
                wk8001_time_start |= BIT3;
        } else if (wk8001_data[3] == 3) {                                        
                wk8001_time_cnt = 2000;
                wk8001_time_start |= BIT4;
        }

        if (last_data3 == 4)
                last_data3 = wk8001_data[3];
        else if (last_data3 != wk8001_data[3]) {
                last_data3 = wk8001_data[3];
                wk8001_time_start = 0;
        }

        u8 data45 = 0;
        if (wk8001_data[4] == wk8001_data[5]) {
                wk8001_time_cnt_2 = 6000;
                wk8001_time_start_2 |= BIT1;
                data45 = 0;
        } else if (wk8001_data[4] > wk8001_data[5]) {                                        
                wk8001_time_cnt_2 = 6000;
                wk8001_time_start_2 |= BIT2;
                data45 = 1;
        } else if (wk8001_data[4] < wk8001_data[5]) {                                        
                wk8001_time_cnt_2 = 6000;
                wk8001_time_start_2 |= BIT3;
                data45 = 2;
        }

        if (last_data45 == 3)
                last_data45 = data45;
        else if (last_data45 != data45) {
                wk8001_time_start_2 = 0;
                last_data45 = data45;
        }
}

void task_485_receive(void* param)
{
        receive_485_queue = xQueueCreate(10, sizeof(struct portocol_data_frame *));
        while(1) {
                static struct portocol_data_frame *rx = NULL;
                if ( xQueueReceive(receive_485_queue, &rx, portMAX_DELAY) == pdTRUE ) {
                        if (rx->pro_id == SHANGHUI_PROTOCOL) {
                                switch (rx->data[0]) {
                                case 0x11:
                                       key_0x11(rx->data[1]);
                                        break;
                                case 0x12:
                                       key_0x12(rx->data[1]);
                                        break;
                                case 0x13:
                                       key_0x13(rx->data[1]);
                                        break;                                                                
                                case 0x14:
                                       key_0x14(rx->data[1]);
                                        break;
                                case 0x15:
                                       key_0x15(rx->data[1]);
                                        break;
                                case 0x16:
                                       key_0x16(rx->data[1]);
                                        break;
                                default:
                                        break;
                                }
                        } else if( rx->pro_id == WK8001_PROTOCOL) {                                       
                                if (rx->data[4] == 0x01) {
                                        update_wk_data(&rx->data[4]);
                                        wk8001_proc(&rx->data[5]);
                                }
                        }
                }
                
                rx->use = 0;
        }
}


void task_485_send(void* param)
{
        (void)param;
        send_485_queue = xQueueCreate(10, sizeof(struct portocol_data_frame *));
        while(1) {
                static struct portocol_data_frame *tx = NULL;
                if ( xQueueReceive(send_485_queue, &tx, portMAX_DELAY) == pdTRUE ) {
                       send_protocol_data(tx);
                }
        }
}

