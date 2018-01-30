#include "stm32f10x.h"
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "socket.h"
#include "dhcp.h"
#include "mb.h"
#include "proj_conf.h"
#include "bsp_w5500_port.h"
#include "eeprom_mem.h"
#include "device_manager.h"

static u8 udp_server_ip[4] = {192,168,1,99};								

static u16 tcp_rec_data_len = 0;

void get_tcp_rev_data(u8 **ppucMBTCPFrame, u16 *usTCPLength)
{
        vPortEnterCritical();
        *ppucMBTCPFrame = (u8 *)&w5500_buffer[0];
        *usTCPLength = tcp_rec_data_len;
        tcp_rec_data_len = 0;
        vPortExitCritical();
}

void modbus_tcp_thread(void* param)
{
        (void)param;
        while (1) {
                int32_t ret;
                switch(getSn_SR(MODBUS_TCP_SOCKET)) {
                case SOCK_ESTABLISHED :
                        if(getSn_IR(MODBUS_TCP_SOCKET) & Sn_IR_CON) {
                                setSn_IR(MODBUS_TCP_SOCKET,Sn_IR_CON);
         
                        }
                        
                        if((tcp_rec_data_len = getSn_RX_RSR(MODBUS_TCP_SOCKET)) > 0) {
                                if(tcp_rec_data_len > DATA_BUF_SIZE) tcp_rec_data_len = DATA_BUF_SIZE;
                                ret = recv(MODBUS_TCP_SOCKET,w5500_buffer,tcp_rec_data_len);
                                if (ret > 0) {
                                        xMBPortEventPost(EV_FRAME_RECEIVED);
                                }
                        }
                        break;
                        
                case SOCK_CLOSE_WAIT :
                        close(MODBUS_TCP_SOCKET);
                        break;
                
                case SOCK_INIT :           
                        listen(MODBUS_TCP_SOCKET);                        
//                        connect(MODBUS_TCP_SOCKET, udp_server_ip, MODBUS_TCP_PORT);   
                        break;
                
                case SOCK_CLOSED:
                        socket(MODBUS_TCP_SOCKET,Sn_MR_TCP,MODBUS_TCP_PORT,Sn_MR_ND);
                        setSn_KPALVTR(MODBUS_TCP_SOCKET, 0x01);
                        break;
                
                default:
                        break;
                }
                                
                vTaskDelay(20 / portTICK_PERIOD_MS);
        }
}

void monitor_tcp_thread(void* param)
{
        u8 changed = 0;
        u16 len = 0;
        u8 buff[40];
        while(1) {
        switch(getSn_SR(MONITOR_TCP_SOCKET)) {
                case SOCK_ESTABLISHED :
                        if(getSn_IR(MONITOR_TCP_SOCKET) & Sn_IR_CON) {
                                setSn_IR(MONITOR_TCP_SOCKET,Sn_IR_CON);
                        }
                        
                        device_monitor(buff, &len, &changed);
                        
                        if (changed)
                                send(MONITOR_TCP_SOCKET, (UCHAR *)buff, len);
                        
                        break;
                        
                case SOCK_CLOSE_WAIT :
                        close(MONITOR_TCP_SOCKET);
                        break;
                
                case SOCK_INIT :                       
                        connect(MONITOR_TCP_SOCKET, udp_server_ip, MONITOR_TCP_PORT);
                        break;
                
                case SOCK_CLOSED:
                        socket(MONITOR_TCP_SOCKET, Sn_MR_TCP, MY_TCP_SERVER_PORT, Sn_MR_ND);
                        setSn_KPALVTR(MONITOR_TCP_SOCKET, 0x01);
                        break;
                
                default:
                        break;
                }
                
                vTaskDelay(20 / portTICK_PERIOD_MS);
        }
}

void w5500_dhcp_thread(void* param)
{
        u8 dhcp_retry = 0;
        (void)param;
        w5500_init();
        while (1) {
                switch(DHCP_run()) {
                case DHCP_IP_ASSIGN:
                case DHCP_IP_CHANGED:
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
                
                vTaskDelay(100 * portTICK_PERIOD_MS);
        }        
}

TaskHandle_t modbus_tcp_thread_handle = NULL;  
TaskHandle_t monitor_tcp_thread_handle = NULL; 
void w5500_udp_thread(void* param)
{
	u16 len=0;                                                            
	u8 buff[40];      
        u16 remote_port = 5000;
        if (modbus_tcp_thread_handle == NULL)                                                        
                xTaskCreate(modbus_tcp_thread, "tcp_task", configMINIMAL_STACK_SIZE,NULL, configTCP_PRIORITIES, &modbus_tcp_thread_handle);
                                               
        while(1) {
                switch(getSn_SR(MY_UDP_SOCKET)) {                                                                     
                        case SOCK_CLOSED:                                             
                                socket(MY_UDP_SOCKET,Sn_MR_UDP,UDP_PORT,0);              
                        break;                                                      
                                                                                      
                        case SOCK_UDP:                                                
                                if(getSn_IR(MY_UDP_SOCKET) & Sn_IR_RECV) {                                                     
                                        setSn_IR(MY_UDP_SOCKET, Sn_IR_RECV);               
                                }                                                     
                                                                                      
                                if((len = getSn_RX_RSR(MY_UDP_SOCKET))>0) {
                                        if (len > 40) len = 40;
                                        recvfrom(MY_UDP_SOCKET, buff, len, udp_server_ip, &remote_port);             
                                        if (strstr((char *)buff,"SEARCH RCU") != NULL) { 
                                                get_device_info(buff,(u8 *)&len);                                                
                                                sendto(MY_UDP_SOCKET, buff, len, udp_server_ip, 9001);
                                                // create modbus tcp thread
                                                if (monitor_tcp_thread_handle == NULL)           
                                                        xTaskCreate(monitor_tcp_thread, "monitor_task", configMINIMAL_STACK_SIZE,NULL, configMONITOR_TCP_PRIORITIES, &monitor_tcp_thread_handle);

                                        } else if (len >= 24){
                                                u8 building_num = buff[18];
                                                u8 floor_num = buff[19];
                                                u8 room_num = buff[20];
                                                if (device_check_locate(building_num,floor_num,room_num)) {
                                                        update_device_info(buff);
                                                        // close socket
                                                        close(MODBUS_TCP_SOCKET);
                                                        close(MONITOR_TCP_SOCKET);
                                                }
                                        }                                               
                                }
                        break;
                }
                
                vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
}

void do_tcp_server(void)
{	
	u16 len=0;  
	switch(getSn_SR(MODBUS_TCP_SOCKET))									
	{
		case SOCK_CLOSED:												
			socket(MODBUS_TCP_SOCKET ,Sn_MR_TCP,MY_TCP_SERVER_PORT,Sn_MR_ND);	        
		  break;     
    
		case SOCK_INIT:													
			listen(MODBUS_TCP_SOCKET);										
		  break;
		
		case SOCK_ESTABLISHED:												
			if(getSn_IR(MODBUS_TCP_SOCKET) & Sn_IR_CON)
			{
				setSn_IR(MODBUS_TCP_SOCKET, Sn_IR_CON);								
			}
			len=getSn_RX_RSR(MODBUS_TCP_SOCKET);									
			if(len>0)
			{
				recv(MODBUS_TCP_SOCKET,w5500_buffer,len);							

				send(MODBUS_TCP_SOCKET,w5500_buffer,len);							
                        }
		  break;
		
		case SOCK_CLOSE_WAIT:												
			close(MODBUS_TCP_SOCKET);
		  break;
	}
}

