#ifndef _BSP_W5500_PORT_H
#define _BSP_W5500_PORT_H
#include "socket.h"
#define DATA_BUF_SIZE           2048
#define DHCP_RETRY_TIMES	3

#define MY_DHCP_SOCKET               3
#define MODBUS_TCP_SOCKET            1
#define MY_UDP_SOCKET                2
#define MONITOR_TCP_SOCKET           0

#define SERVER_IP               {192,168,1,17}
#define MODBUS_TCP_PORT         502
#define MONITOR_TCP_PORT        8000
#define UDP_PORT                9000
#define MY_TCP_SERVER_PORT      666

#define USE_DHCP                0
extern u8 w5500_buffer[];
extern wiz_NetInfo w5500_eth_info;
void network_init(void);
void w5500_init(void);
void w5500_ip_assign(void);
#endif


