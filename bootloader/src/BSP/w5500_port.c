#include "stm32f10x.h"
#include "socket.h"
#include "dhcp.h"
#include "proj_conf.h"
#include "w5500_port.h"
#include "delay.h"

static u8 w5500_buffer[DATA_BUF_SIZE];                
static volatile u8 start_dhcp_timer = 0;
static volatile u32 ms_ticks = 0;
static u8 dhcp_ok = 0;
static u8 w5500_state = W5500_STATE_INIT;

wiz_NetInfo w5500_eth_info = {
        .mac = {0x00, 0x08, 0xdc,0x00, 0xab, 0xcd},
        .ip = {192, 168, 1, 123},
        .sn = {255,255,255,0},
        .gw = {192, 168, 1, 1},
        .dns = {0,0,0,0},
        .dhcp = NETINFO_DHCP
};


static void w5500_spi_init(void)
{
        SPI_InitTypeDef SPI_InitStructure;
        GPIO_InitTypeDef GPIO_InitStructure;

        RCC_APB1PeriphClockCmd( RCC_APB1Periph_SPI2, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);

        /* Configure SPIy pins: SCK, MISO and MOSI */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_Init(GPIOB, &GPIO_InitStructure);

        // CS
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOC, &GPIO_InitStructure);
        GPIO_SetBits(GPIOC, GPIO_Pin_9);

        /* SPI Config -------------------------------------------------------------*/
        SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
        SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
        SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
        SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
        SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
        SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
        SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
        SPI_InitStructure.SPI_CRCPolynomial = 7;

        SPI_Init(SPI2, &SPI_InitStructure);
        SPI_Cmd(SPI2, ENABLE);
}

static void w5500_rst_pin_init(void)
{
        GPIO_InitTypeDef GPIO_InitStructure;
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOC, &GPIO_InitStructure);
        GPIO_SetBits(GPIOC, GPIO_Pin_6);
}

static void w5500_hard_reset(void)
{
        GPIO_ResetBits(GPIOC, GPIO_Pin_6);
        delay_nms(1);
        GPIO_SetBits(GPIOC, GPIO_Pin_6);
        delay_nms(1600);
}

static void w5500_cs_select(void)
{
        GPIO_ResetBits(GPIOC, GPIO_Pin_9);
}

static void w5500_cs_deselect(void)
{
        GPIO_SetBits(GPIOC, GPIO_Pin_9);
}

static void w5500_spi_send_byte(u8 byte)
{
        while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

        SPI_I2S_SendData(SPI2, byte);
}

static u8 w5500_spi_read_byte(void)
{
        while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);

        return SPI_I2S_ReceiveData(SPI2);
}

static void ioLibrary_fun_register(void)
{
//        reg_wizchip_cris_cbfunc(vPortEnterCritical, vPortExitCritical);
        reg_wizchip_cs_cbfunc(w5500_cs_select, w5500_cs_deselect);
        reg_wizchip_spi_cbfunc(w5500_spi_read_byte, w5500_spi_send_byte);
}



static void network_init(void)
{
        // Set Network information from netinfo structure
        ctlnetwork(CN_SET_NETINFO, (void*)&w5500_eth_info);

#if DEBUG_OUTPUT_ENABLE == 1
        uint8_t tmpstr[6] = {0,};
        wiz_NetInfo netinfo;
        // Get Network information
        ctlnetwork(CN_GET_NETINFO, (void*)&netinfo);

        // Display Network Information
        ctlwizchip(CW_GET_ID,(void*)tmpstr);

        if(netinfo.dhcp == NETINFO_DHCP)
                LOG_OUT(LOG_INFO "\r\n=== %s NET CONF : DHCP ===\r\n",(char*)tmpstr);
        else
                LOG_OUT(LOG_INFO "\r\n=== %s NET CONF : Static ===\r\n",(char*)tmpstr);

        LOG_OUT(LOG_INFO "MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n",netinfo.mac[0],netinfo.mac[1],netinfo.mac[2],netinfo.mac[3],netinfo.mac[4],netinfo.mac[5]);
        LOG_OUT(LOG_INFO "SIP: %d.%d.%d.%d\r\n", netinfo.ip[0],netinfo.ip[1],netinfo.ip[2],netinfo.ip[3]);
        LOG_OUT(LOG_INFO "GAR: %d.%d.%d.%d\r\n", netinfo.gw[0],netinfo.gw[1],netinfo.gw[2],netinfo.gw[3]);
        LOG_OUT(LOG_INFO "SUB: %d.%d.%d.%d\r\n", netinfo.sn[0],netinfo.sn[1],netinfo.sn[2],netinfo.sn[3]);
        LOG_OUT(LOG_INFO "DNS: %d.%d.%d.%d\r\n", netinfo.dns[0],netinfo.dns[1],netinfo.dns[2],netinfo.dns[3]);
        LOG_OUT(LOG_INFO "===========================\r\n");
#endif
}


static void dhcp_timer_start(void)
{
        start_dhcp_timer = 1;
        ms_ticks = 0;
}

static void dhcp_timer_stop(void)
{
        start_dhcp_timer = 0;
        ms_ticks = 0;
}

void w5500_timer_isr(void)
{
        ms_ticks++;
        if (start_dhcp_timer == 1) {  
                if (ms_ticks % 1000 == 0)
                        DHCP_time_handler();
        }
        
        if (ms_ticks % 1000 == 0) {
                u8 tmp = PHY_LINK_OFF;
                ctlwizchip(CW_GET_PHYLINK, (void*)&tmp);
                if ((tmp == PHY_LINK_OFF) && (w5500_state != W5500_STATE_INIT))                // PHY连接异常
                        w5500_state = W5500_STATE_INIT;
        }
}


void w5500_ip_assign(void)
{
        dhcp_ok = 1;
        DHCP_stop();
        dhcp_timer_stop();

        getIPfromDHCP(w5500_eth_info.ip);
        getGWfromDHCP(w5500_eth_info.gw);
        getSNfromDHCP(w5500_eth_info.sn);
        getDNSfromDHCP(w5500_eth_info.dns);
        w5500_eth_info.dhcp = NETINFO_DHCP;
        /* Network initialization */
        network_init();      // apply from dhcp

        LOG_OUT(LOG_INFO "DHCP LEASED TIME : %ld Sec.\r\n", getDHCPLeasetime());

}

void w5500_ip_conflict(void)
{
        LOG_OUT(LOG_ERR "CONFLICT IP from DHCP\r\n");

        DHCP_init(SOCK_DHCP, w5500_buffer);

        dhcp_timer_start();
        dhcp_ok = 0;
}

static void start_dhcp_serve(void)
{
        dhcp_ok = 0;
        reg_dhcp_cbfunc(w5500_ip_assign, w5500_ip_assign, w5500_ip_conflict);
        DHCP_init(SOCK_DHCP, w5500_buffer);
        dhcp_timer_start();
}

static u8 w5500_init(void)
{
        w5500_spi_init();
        w5500_rst_pin_init();
        w5500_hard_reset();
        ioLibrary_fun_register();

        u8 memsize[2][8] = {{2,2,2,2,2,2,2,2},{2,2,2,2,2,2,2,2}};
        while (ctlwizchip(CW_INIT_WIZCHIP, (void*)memsize) == -1) {
                delay_nms(1000);
                w5500_hard_reset();
                LOG_OUT(LOG_ERR "WIZCHIP Initialized fail.\r\n");
        }

        // 检查PHY连接状态
        u8 tmp = PHY_LINK_OFF;
        do {
                ctlwizchip(CW_GET_PHYLINK, (void*)&tmp);;
                delay_nms(1000);                        // 等待PHY连接
        } while(tmp == PHY_LINK_OFF);

        return (1);
}

static void start_dhcp(void)
{
        // 开启DHCP服务之前，必须设定MAC地址.
        // 在这里读取STM32的ID前40位作为MAC地址
        w5500_eth_info.mac[0] = 0x00;
        w5500_eth_info.mac[1] = *(u8*)(0x1FFFF7E8);
        w5500_eth_info.mac[2] = *(u8*)(0x1FFFF7E9);
        w5500_eth_info.mac[3] = *(u8*)(0x1FFFF7EA);
        w5500_eth_info.mac[4] = *(u8*)(0x1FFFF7EB);
        w5500_eth_info.mac[5] = *(u8*)(0x1FFFF7EC);
        setSHAR(w5500_eth_info.mac);

        start_dhcp_serve();
}

static u8 do_dhcp(void)
{
        static u8 my_dhcp_retry = 0;
        if (dhcp_ok == 1)
                return (1);

        switch(DHCP_run()) {
        case DHCP_IP_ASSIGN:
        case DHCP_IP_CHANGED:
                break;
        case DHCP_IP_LEASED:
                break;
        case DHCP_FAILED:
                my_dhcp_retry++;
                if(my_dhcp_retry > 3) {
                        my_dhcp_retry = 0;
                        DHCP_init(SOCK_DHCP, w5500_buffer);
                        dhcp_timer_start();
                        dhcp_ok = 0;
                }
                break;
        default:
                break;
        }

        return (0);
}


static void w5500_tcp_client(void)
{
        u8 server_ip[4] = SERVER_IP;
        s32 ret;
        u16 size = 0, sentsize=0;
        switch(getSn_SR(SOCK_TCP)) {
        case SOCK_ESTABLISHED :
                if(getSn_IR(SOCK_TCP) & Sn_IR_CON) {
                        setSn_IR(SOCK_TCP,Sn_IR_CON);
                }

                if((size = getSn_RX_RSR(SOCK_TCP)) > 0) {
                        if(size > DATA_BUF_SIZE) size = DATA_BUF_SIZE;
                        ret = recv(SOCK_TCP,w5500_buffer,size);
                        sentsize = 0;
                        while(size != sentsize) {
                                ret = send(SOCK_TCP,w5500_buffer + sentsize,size-sentsize);
                                if(ret < 0) {
                                        close(SOCK_TCP);
                                }
                                sentsize += ret;
                        }
                }
                break;

        case SOCK_CLOSE_WAIT:
                close(SOCK_TCP);
                break;

        case SOCK_INIT:
                connect(SOCK_TCP, server_ip, SERVER_PORT);
                setSn_KPALVTR(SOCK_TCP, 0x01);                  // keepalive 5s
                break;

        case SOCK_CLOSED:
                socket(SOCK_TCP,Sn_MR_TCP,LOCAL_PORT,Sn_MR_ND);
                break;

        default:
                break;
        }
}

void w5500_socket_process(void)
{
        switch (w5500_state) {
        case W5500_STATE_INIT:
                if (w5500_init() == 1) {
                        start_dhcp();
                        w5500_state = W5500_STATE_DHCP;
                }
                break;
                
        case W5500_STATE_DHCP:
                if (do_dhcp() == 1)
                        w5500_state = W5500_STATE_SOCKET;
                break;

        case W5500_STATE_SOCKET:
                w5500_tcp_client(); 
                break;
                
        default:
                break;

        }

}
