#ifndef _TASK_485_H
#define _TASK_485_H

#define BIT1  0x01
#define BIT2  0x02
#define BIT3  0x04
#define BIT4  0x08
#define BIT5  0x10
#define BIT6  0x20
#define BIT7  0x40
#define BIT8  0x80

#define WK8001_PROTOCOL         0x01
#define SHANGHUI_PROTOCOL       0x02

void task_485_poll(void* param);
void task_485_send(void* param);
void task_485_receive(void* param);
void receive_protocol_data(u8 data);
void time_scheduler(void);
void gpio_input_process(void);

#endif

