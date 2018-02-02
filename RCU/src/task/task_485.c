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
volatile u8 rs485_data_in = 0;
volatile u8 data_02 = 0;
volatile u8 data_04 = 0;
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
                time_scheduler_start = 0;
                time_scheduler_flag = 0;
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

static u8 key_disable = 0;
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
                last_PA4 = PA4;last_PA5 = PA5;
                return;
        }

        static u16 delay_off_time_D12 = 0;
        static u8 start_delay_D12 = 0;

        if (PD12 != last_PD12) {
                last_PD12 = PD12;
                if (PD12 == Bit_RESET) {                        
                        start_delay_D12 = 0;
                        delay_off_time_D12 = 0;
                        RL_ON(1);RL_ON(2);RL_ON(3);
                        RL_ON(7);RL_ON(8);RL_ON(11);
                        GPIO_SetBits(GPIOE,GPIO_Pin_0);
                        key_disable &= (~(KEY_0x12 | KEY_0x14 | KEY_wkq));
                        u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                        protocol_data_wait_sent[idx].use = 1;;
                        u8* protocol_data = protocol_data_wait_sent[idx].data;
                        protocol_data[0] = 0xd8;
                        data_02 |= BIT3 | BIT4;
                        data_04 &= (~BIT1);
                        protocol_data[3] = data_02;
                        protocol_data[5] = data_04;
                        protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^ protocol_data[2] ^ 
                                           protocol_data[3] ^ protocol_data[4] ^ protocol_data[5] ^
                                           protocol_data[6] ;

                        protocol_data_wait_sent[idx].data_len = 8;
                        void* ptr = &protocol_data_wait_sent[idx];
                        rtl = send_queue_item(1,ptr);
                        
                        idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                        protocol_data_wait_sent[idx].use = 1;;
                        protocol_data = protocol_data_wait_sent[idx].data;

                        protocol_data[0] = 0x41;protocol_data[1] = 0x43;protocol_data[2] = 0x54;
                        protocol_data[3] = 0x11;

                        copy_wk_data(&protocol_data[4]);

                        protocol_data[5] = 0x01;protocol_data[6] = 0x00;protocol_data[7] = 0x00;
                        protocol_data[8] = 0x01;protocol_data[9] = 0x19;protocol_data[10] = 0x19;
                        
                        crc16_cal(&protocol_data[19], &protocol_data[18], &protocol_data[3], 15);

                        protocol_data_wait_sent[idx].data_len = 20;
                        ptr = &protocol_data_wait_sent[idx];
                        rtl = send_queue_item(1,ptr);
                        
                } else if (PD12 == Bit_SET) {
                        key_disable |= (KEY_0x12 | KEY_0x14 | KEY_wkq);                        
                        start_delay_D12 = 1;
                        delay_off_time_D12 = 60000;
                }
        }

        if (delay_off_time_D12)
                delay_off_time_D12--;

        if ((start_delay_D12 == 1) && (delay_off_time_D12 == 0)) {
                start_delay_D12 = 0;
                RL_OFF(1);RL_OFF(6);RL_OFF(10);RL_OFF(14);
                RL_OFF(2);RL_OFF(7);RL_OFF(11);RL_OFF(15);
                RL_OFF(3);RL_OFF(8);RL_OFF(12);RL_OFF(16);
                RL_OFF(4);RL_OFF(9);RL_OFF(13);RL_OFF(17);
                RL_OFF(5);GPIO_ResetBits(GPIOE,GPIO_Pin_0);

                u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                protocol_data_wait_sent[idx].use = 1;;
                u8* protocol_data = protocol_data_wait_sent[idx].data;
                protocol_data[0] = 0xd8;
                data_02 = 0;
                data_04 = 0;
                protocol_data[3] = data_02;
                protocol_data[5] = data_04;
                protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^ protocol_data[2] ^ 
                                   protocol_data[3] ^ protocol_data[4] ^ protocol_data[5] ^
                                   protocol_data[6] ;
                protocol_data_wait_sent[idx].data_len = 8;
                void* ptr = &protocol_data_wait_sent[idx];
                rtl = send_queue_item(1,ptr);
                
                idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                protocol_data_wait_sent[idx].use = 1;;
                protocol_data = protocol_data_wait_sent[idx].data;

                protocol_data[0] = 0x41;protocol_data[1] = 0x43;protocol_data[2] = 0x54;
                protocol_data[3] = 0x11;

                copy_wk_data(&protocol_data[4]);

                protocol_data[5] = 0x00;protocol_data[6] = 0x01;protocol_data[7] = 0x00;
                protocol_data[8] = 0x01;protocol_data[9] = 0x19;protocol_data[10] = 0x19;
                
                crc16_cal(&protocol_data[19], &protocol_data[18], &protocol_data[3], 15);

                protocol_data_wait_sent[idx].data_len = 20;
                ptr = &protocol_data_wait_sent[idx];
                rtl = send_queue_item(1,ptr);
        }


        static u16 delay_off_time_A4 = 0;
        static u8 start_delay_A4 = 0;
        if ((PA4 != last_PA4) && (PD12 == Bit_SET)) {
                last_PA4 = PA4;
                if (PA4 == Bit_RESET) {
                        start_delay_A4 = 1;
                        delay_off_time_A4 = 60000;
                        RL_ON(2);
                        u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                        protocol_data_wait_sent[idx].use = 1;;
                        u8* protocol_data = protocol_data_wait_sent[idx].data;
                        protocol_data[0] = 0xd8;
                        data_02 |= BIT4;
                        data_04 |= BIT4;
                        protocol_data[3] = data_02;
                        protocol_data[5] = data_04;
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
                if (PD12 != Bit_RESET) {
                        RL_OFF(2);
                        u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                        protocol_data_wait_sent[idx].use = 1;;
                        u8* protocol_data = protocol_data_wait_sent[idx].data;
                        protocol_data[0] = 0xd8;
                        data_02 &= (~BIT4);
                        data_04 &= (~BIT4);
                        protocol_data[3] = data_02;
                        protocol_data[5] = data_04;
                        protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^ protocol_data[2] ^ 
                                           protocol_data[3] ^ protocol_data[4] ^ protocol_data[5] ^
                                           protocol_data[6] ;
                        protocol_data_wait_sent[idx].data_len = 8;
                        void* ptr = &protocol_data_wait_sent[idx];
                        rtl = send_queue_item(1,ptr);
                }
        }

        static u8 start_delay_A5 = 0;
        static u16 delay_off_time_A5 = 0;

        if (PD12 == Bit_SET)
                PA5_enable = 0;
        else
                PA5_enable = 1;
      
        if ((PA5 != last_PA5) && (PA5_enable == 1)) {
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
                } else
                if (delay_cnt == 200) {
                        current_protocol = WK8001_PROTOCOL;
                        device_poll();
                        vTaskDelay(90 / portTICK_PERIOD_MS);
                        delay_cnt = 0;
                }                         
        }
}

static u8 last_data3 = 4;
static u8 last_data45 = 3;
static u8 last_data0 = 3;
void task_485_receive(void* param)
{
        receive_485_queue = xQueueCreate(10, sizeof(struct portocol_data_frame *));
        while(1) {
                static struct portocol_data_frame *rx = NULL;
                if ( xQueueReceive(receive_485_queue, &rx, portMAX_DELAY) == pdTRUE ) {
                        if (rx->pro_id == SHANGHUI_PROTOCOL) {
                                switch (rx->data[0]) {
                                case 0x12:
                                        if (key_disable & KEY_0x12)
                                                rx->data[1] &= (~(BIT1 | BIT2 | BIT3 | BIT4));
                                        
                                        if (rx->data[1] & BIT1) {                                                
                                                u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                                                protocol_data_wait_sent[idx].use = 1;;
                                                u8* protocol_data = protocol_data_wait_sent[idx].data;
                                                protocol_data[0] = 0xd8;
                                                data_02 = (RL_STATE(4) ? data_02 & (~BIT1) : data_02 | BIT1);
                                                protocol_data[3] = data_02;
                                                protocol_data[5] = data_04;
                                                protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^ protocol_data[2] ^ 
                                                                   protocol_data[3] ^ protocol_data[4] ^ protocol_data[5] ^
                                                                   protocol_data[6] ;
                                                protocol_data_wait_sent[idx].data_len = 8;
                                                RL_TOGGLE(4);
                                                void* ptr = &protocol_data_wait_sent[idx];
                                                send_queue_item(0,ptr);
                                        }
                                        if (rx->data[1] & BIT2) {
                                                u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                                                protocol_data_wait_sent[idx].use = 1;
                                                u8* protocol_data = protocol_data_wait_sent[idx].data;
                                                protocol_data[0] = 0xd8;
                                                data_02 = (RL_STATE(6) ? data_02 & (~BIT2) : data_02 | BIT2);
                                                protocol_data[3] = data_02;
                                                protocol_data[5] = data_04;
                                                protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^ protocol_data[2] ^ 
                                                                   protocol_data[3] ^ protocol_data[4] ^ protocol_data[5] ^
                                                                   protocol_data[6] ;
                                                protocol_data_wait_sent[idx].data_len = 8;
                                                RL_TOGGLE(6);
                                                void* ptr = &protocol_data_wait_sent[idx];
                                                send_queue_item(0,ptr);
                                        }
                                        if (rx->data[1] & BIT3) {                                                
                                                u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                                                protocol_data_wait_sent[idx].use = 1;;
                                                u8* protocol_data = protocol_data_wait_sent[idx].data;
                                                protocol_data[0] = 0xd8;
                                                data_02 = (RL_STATE(3) ? data_02 & (~BIT3) : data_02 | BIT3);
                                                protocol_data[3] = data_02;
                                                protocol_data[5] = data_04;
                                                protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^ protocol_data[2] ^ 
                                                                   protocol_data[3] ^ protocol_data[4] ^ protocol_data[5] ^
                                                                   protocol_data[6] ;
                                                protocol_data_wait_sent[idx].data_len = 8;                                                
                                                RL_TOGGLE(3);                                                
                                                void* ptr = &protocol_data_wait_sent[idx];
                                                send_queue_item(0,ptr);
                                        }

                                        if (rx->data[1] & BIT4) {                                              
                                                u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                                                protocol_data_wait_sent[idx].use = 1;;
                                                u8* protocol_data = protocol_data_wait_sent[idx].data;
                                                protocol_data[0] = 0xd8;
                                                data_02 = (RL_STATE(2) ? data_02 & (~BIT4) : data_02 | BIT4);
                                                data_04 = (RL_STATE(2) ? data_04 & (~BIT4) : data_04 | BIT4);
                                                protocol_data[3] = data_02;
                                                protocol_data[5] = data_04;
                                                protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^ protocol_data[2] ^ 
                                                                   protocol_data[3] ^ protocol_data[4] ^ protocol_data[5] ^
                                                                   protocol_data[6] ;
                                                protocol_data_wait_sent[idx].data_len = 8;
                                                RL_TOGGLE(2);
                                                void* ptr = &protocol_data_wait_sent[idx];
                                                send_queue_item(0,ptr);
                                        }

                                        if (rx->data[1] & BIT5) {
                                                ((GPIO_ReadOutputDataBit(GPIOE,GPIO_Pin_1 ) == Bit_SET) ? GPIO_ResetBits(GPIOE,GPIO_Pin_1) : GPIO_SetBits(GPIOE,GPIO_Pin_1));
                                                ((GPIO_ReadOutputDataBit(GPIOE,GPIO_Pin_2 ) == Bit_SET) ? GPIO_ResetBits(GPIOE,GPIO_Pin_2) : GPIO_SetBits(GPIOE,GPIO_Pin_2));
                                                u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                                                protocol_data_wait_sent[idx].use = 1;;
                                                u8* protocol_data = protocol_data_wait_sent[idx].data;
                                                protocol_data[0] = 0xd8;
                                                if (GPIO_ReadOutputDataBit(GPIOE,GPIO_Pin_1 )  == Bit_RESET) {
                                                        data_02 &= (~BIT5);
//                                                        PA5_enable = 0;
                                                } else {
                                                        GPIO_ResetBits(GPIOE,GPIO_Pin_3);
                                                        GPIO_ResetBits(GPIOE,GPIO_Pin_4);        
                                                        data_02 = (data_02 | BIT5) & (~BIT6);
                                                        PA5_enable = 1;
                                                }
                                                
                                                protocol_data[3] = data_02;
                                                protocol_data[5] = data_04;
                                                protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^ protocol_data[2] ^ 
                                                                   protocol_data[3] ^ protocol_data[4] ^ protocol_data[5] ^
                                                                   protocol_data[6] ;
                                                protocol_data_wait_sent[idx].data_len = 8;
                                                void* ptr = &protocol_data_wait_sent[idx];
                                                send_queue_item(0,ptr);
                                        }

                                        if (rx->data[1] & BIT6) {
                                                ((GPIO_ReadOutputDataBit(GPIOE,GPIO_Pin_3 ) == Bit_SET) ? GPIO_ResetBits(GPIOE,GPIO_Pin_3) : GPIO_SetBits(GPIOE,GPIO_Pin_3));
                                                ((GPIO_ReadOutputDataBit(GPIOE,GPIO_Pin_4 ) == Bit_SET) ? GPIO_ResetBits(GPIOE,GPIO_Pin_4) : GPIO_SetBits(GPIOE,GPIO_Pin_4));
                                                u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                                                protocol_data_wait_sent[idx].use = 1;;
                                                u8* protocol_data = protocol_data_wait_sent[idx].data;
                                                protocol_data[0] = 0xd8;
                                                if (GPIO_ReadOutputDataBit(GPIOE,GPIO_Pin_3 ) == Bit_RESET) {
                                                        PA5_enable = 1;
                                                        data_02 &= (~BIT6);
                                                } else {
                                                        GPIO_ResetBits(GPIOE,GPIO_Pin_1);
                                                        GPIO_ResetBits(GPIOE,GPIO_Pin_2);        
                                                        data_02 = (data_02 | BIT6) & (~BIT5);
                                                        PA5_enable = 0;
                                                }
                                                
                                                protocol_data[3] = data_02;
                                                protocol_data[5] = data_04;
                                                protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^ protocol_data[2] ^ 
                                                                   protocol_data[3] ^ protocol_data[4] ^ protocol_data[5] ^
                                                                   protocol_data[6] ;
                                                protocol_data_wait_sent[idx].data_len = 8;
                                                void* ptr = &protocol_data_wait_sent[idx];
                                                send_queue_item(0,ptr);
                                        }
                                        break;
                                        
                                case 0x14:
                                        if (key_disable & KEY_0x14)
                                                rx->data[1] &= (~(BIT1 | BIT2 | BIT3 | BIT4));
                                        
                                        if (rx->data[1] & BIT4) {
                                                RL_TOGGLE(2);
                                                u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                                                protocol_data_wait_sent[idx].use = 1;;
                                                u8* protocol_data = protocol_data_wait_sent[idx].data;
                                                protocol_data[0] = 0xd8;
                                                data_02 = (RL_STATE(2) ? data_02 | BIT4 : data_02 & (~BIT4));
                                                data_04 = (RL_STATE(2) ? data_04 | BIT4 : data_04 & (~BIT4));
                                                protocol_data[3] = data_02;
                                                protocol_data[5] = data_04;
                                                protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^ protocol_data[2] ^ 
                                                                   protocol_data[3] ^ protocol_data[4] ^ protocol_data[5] ^
                                                                   protocol_data[6] ;
                                                protocol_data_wait_sent[idx].data_len = 8;
                                                
                                                void* ptr = &protocol_data_wait_sent[idx];
                                                send_queue_item(0,ptr);
                                        }

                                        if (rx->data[1] & BIT3) {
                                                u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                                                protocol_data_wait_sent[idx].use = 1;;
                                                u8* protocol_data = protocol_data_wait_sent[idx].data;
                                                protocol_data[0] = 0xd8;
                                                if (light_state_word == 0) {
                                                        read_state_word = 0;
                                                        sleep_state_word = 0;
                                                        light_state_word = 1;
                                                        data_02 = data_02 | BIT4;
                                                        data_04 = BIT3 | BIT4;
                                                        RL_ON(2);RL_ON(7);RL_ON(8);RL_ON(9);RL_ON(10);
                                                } else if (light_state_word == 1) {
                                                        light_state_word = 0;
                                                        data_02 = data_02 & (~BIT4);
                                                        data_04 = data_04 & (~(BIT3 | BIT4));
                                                        RL_OFF(2);RL_OFF(7);RL_OFF(8);RL_OFF(9);RL_OFF(10);
                                                }
                                                protocol_data[3] = data_02;
                                                protocol_data[5] = data_04;
                                                protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^ protocol_data[2] ^ 
                                                                   protocol_data[3] ^ protocol_data[4] ^ protocol_data[5] ^
                                                                   protocol_data[6] ;
                                                protocol_data_wait_sent[idx].data_len = 8;
                                                void* ptr = &protocol_data_wait_sent[idx];
                                                send_queue_item(0,ptr);
                                     
                                        }

                                        if (rx->data[1] & BIT2) {
                                                read_state_word = 0;
                                                light_state_word = 0;
                                                sleep_state_word = 1;
                                                RL_ON(11);RL_OFF(2);RL_OFF(3);RL_OFF(4);RL_OFF(5);RL_OFF(6);
                                                RL_OFF(7);RL_OFF(8);RL_OFF(9);RL_OFF(10);RL_OFF(12);RL_OFF(13);
                                                u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                                                protocol_data_wait_sent[idx].use = 1;;
                                                u8* protocol_data = protocol_data_wait_sent[idx].data;
                                                protocol_data[0] = 0xd8;
                                                data_02 = 0x00;
                                                data_04 = data_04 & (~(BIT1 | BIT3 | BIT4));
                                                data_04 |= BIT2;
                                                protocol_data[3] = data_02;
                                                protocol_data[5] = data_04;
                                                protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^ protocol_data[2] ^ 
                                                                   protocol_data[3] ^ protocol_data[4] ^ protocol_data[5] ^
                                                                   protocol_data[6] ;
                                                protocol_data_wait_sent[idx].data_len = 8;
                                                void* ptr = &protocol_data_wait_sent[idx];
                                                send_queue_item(0,ptr);
                                                time_scheduler_start = 1;
                                                time_scheduler_time = 10000;
                                        }

                                        if (rx->data[1] & BIT1) {
                                                u8 idx = get_avalid_frame(protocol_data_wait_sent,SEND_FRAME_LEN);
                                                protocol_data_wait_sent[idx].use = 1;;
                                                u8* protocol_data = protocol_data_wait_sent[idx].data;
                                                protocol_data[0] = 0xd8;
                                                if (read_state_word == 0) {
                                                        read_state_word = 1;
                                                        light_state_word = 0;
                                                        sleep_state_word = 0;
                                                        data_02 = data_02 & (~BIT4);
                                                        data_04 |= BIT1;
                                                        data_04 &= (~(BIT2 | BIT3 | BIT4));
                                                        RL_ON(8);RL_ON(9);RL_OFF(2);RL_OFF(7);RL_OFF(10);
                                                } else if (read_state_word == 1) {
                                                        read_state_word = 0;
                                                        RL_OFF(8);RL_OFF(9);
                                                        data_04 &= (~BIT1);
                                                }
                                                protocol_data[3] = data_02;
                                                protocol_data[5] = data_04;
                                                protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^ protocol_data[2] ^ 
                                                                   protocol_data[3] ^ protocol_data[4] ^ protocol_data[5] ^
                                                                   protocol_data[6] ;
                                                protocol_data_wait_sent[idx].data_len = 8;
                                                void* ptr = &protocol_data_wait_sent[idx];
                                                send_queue_item(0,ptr);

                                        }

                                        break;
                                default:
                                        break;

                                }
                        } else if( rx->pro_id == WK8001_PROTOCOL) {
                                if (!(key_disable & KEY_wkq)) {
                                        u8* wk8001_data = &rx->data[5];
                                        
                                        update_wk_data(&rx->data[4]);
                                        
                                        if (last_data3 == 4)
                                                last_data3 = wk8001_data[3];
                                        else if (last_data3 != wk8001_data[3]) {
                                                last_data3 = wk8001_data[3];
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
                                        }
                                        
                                        if (last_data0 != wk8001_data[0]) {
                                                last_data0 = wk8001_data[0];
                                                if (wk8001_data[0] == 1)
                                                        RL_ON(13);
                                        }
                                        
                                        if (wk8001_data[0] == 0) {
                                                wk8001_time_cnt = 0;
                                                wk8001_time_start = 0;
                                                RL_OFF(11);RL_OFF(12);RL_OFF(13);
                                        }
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

