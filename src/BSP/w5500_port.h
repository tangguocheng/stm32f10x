#ifndef _W5500_PORT_H
#define _W5500_PORT_H


#define SERVER_PORT             5000
#define DATA_BUF_SIZE           2048
#define DHCP_RETRY_TIMES	3

#define SOCK_DHCP               0
#define SOCK_TCP                1

#define SERVER_IP               {192,168,1,1}
#define SERVER_PORT             5000
#define LOCAL_PORT              6000

void w5500_dhcp_thread(void* param);
void w5500_tcp_thread(void* param);

#endif


