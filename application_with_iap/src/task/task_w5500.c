#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "socket.h"
#include "dhcp.h"
#include "mb.h"
#include "proj_conf.h"
#include "bsp_w5500_port.h"
#include "eeprom_mem.h"

void w5500_tcp_thread(void* param);

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

static u16 tcp_rec_data_len = 0;

void get_tcp_rev_data(u8 **ppucMBTCPFrame, u16 *usTCPLength)
{
        vPortEnterCritical();
        *ppucMBTCPFrame = (u8 *)&w5500_buffer[0];
        *usTCPLength = tcp_rec_data_len;
        tcp_rec_data_len = 0;
        vPortExitCritical();
}

void w5500_tcp_thread(void* param)
{
        (void)param;
        u8 server_ip[4] = SERVER_IP;	
        u16 server_port = SERVER_PORT;
//        read_server_ip(server_ip,&server_port);
        u8 socket_first_burn = 0;
        while (1) {
                int32_t ret;
                switch(getSn_SR(SOCK_TCP)) {
                case SOCK_ESTABLISHED :
                        if(getSn_IR(SOCK_TCP) & Sn_IR_CON) {
                                setSn_IR(SOCK_TCP,Sn_IR_CON);
                        }
                        
                        if (socket_first_burn == 1) {
                                socket_first_burn = 0;
                                // send system information to server
                        }
                        
                        if((tcp_rec_data_len = getSn_RX_RSR(SOCK_TCP)) > 0) {
                                if(tcp_rec_data_len > DATA_BUF_SIZE) tcp_rec_data_len = DATA_BUF_SIZE;
                                ret = recv(SOCK_TCP,w5500_buffer,tcp_rec_data_len);
                                if (ret > 0) {
                                        xMBPortEventPost(EV_FRAME_RECEIVED);
                                }
                        }
                        break;
                case SOCK_CLOSE_WAIT :
                        close(SOCK_TCP);
                        break;
                case SOCK_INIT :
                        socket_first_burn = 1;                        
                        connect(SOCK_TCP, server_ip, server_port);			
                        break;
                case SOCK_CLOSED:
                        socket(SOCK_TCP,Sn_MR_TCP,LOCAL_PORT,Sn_MR_ND);
                        break;
                default:
                        break;
                }
        }
}


void w5500_udp_thread(void* param)
{
	u16 len=0;
	u8 buff[2048];                                                          /*定义一个2KB的缓存*/	
	switch(getSn_SR(SOCK_UDP))                                                /*获取socket的状态*/
	{
		case SOCK_CLOSED:                                                        /*socket处于关闭状态*/
			socket(SOCK_UDP,Sn_MR_UDP,LOCAL_PORT,0);                              /*初始化socket*/
		  break;
		
		case SOCK_UDP:                                                           /*socket初始化完成*/
			if(getSn_IR(SOCK_UDP) & Sn_IR_RECV)
			{
				setSn_IR(SOCK_UDP, Sn_IR_RECV);                                     /*清接收中断*/
			}
			if((len=getSn_RX_RSR(SOCK_UDP))>0)                                    /*接收到数据*/
			{
				recvfrom(SOCK_UDP,buff, len, remote_ip,&remote_port);               /*W5500接收计算机发送来的数据*/
				buff[len-8]=0x00;                                                    /*添加字符串结束符*/
				sendto(SOCK_UDP,buff,len-8, remote_ip, remote_port);                /*W5500把接收到的数据发送给Remote*/
			}
			break;
	}

}

void do_tcp_server(void)
{	
	u16 len=0;  
	switch(getSn_SR(SOCK_TCP))											            	/*获取socket的状态*/
	{
		case SOCK_CLOSED:													                  /*socket处于关闭状态*/
			socket(SOCK_TCP ,Sn_MR_TCP,LOCAL_PORT,Sn_MR_ND);	        /*打开socket*/
		  break;     
    
		case SOCK_INIT:														                  /*socket已初始化状态*/
			listen(SOCK_TCP);												                /*socket建立监听*/
		  break;
		
		case SOCK_ESTABLISHED:												              /*socket处于连接建立状态*/
		
			if(getSn_IR(SOCK_TCP) & Sn_IR_CON)
			{
				setSn_IR(SOCK_TCP, Sn_IR_CON);								          /*清除接收中断标志位*/
			}
			len=getSn_RX_RSR(SOCK_TCP);									            /*定义len为已接收数据的长度*/
			if(len>0)
			{
				recv(SOCK_TCP,w5500_buffer,len);								              	/*接收来自Client的数据*/

				send(SOCK_TCP,w5500_buffer,len);									              /*向Client发送数据*/
		  }
		  break;
		
		case SOCK_CLOSE_WAIT:												                /*socket处于等待关闭状态*/
			close(SOCK_TCP);
		  break;
	}
}

/**
*@brief		TCP Client回环演示函数。
*@param		无
*@return	无
*/
void do_tcp_client(void)
{	
        u16 len=0;	

	switch(getSn_SR(SOCK_TCP))								  				         /*获取socket的状态*/
	{
		case SOCK_CLOSED:											        		         /*socket处于关闭状态*/
			socket(SOCK_TCP,Sn_MR_TCP,local_port++,Sn_MR_ND);
		  break;
		
		case SOCK_INIT:													        	         /*socket处于初始化状态*/
			connect(SOCK_TCP,remote_ip,remote_port);                /*socket连接服务器*/ 
		  break;
		
		case SOCK_ESTABLISHED: 												             /*socket处于连接建立状态*/
			if(getSn_IR(SOCK_TCP) & Sn_IR_CON)
			{
				setSn_IR(SOCK_TCP, Sn_IR_CON); 							         /*清除接收中断标志位*/
			}
		
			len=getSn_RX_RSR(SOCK_TCP); 								  	         /*定义len为已接收数据的长度*/
			if(len>0)
			{
				recv(SOCK_TCP,w5500_buffer,len); 							   		         /*接收来自Server的数据*/
				send(SOCK_TCP,w5500_buffer,len);								     	         /*向Server发送数据*/
			}		  
		  break;
			
		case SOCK_CLOSE_WAIT: 											    	         /*socket处于等待关闭状态*/
			close(SOCK_TCP);
		  break;

	}
}

