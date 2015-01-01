#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "bsp_uart1.h"
#include "CRC16.h"
#include "bsp_board_gpio.h"
#include "task_485.h"
#include "bsp_delay.h"
#include "bsp_uart3.h"

static u8 current_protocol = WK8001_PROTOCOL;           // 协议切换
static u8 data_cache[256];                              // 串口数据缓存
static u8 data_idx = 0;
volatile u8 data_02 = 0;        
volatile u8 data_04 = 0;

void uart_sent_data(u8* data,u8 len)
{
        vPortEnterCritical();
        GPIO_SetBits(GPIOA,GPIO_Pin_8);
        GPIO_SetBits(GPIOB,GPIO_Pin_12);
        delay_nms(50);
        for (u8 i = 0; i < len; i++)
                Uart1_send_data(data[i]);
        
        for (u8 i = 0; i < len; i++)
                Uart3_send_data(data[i]);
        
        GPIO_ResetBits(GPIOA, GPIO_Pin_8);
        GPIO_ResetBits(GPIOB,GPIO_Pin_12);
        delay_nms(50);
        vPortExitCritical();
        
}

static u8 time_scheduler_start = 0;
static u16 time_scheduler_time = 0;
void time_scheduler(void)
{
        if (time_scheduler_time)
                time_scheduler_time--;
        
        if ((time_scheduler_start == 1) && (time_scheduler_time == 0)) {
                time_scheduler_start = 0;
                u8 protocol_data[8];
                protocol_data[0] = 0xd8;
                protocol_data[1] = 0;
                protocol_data[2] = 0;
                protocol_data[3] = 0;
                protocol_data[4] = data_04 & (~BIT3);
                protocol_data[5] = 0;
                protocol_data[6] = 0;
                protocol_data[7] = protocol_data[0] ^ protocol_data[1] ^
                                   protocol_data[2] ^ protocol_data[3] ^
                                   protocol_data[4] ^ protocol_data[5] ^
                                   protocol_data[6] ;
                uart_sent_data(protocol_data,8);
        }        
}

static volatile u8 get_device_echo = 0;
void send_protocol_data(void)
{
        u8 protocol_data[20];
        get_device_echo = 0;
        if (current_protocol == WK8001_PROTOCOL)
        {
                protocol_data[0] = 0x41;
                protocol_data[1] = 0x43;
                protocol_data[2] = 0x54;
                protocol_data[3] = 0x11;
                protocol_data[4] = 0x01;        //ID
                
                protocol_data[5] = 0x01;
                protocol_data[6] = 0x01;
                protocol_data[7] = 0x00;
                protocol_data[8] = 0x01;
                protocol_data[9] = 0x19;
                protocol_data[10] = 0x14;
                protocol_data[11] = 0x00;
                protocol_data[12] = 0x30;
                protocol_data[13] = 0x08;
                protocol_data[14] = 0x30;
                protocol_data[15] = 0x12;
                protocol_data[16] = 0x06;
                protocol_data[17] = 0x17;
                crc16_cal(&protocol_data[19], &protocol_data[18], &protocol_data[3], 15);
                uart_sent_data(protocol_data,20);
        } else if (current_protocol == SHANGHUI_PROTOCOL) {
                protocol_data[0] = 0x74;
                protocol_data[1] = 0xAA;
                protocol_data[2] = 0xDE;
                uart_sent_data(protocol_data,3);
        }
}


volatile u8 rs485_data_in = 0;;

void receive_protocol_data(u8 data)
{
        rs485_data_in = 1;
        
        if (current_protocol == WK8001_PROTOCOL)
        {       
                data_cache[data_idx] = data;
                data_idx++;
                if (data_cache[0] != 0x41) {
                        data_idx = 0;
                        return;
                } else if (data_cache[1] != 0x43) {
                        data_idx = 0;
                        return;
                } else if (data_cache[2] != 0x54) {
                        data_idx = 0;
                        return;
                } else {
                        if (data_idx >= 20) {
                                u8 high_byte,low_byte;
                                u8 *wk8001_data;
                                crc16_cal(&high_byte, &low_byte, &data_cache[3], 15);
                                if ((high_byte == data_cache[19]) && (low_byte == data_cache[18])) {
                                        get_device_echo = 1;
                                        wk8001_data = &data_cache[5];
                                        
                                        if (wk8001_data[0] == 1)
                                                RL_ON(11);
                                        else if (wk8001_data[0] == 0) {
                                                RL_OFF(11);
                                                RL_OFF(12);
                                                RL_OFF(13);
                                        }
                                
                                        if (wk8001_data[3] == 0)
                                                RL_ON(11);
                                        else if (wk8001_data[3] == 1) 
                                                RL_ON(11);
                                        else if (wk8001_data[3] == 2) 
                                                RL_ON(12);
                                        else if (wk8001_data[3] == 3) 
                                                RL_ON(13);
                                        
                                        if (wk8001_data[4] == wk8001_data[5])
                                                RL_ON(11);
                                        else if (wk8001_data[4] > wk8001_data[5])
                                                RL_ON(13);
                                        else if (wk8001_data[4] < wk8001_data[5])
                                                RL_ON(13);
                                                
                                }
                        
                        }
                }
        } else if (current_protocol == SHANGHUI_PROTOCOL) {
                data_cache[data_idx] = data;
                data_idx++;
                if ((data_cache[0] < 0x10) || (data_cache[0] > 0x73)) {
                        data_idx = 0;
                        return ;
                }
                
                if (data_idx >= 3) {
                        data_idx = 0;
                        if ((data_cache[0] >= 0x10) && (data_cache[0] <= 0x73)) {
                                if (1) {//data_cache[2] == (data_cache[0] ^ data_cache[1])) {                                
                                        get_device_echo = 1;
                                        switch (data_cache[0]) {
                                                case 0x12:
                                                        data_02 = data_cache[1];
                                                        if (data_cache[1] & BIT1) RL_ON(4); else RL_OFF(4);
                                                        if (data_cache[1] & BIT2) RL_ON(6); else RL_OFF(6);
                                                        if (data_cache[1] & BIT3) RL_ON(3); else RL_OFF(3);
                                                        if (data_cache[1] & BIT4) RL_ON(2); else RL_OFF(2);
                                                        if (data_cache[1] & BIT5) IO_OUTPUT10_OFF(); else IO_OUTPUT10_ON();
                                                        if (data_cache[1] & BIT6) IO_OUTPUT9_OFF(); else IO_OUTPUT9_ON();
                                                        break;
                                                case 0x14:
                                                        data_04 = data_cache[1];
                                                        if (data_cache[1] & BIT1) RL_ON(2); else RL_OFF(2);
                                                
                                                        if (data_cache[1] & BIT2) {
                                                                RL_ON(2);RL_ON(3);
                                                                RL_ON(4);RL_ON(5);
                                                                RL_ON(6);RL_ON(7);
                                                                RL_ON(8);RL_ON(9);
                                                                RL_ON(10);
                                                        }         
                                                        
                                                        if (data_cache[1] & BIT3) { 
                                                                RL_ON(11);RL_OFF(2);
                                                                RL_OFF(2);RL_OFF(3);
                                                                RL_OFF(4);RL_OFF(5);
                                                                RL_OFF(4);RL_OFF(5);
                                                                RL_OFF(6);RL_OFF(7);
                                                                RL_OFF(8);RL_OFF(9);
                                                                RL_OFF(10);RL_OFF(13);
                                                        }else{ 
                                                                // Todo:延时30秒，04地址03开关量置0
                                                                time_scheduler_start = 1;
                                                                time_scheduler_time = 30000;
                                                        }
                                                        
                                                        if (data_cache[1] & BIT4) {
                                                                RL_ON(8); RL_ON(9); 
                                                                RL_OFF(2);RL_OFF(3);
                                                                RL_OFF(4);RL_OFF(5);
                                                                RL_OFF(6);RL_OFF(7);
                                                                RL_OFF(10);
                                                        }
                                                        
                                                        break;
                                                default:
                                                        break;
                                        
                                        }
                                
                                
                                
                                }
                        }
                }
        }
}

void task_485(void* param)
{
        (void)param;
        while(1) {
//                current_protocol = WK8001_PROTOCOL;
//                send_protocol_data();
//                vTaskDelay(200 / portTICK_PERIOD_MS);
                
                current_protocol = SHANGHUI_PROTOCOL;
                send_protocol_data();
                vTaskDelay(300 / portTICK_PERIOD_MS);
                
//                send_protocol_data();
//                vTaskDelay(200 / portTICK_PERIOD_MS);
        }
}

