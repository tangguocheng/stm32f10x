#include "stm32f10x.h"
#include "freeRTOS.h"
#include "task.h"
#include "IAP.h"
#include "bsp_eeprom_24xx.h"
#include "bsp_w5500_port.h"
#include "eeprom_mem.h"



void ee_write_bytes_os(u8 *write_buf, u16 addr, u16 buf_len)
{
        EEPROM_ENTER_CRITICAL();
        ee_write_byts(write_buf,addr,buf_len);
        EEPROM_EXIT_CRITICAL();
}

void ee_read_bytes_os(u8 *write_buf, u16 addr, u16 buf_len)
{
        EEPROM_ENTER_CRITICAL();
        ee_read_byts(write_buf,addr,buf_len);
        EEPROM_EXIT_CRITICAL();
}

void update_server_ip(u8* ip, u16 port)
{
        u8 eeprom_data[6];
        eeprom_data[0] = ip[0];
        eeprom_data[1] = ip[1];
        eeprom_data[2] = ip[2];
        eeprom_data[3] = ip[3];
        eeprom_data[4] = (u8)(port & 0xFF);
        eeprom_data[5] = (u8)((port >> 8) & 0xFF);      // stm32 little endding

        ee_write_bytes_os(eeprom_data,MEM_SERVER_IP_ADDR,0x06);
}

void read_server_ip(u8 *ip,u16 *port)
{
        u8 eeprom_data[6];
        ee_read_bytes_os(eeprom_data,MEM_SERVER_IP_ADDR,0x06);

        ip[0] = eeprom_data[0];
        ip[1] = eeprom_data[1];
        ip[2] = eeprom_data[2];
        ip[3] = eeprom_data[3];
        *port = *(u16 *)(eeprom_data + 4);              // stm32 little endding
}

void eeprom_write_update_done(void)
{
        u8 eeprom_data[2];
        eeprom_data[0] = (u8)(DONE_UPDATE_FLAG & 0xFF);
        eeprom_data[1] = (u8)((DONE_UPDATE_FLAG >> 8 )& 0xFF);
        ee_write_bytes_os(eeprom_data,MEM_UPDATE_FLAG_ADDR,0x02);
}

void eeprom_write_update_start(void)
{
        u8 eeprom_data[2];
        eeprom_data[0] = (u8)(WAIT_UPDATE_FLAG & 0xFF);
        eeprom_data[1] = (u8)((WAIT_UPDATE_FLAG >> 8 )& 0xFF);
        ee_write_bytes_os(eeprom_data,MEM_UPDATE_FLAG_ADDR,0x02);
}

void eeprom_first_burn(void)
{
        u8 eeprom_data[10];
        if (ee_check_connect()) {
                ee_read_bytes_os(eeprom_data,MEM_INITFL_ADDR,0x02);

                if ( (eeprom_data[0] != MEM_INITFL_DATA1) || (eeprom_data[1] != MEM_INITFL_DATA2) ) {
                        eeprom_data[0] = MEM_INITFL_DATA1;
                        eeprom_data[1] = MEM_INITFL_DATA2;
                        ee_write_bytes_os(eeprom_data,MEM_INITFL_ADDR,0x02);

                        eeprom_data[0] = MEM_FIRMWARE_VER_DATA1;
                        eeprom_data[1] = MEM_FIRMWARE_VER_DATA2;
                        ee_write_bytes_os(eeprom_data,MEM_INITFL_ADDR,0x02);

                        u8 ip[4] = SERVER_IP;
                        u16 server_port = SERVER_PORT;
                        update_server_ip(ip, server_port);
                } else {
                
                }
        } else {

        }
}

u8 eeprom_init(void)
{
        return (ee_check_connect());
}


