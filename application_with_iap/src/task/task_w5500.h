#ifndef _TASK_W5500_H
#define _TASK_W5500_H

void w5500_dhcp_thread(void* param);
void w5500_tcp_thread(void* param);
void get_tcp_rev_data(u8 **ppucMBTCPFrame, u16 *usTCPLength);

#endif

