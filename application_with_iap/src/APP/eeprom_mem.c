#include "stm32f10x.h"
#include "freeRTOS.h"
#include "task.h"
#include "IAP.h"
#include "bsp_delay.h"
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

void eeprom_write_update_done(void)
{
        u8 eeprom_data[2];
        eeprom_data[0] = (u8)(WAIT_UPDATE_FLAG & 0xFF);
        eeprom_data[1] = (u8)((WAIT_UPDATE_FLAG >> 8 )& 0xFF);
        ee_write_bytes_os(eeprom_data,MEM_UPDATE_FLAG_ADDR,0x02);
}

u8 eeprom_check_update(void)
{
        u8 eeprom_data[2] = {0,0};
        u8 need_update = 0;
        if (ee_check_connect()) 
                ee_read_bytes_os(eeprom_data,MEM_UPDATE_FLAG_ADDR,0x02);
        
        if (*(u16*)eeprom_data == WAIT_UPDATE_FLAG) 
                need_update = 1;
        
        if (need_update == 0) {
                need_update = 1;
                start_user_app();
        }
        
        return (need_update);
}

u8 eeprom_init(void)
{
        u8 rtl = 0;
        rtl = ee_check_connect();
        delay_nms(100);
        return (rtl);
}

