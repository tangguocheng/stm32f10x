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
	u8 buff[2048];                                                          /*����һ��2KB�Ļ���*/	
	switch(getSn_SR(SOCK_UDP))                                                /*��ȡsocket��״̬*/
	{
		case SOCK_CLOSED:                                                        /*socket���ڹر�״̬*/
			socket(SOCK_UDP,Sn_MR_UDP,LOCAL_PORT,0);                              /*��ʼ��socket*/
		  break;
		
		case SOCK_UDP:                                                           /*socket��ʼ�����*/
			if(getSn_IR(SOCK_UDP) & Sn_IR_RECV)
			{
				setSn_IR(SOCK_UDP, Sn_IR_RECV);                                     /*������ж�*/
			}
			if((len=getSn_RX_RSR(SOCK_UDP))>0)                                    /*���յ�����*/
			{
				recvfrom(SOCK_UDP,buff, len, remote_ip,&remote_port);               /*W5500���ռ����������������*/
				buff[len-8]=0x00;                                                    /*����ַ���������*/
				sendto(SOCK_UDP,buff,len-8, remote_ip, remote_port);                /*W5500�ѽ��յ������ݷ��͸�Remote*/
			}
			break;
	}

}

void do_tcp_server(void)
{	
	u16 len=0;  
	switch(getSn_SR(SOCK_TCP))											            	/*��ȡsocket��״̬*/
	{
		case SOCK_CLOSED:													                  /*socket���ڹر�״̬*/
			socket(SOCK_TCP ,Sn_MR_TCP,LOCAL_PORT,Sn_MR_ND);	        /*��socket*/
		  break;     
    
		case SOCK_INIT:														                  /*socket�ѳ�ʼ��״̬*/
			listen(SOCK_TCP);												                /*socket��������*/
		  break;
		
		case SOCK_ESTABLISHED:												              /*socket�������ӽ���״̬*/
		
			if(getSn_IR(SOCK_TCP) & Sn_IR_CON)
			{
				setSn_IR(SOCK_TCP, Sn_IR_CON);								          /*��������жϱ�־λ*/
			}
			len=getSn_RX_RSR(SOCK_TCP);									            /*����lenΪ�ѽ������ݵĳ���*/
			if(len>0)
			{
				recv(SOCK_TCP,w5500_buffer,len);								              	/*��������Client������*/

				send(SOCK_TCP,w5500_buffer,len);									              /*��Client��������*/
		  }
		  break;
		
		case SOCK_CLOSE_WAIT:												                /*socket���ڵȴ��ر�״̬*/
			close(SOCK_TCP);
		  break;
	}
}

/**
*@brief		TCP Client�ػ���ʾ������
*@param		��
*@return	��
*/
void do_tcp_client(void)
{	
        u16 len=0;	

	switch(getSn_SR(SOCK_TCP))								  				         /*��ȡsocket��״̬*/
	{
		case SOCK_CLOSED:											        		         /*socket���ڹر�״̬*/
			socket(SOCK_TCP,Sn_MR_TCP,local_port++,Sn_MR_ND);
		  break;
		
		case SOCK_INIT:													        	         /*socket���ڳ�ʼ��״̬*/
			connect(SOCK_TCP,remote_ip,remote_port);                /*socket���ӷ�����*/ 
		  break;
		
		case SOCK_ESTABLISHED: 												             /*socket�������ӽ���״̬*/
			if(getSn_IR(SOCK_TCP) & Sn_IR_CON)
			{
				setSn_IR(SOCK_TCP, Sn_IR_CON); 							         /*��������жϱ�־λ*/
			}
		
			len=getSn_RX_RSR(SOCK_TCP); 								  	         /*����lenΪ�ѽ������ݵĳ���*/
			if(len>0)
			{
				recv(SOCK_TCP,w5500_buffer,len); 							   		         /*��������Server������*/
				send(SOCK_TCP,w5500_buffer,len);								     	         /*��Server��������*/
			}		  
		  break;
			
		case SOCK_CLOSE_WAIT: 											    	         /*socket���ڵȴ��ر�״̬*/
			close(SOCK_TCP);
		  break;

	}
}

