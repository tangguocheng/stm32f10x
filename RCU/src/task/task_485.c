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

static u8 current_protocol = WK8001_PROTOCOL;           // 协议切换
static u8 data_idx = 0;
static QueueHandle_t  send_485_queue = NULL;
static QueueHandle_t  receive_485_queue = NULL;
static u8 time_scheduler_start = 0;
static u16 time_scheduler_time = 0;
static u8 light_state_word = 0;
static u8 read_state_word = 0;

volatile u8 rs485_data_in = 0;
volatile u8 data_02 = 0;
volatile u8 data_04 = 0;

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

        delay_nms(2);
        GPIO_ResetBits(GPIOA, GPIO_Pin_8);

        protocol_data->use = 0;

}


void time_scheduler(void)
{
        if (time_scheduler_time)
                time_scheduler_time--;

        if ((time_scheduler_start == 1) && (time_scheduler_time == 0)) {
                time_scheduler_start = 0;
                u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                protocol_data_wait_sent[idx].use = 1;;
                u8* protocol_data = protocol_data_wait_sent[idx].data;
                protocol_data[0] = 0xd8;
                protocol_data[3] = 0;
                protocol_data[5] = data_04 & (~BIT3);
                protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^
                                   protocol_data[2] ^ protocol_data[3] ^
                                   protocol_data[4] ^ protocol_data[5] ^
                                   protocol_data[6] ;

                protocol_data_wait_sent[idx].data_len = 8;
                void* ptr = &protocol_data_wait_sent[idx];
                send_queue_item(1,ptr);
        }
}

void device_poll(void)
{
        if (current_protocol == WK8001_PROTOCOL) {
                u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                protocol_data_wait_sent[idx].use = 1;;
                u8* protocol_data = protocol_data_wait_sent[idx].data;

                protocol_data[0] = 0x41;protocol_data[1] = 0x43;protocol_data[2] = 0x54;
                protocol_data[3] = 0x11;protocol_data[4] = 0x01;protocol_data[5] = 0x01;
                protocol_data[6] = 0x01;protocol_data[7] = 0x00;protocol_data[8] = 0x01;
                protocol_data[9] = 0x19;protocol_data[10] = 0x14;protocol_data[11] = 0x00;
                protocol_data[12] = 0x30;protocol_data[13] = 0x08;protocol_data[14] = 0x30;
                protocol_data[15] = 0x12;protocol_data[16] = 0x06;protocol_data[17] = 0x17;

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


static struct portocol_data_frame * receive_data = &protocol_data_receive[0];

void receive_protocol_data(u8 data)
{
        rs485_data_in = 1;
        if (current_protocol == WK8001_PROTOCOL) {
                receive_data->data[data_idx] = data;
                data_idx++;
                if (receive_data->data[0] != 0x41) {
                        data_idx = 0;
                        return;
                } else if (receive_data->data[1] != 0x43) {
                        data_idx = 0;
                        return;
                } else if (receive_data->data[2] != 0x54) {
                        data_idx = 0;
                        return;
                } else {
                        if (data_idx >= 20) {
                                u8 high_byte,low_byte;
                                data_idx = 0;
                                crc16_cal(&high_byte, &low_byte, &receive_data->data[3], 15);
                                if ((high_byte == receive_data->data[19]) && (low_byte == receive_data->data[18])) {
                                        receive_data->use = 1;
                                        receive_data->pro_id = WK8001_PROTOCOL;
                                        void* ptr = &receive_data;
                                        BaseType_t task_woken = pdFALSE;
                                        xQueueSendFromISR( receive_485_queue, &ptr, &task_woken );
                                        portYIELD_FROM_ISR(task_woken);

                                }
                                u8 idx = get_avalid_frame(protocol_data_receive,RECV_FRAME_LEN);
                                receive_data = &protocol_data_receive[idx];
                        }
                }
        } else if (current_protocol == SHANGHUI_PROTOCOL) {
                receive_data->data[data_idx] = data;
                data_idx++;
                if ((receive_data->data[0] < 0x10) || (receive_data->data[0] > 0x73)) {
                        data_idx = 0;
                        return ;
                }

                if (data_idx >= 3) {
                        data_idx = 0;
                        if (receive_data->data[2] == (receive_data->data[0] ^ receive_data->data[1])) {
                                receive_data->use = 1;
                                receive_data->pro_id = SHANGHUI_PROTOCOL;
                                void* ptr = &receive_data;
                                BaseType_t task_woken = pdFALSE;
                                xQueueSendFromISR( receive_485_queue, &ptr, &task_woken );
                                portYIELD_FROM_ISR(task_woken);
                        }

                        u8 idx = get_avalid_frame(protocol_data_receive,RECV_FRAME_LEN);
                        receive_data = &protocol_data_receive[idx];
                }

        }
}


void gpio_input_process(void)
{
        static u8 first = 1;
        static u8 last_PA3 = 3,last_PA4 = 3,last_PA5 = 3;
        u8 PA3 = GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_12);
        u8 PA4 = GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_4);
        u8 PA5 = GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_5);
        u8 PA7 = GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_7);
        BaseType_t rtl = pdFALSE;

        if (first) {
                first = 0;
                last_PA3 = PA3;last_PA4 = PA4;last_PA5 = PA5;
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
                        GPIO_SetBits(GPIOE,GPIO_Pin_0);
                        
                        u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                        protocol_data_wait_sent[idx].use = 1;;
                        u8* protocol_data = protocol_data_wait_sent[idx].data;
                        protocol_data[0] = 0xd8;
                        protocol_data[3] = BIT3 | BIT4;
                        protocol_data[5] = BIT1;
                        protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^
                                           protocol_data[2] ^ protocol_data[3] ^
                                           protocol_data[4] ^ protocol_data[5] ^
                                           protocol_data[6] ;
                        protocol_data_wait_sent[idx].data_len = 8;
                        void* ptr = &protocol_data_wait_sent[idx];
                        rtl = send_queue_item(1,ptr);
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
                RL_OFF(5);GPIO_ResetBits(GPIOE,GPIO_Pin_0);

                u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                protocol_data_wait_sent[idx].use = 1;;
                u8* protocol_data = protocol_data_wait_sent[idx].data;
                protocol_data[0] = 0xd8;
                protocol_data[3] = 0;
                protocol_data[4] = 0;
                protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^
                                   protocol_data[2] ^ protocol_data[3] ^
                                   protocol_data[4] ^ protocol_data[5] ^
                                   protocol_data[6] ;
                protocol_data_wait_sent[idx].data_len = 8;
                void* ptr = &protocol_data_wait_sent[idx];
                rtl = send_queue_item(1,ptr);
        }


        static u16 delay_off_time_A4 = 0;
        static u8 start_delay_A4 = 0;
        if ((PA4 != last_PA4) && (PA3 != Bit_RESET)) {
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
                if (PA3 != Bit_RESET)
                        RL_OFF(2);
        }

        static u8 start_delay_A5 = 0;
        static u16 delay_off_time_A5 = 0;

        if ((PA5 != last_PA5) && (PA3 == Bit_RESET) && (PA7 == Bit_RESET)) {
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

void task_485_poll(void* param)
{
        (void)param;
        while(1) {
//                current_protocol = WK8001_PROTOCOL;
//                device_poll();
//                vTaskDelay(200 / portTICK_PERIOD_MS);

                current_protocol = SHANGHUI_PROTOCOL;
                device_poll();
                vTaskDelay(30 / portTICK_PERIOD_MS);

//                device_poll();
//                vTaskDelay(200 / portTICK_PERIOD_MS);
        }
}

void task_485_receive(void* param)
{
        receive_485_queue = xQueueCreate(10, sizeof(struct portocol_data_frame *));
        while(1) {
                static struct portocol_data_frame *rx = NULL;
                if ( xQueueReceive(receive_485_queue, &rx, portMAX_DELAY) == pdTRUE ) {
                        if (rx->pro_id == WK8001_PROTOCOL) {
                                BaseType_t rtl = pdFALSE;
                                switch (rx->data[0]) {
                                case 0x12:
                                        data_02 = rx->data[1];
                                        if (rx->data[1] & BIT1) {
                                                RL_TOGGLE(4);
                                                u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                                                protocol_data_wait_sent[idx].use = 1;;
                                                u8* protocol_data = protocol_data_wait_sent[idx].data;
                                                protocol_data[0] = 0xd8;
                                                protocol_data[3] = (RL_STATE(4) ? data_02 | BIT1 : data_02 & (~BIT1));
                                                protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^
                                                                   protocol_data[2] ^ protocol_data[3] ^
                                                                   protocol_data[4] ^ protocol_data[5] ^
                                                                   protocol_data[6] ;
                                                protocol_data_wait_sent[idx].data_len = 8;
                                                void* ptr = &protocol_data_wait_sent[idx];
                                                rtl = send_queue_item(0,ptr);
                                        }
                                        if (rx->data[1] & BIT2) {
                                                RL_TOGGLE(6);
                                                u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                                                protocol_data_wait_sent[idx].use = 1;;
                                                u8* protocol_data = protocol_data_wait_sent[idx].data;
                                                protocol_data[0] = 0xd8;
                                                protocol_data[3] = (RL_STATE(6) ? data_02 | BIT1 : data_02 & (~BIT1));
                                                protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^
                                                                   protocol_data[2] ^ protocol_data[3] ^
                                                                   protocol_data[4] ^ protocol_data[5] ^
                                                                   protocol_data[6] ;
                                                protocol_data_wait_sent[idx].data_len = 8;
                                                void* ptr = &protocol_data_wait_sent[idx];
                                                rtl = send_queue_item(0,ptr);
                                        }
                                        if (rx->data[1] & BIT3) {
                                                RL_TOGGLE(3);
                                                u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                                                protocol_data_wait_sent[idx].use = 1;;
                                                u8* protocol_data = protocol_data_wait_sent[idx].data;
                                                protocol_data[0] = 0xd8;
                                                protocol_data[3] = (RL_STATE(3) ? data_02 | BIT1 : data_02 & (~BIT1));
                                                protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^
                                                                   protocol_data[2] ^ protocol_data[3] ^
                                                                   protocol_data[4] ^ protocol_data[5] ^
                                                                   protocol_data[6] ;
                                                protocol_data_wait_sent[idx].data_len = 8;
                                                void* ptr = &protocol_data_wait_sent[idx];
                                                rtl = send_queue_item(0,ptr);
                                        }

                                        if (rx->data[1] & BIT4) {
                                                RL_TOGGLE(2);
                                                u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                                                protocol_data_wait_sent[idx].use = 1;;
                                                u8* protocol_data = protocol_data_wait_sent[idx].data;
                                                protocol_data[0] = 0xd8;

                                                protocol_data[3] = (RL_STATE(2) ? data_02 | BIT4 : data_02 & (~BIT4));
                                                protocol_data[5] = (RL_STATE(2) ? data_04 | BIT1 : data_04 & (~BIT1));
                                                protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^
                                                                   protocol_data[2] ^ protocol_data[3] ^
                                                                   protocol_data[4] ^ protocol_data[5] ^
                                                                   protocol_data[6] ;
                                                protocol_data_wait_sent[idx].data_len = 8;
                                                void* ptr = &protocol_data_wait_sent[idx];
                                                rtl = send_queue_item(0,ptr);
                                        }

                                        if (rx->data[1] & BIT5) {
                                                ((GPIO_ReadOutputDataBit(GPIOE,GPIO_Pin_1 ) == Bit_SET) ? GPIO_ResetBits(GPIOE,GPIO_Pin_1) : GPIO_SetBits(GPIOE,GPIO_Pin_1));
                                                ((GPIO_ReadOutputDataBit(GPIOE,GPIO_Pin_2 ) == Bit_SET) ? GPIO_ResetBits(GPIOE,GPIO_Pin_2) : GPIO_SetBits(GPIOE,GPIO_Pin_2));
                                        }

                                        if (rx->data[1] & BIT6) {
                                                ((GPIO_ReadOutputDataBit(GPIOE,GPIO_Pin_3 ) == Bit_SET) ? GPIO_ResetBits(GPIOE,GPIO_Pin_3) : GPIO_SetBits(GPIOE,GPIO_Pin_3));
                                                ((GPIO_ReadOutputDataBit(GPIOE,GPIO_Pin_4 ) == Bit_SET) ? GPIO_ResetBits(GPIOE,GPIO_Pin_4) : GPIO_SetBits(GPIOE,GPIO_Pin_4));
                                        }
                                        break;
                                case 0x14:
                                        data_04 = rx->data[1];
                                        if (rx->data[1] & BIT1) {
                                                RL_TOGGLE(2);
                                                u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                                                protocol_data_wait_sent[idx].use = 1;;
                                                u8* protocol_data = protocol_data_wait_sent[idx].data;
                                                protocol_data[0] = 0xd8;
                                                protocol_data[5] = (RL_STATE(2) ? data_04 | BIT1 | BIT4: data_04 & (~BIT1) & (~BIT4));
                                                protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^
                                                                   protocol_data[2] ^ protocol_data[3] ^
                                                                   protocol_data[4] ^ protocol_data[5] ^
                                                                   protocol_data[6] ;
                                                protocol_data_wait_sent[idx].data_len = 8;
                                                void* ptr = &protocol_data_wait_sent[idx];
                                                send_queue_item(0,ptr);
                                        }

                                        if (rx->data[1] & BIT2) {
                                                if (light_state_word == 0) {
                                                        RL_ON(2);RL_ON(7);RL_ON(8);RL_ON(9);RL_ON(10);
                                                        light_state_word = 1;
                                                        read_state_word = 0;
                                                        u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                                                        protocol_data_wait_sent[idx].use = 1;;
                                                        u8* protocol_data = protocol_data_wait_sent[idx].data;
                                                        protocol_data[0] = 0xd8;
                                                        protocol_data[3] = data_02 | BIT4;
                                                        protocol_data[5] = data_04 | BIT1 | BIT4;
                                                        protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^
                                                                           protocol_data[2] ^ protocol_data[3] ^
                                                                           protocol_data[4] ^ protocol_data[5] ^
                                                                           protocol_data[6] ;
                                                        protocol_data_wait_sent[idx].data_len = 8;
                                                        void* ptr = &protocol_data_wait_sent[idx];
                                                        send_queue_item(0,ptr);
                                                } else {
                                                        RL_OFF(2);RL_OFF(7);RL_OFF(8);RL_OFF(9);RL_OFF(10);
                                                        light_state_word = 0;
                                                        u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                                                        protocol_data_wait_sent[idx].use = 1;;
                                                        u8* protocol_data = protocol_data_wait_sent[idx].data;
                                                        protocol_data[0] = 0xd8;
                                                        protocol_data[3] = data_02 | BIT4;
                                                        protocol_data[5] = data_04 | BIT1 | BIT4;
                                                        protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^
                                                                           protocol_data[2] ^ protocol_data[3] ^
                                                                           protocol_data[4] ^ protocol_data[5] ^
                                                                           protocol_data[6] ;
                                                        protocol_data_wait_sent[idx].data_len = 8;
                                                        void* ptr = &protocol_data_wait_sent[idx];
                                                        send_queue_item(0,ptr);
                                                }
                                        }

                                        if (rx->data[1] & BIT3) {
                                                RL_ON(11);RL_OFF(2);RL_OFF(3);RL_OFF(4);RL_OFF(5);RL_OFF(6);
                                                RL_OFF(7);RL_OFF(8);RL_OFF(9);RL_OFF(10);RL_OFF(13);
                                        } else {
                                                // Todo:延时30秒，04地址03开关量置0
                                                time_scheduler_start = 1;
                                                time_scheduler_time = 30000;
                                        }

                                        if (rx->data[1] & BIT4) {
                                                RL_ON(8);RL_ON(9);RL_OFF(2);RL_OFF(3);
                                                RL_OFF(4);RL_OFF(5);RL_OFF(6);RL_OFF(7);
                                                RL_OFF(10);
                                        }

                                        break;
                                default:
                                        break;

                                }
                        } else if( rx->pro_id == SHANGHUI_PROTOCOL) {
                                u8* wk8001_data = &rx->data[5];

                                if (wk8001_data[0] == 1)
                                        RL_ON(3);
                                else if (wk8001_data[0] == 0) {
                                        RL_OFF(11);RL_OFF(12);RL_OFF(13);
                                }

                                if (wk8001_data[3] == 0) {
                                        RL_OFF(11);RL_OFF(12);RL_OFF(13);
                                } else if (wk8001_data[3] == 1) {
                                        RL_ON(11);RL_OFF(12);RL_OFF(13);
                                } else if (wk8001_data[3] == 2) {
                                        RL_ON(12);RL_OFF(11);RL_OFF(13);
                                } else if (wk8001_data[3] == 3) {
                                        RL_ON(13);RL_OFF(12);RL_OFF(11);
                                }

                                if (wk8001_data[4] == wk8001_data[5]) {
                                        RL_ON(11);RL_OFF(12);RL_OFF(13);
                                } else if (wk8001_data[4] > wk8001_data[5]) {
                                        RL_ON(13);RL_OFF(12);RL_OFF(11);
                                } else if (wk8001_data[4] < wk8001_data[5]) {
                                        RL_ON(13);RL_OFF(12);RL_OFF(11);
                                }
                        }
                        
                        rx->use = 0;
                }
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

