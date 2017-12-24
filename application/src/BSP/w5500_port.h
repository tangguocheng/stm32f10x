#ifndef _W5500_PORT_H
#define _W5500_PORT_H


#define SERVER_PORT             5000
#define DATA_BUF_SIZE           2048
#define DHCP_RETRY_TIMES	3

#define SOCK_DHCP               0
#define SOCK_TCP                1

#define SERVER_IP               {192,168,31,61}
#define SERVER_PORT             5000
#define LOCAL_PORT              6000

extern u8 w5500_buffer[]; 

void network_init(void);
void w5500_init(void);


#endif


