#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "socket.h"
#include "dhcp.h"
#include "proj_conf.h"
#include "w5500_port.h"

void w5500_dhcp_thread(void* param)
{
        u8 dhcp_retry = 0;
        (void)param;
        w5500_init();

        while (1) {
                switch(DHCP_run()) {
                case DHCP_IP_ASSIGN:
                case DHCP_IP_CHANGED:
                        goto __final;
                        break;
                case DHCP_FAILED:
                        /* ===== Example pseudo code =====  */
                        // The below code can be replaced your code or omitted.
                        // if omitted, retry to process DHCP
                        dhcp_retry++;
                        if(dhcp_retry > DHCP_RETRY_TIMES) {
                                LOG_OUT(LOG_ERR ">> DHCP %d Failed\r\n", dhcp_retry);
                                dhcp_retry = 0;
                                DHCP_stop();      // if restart, recall DHCP_init()
                                network_init();   // apply the default static network and print out netinfo to serial
                        }
                        break;
                default:
                        break;
                }
        }

__final:
        vTaskDelete(NULL);
}



void w5500_tcp_thread(void* param)
{
        (void)param;
        u8 server_ip[4] = SERVER_IP;			
        while (1) {
                int32_t ret;
                uint16_t size = 0, sentsize=0;
                switch(getSn_SR(SOCK_TCP)) {
                case SOCK_ESTABLISHED :
                        if(getSn_IR(SOCK_TCP) & Sn_IR_CON) {
                                printf("%d:Connected\r\n",SOCK_TCP);
                                setSn_IR(SOCK_TCP,Sn_IR_CON);
                        }
                        
                        if((size = getSn_RX_RSR(SOCK_TCP)) > 0) {
                                if(size > DATA_BUF_SIZE) size = DATA_BUF_SIZE;
                                ret = recv(SOCK_TCP,w5500_buffer,size);
                                if(ret <= 0) printf("ret < 0 \r\n");
                                sentsize = 0;
                                while(size != sentsize) {
                                        ret = send(SOCK_TCP,w5500_buffer + sentsize,size-sentsize);
                                        if(ret < 0) {
                                                close(SOCK_TCP);
                                                printf("ret < 0 \r\n");
                                        }
                                        sentsize += ret; // Don't care SOCKERR_BUSY, because it is zero.
                                }
                        }
                        break;
                case SOCK_CLOSE_WAIT :
                        close(SOCK_TCP);
                        break;
                case SOCK_INIT :													
                        connect(SOCK_TCP, server_ip, SERVER_PORT);			
                        break;
                case SOCK_CLOSED:
                        socket(SOCK_TCP,Sn_MR_TCP,LOCAL_PORT,Sn_MR_ND);
                        break;
                default:
                        break;
                }

        }

}

