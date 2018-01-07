#ifndef _BSP_USART3_H
#define	_BSP_USART3_H

void Uart3_init(u32 baud,u8 pre_priority,u8 sub_priority);
void Uart3_send_data(u8 data);


#endif /* __USART3_H */
