#ifndef _BSP_USART1_H
#define	_BSP_USART1_H


void Uart1_init(u32 baud,u8 pre_priority,u8 sub_priority);
void Uart1_send_data(u8 data);

#endif


