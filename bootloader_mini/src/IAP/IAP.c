#include "stm32f10x.h"
#include "stm_flash.h"
#include "eeprom_mem.h"
#include "IAP.h"

static u16 iapbuf[1024];

void iap_soft_reset(void)
{
        __set_FAULTMASK(1);      
        NVIC_SystemReset();
}

void start_user_app(void)
{
        if ( ((*(u32 *)USER_APP_ADDR) & 0x2FFE0000) == 0x20000000 ) {
                u32 usr_reset_isr = *(__IO u32*)(USER_APP_ADDR + 4);
                usr_app_fun user_application = (usr_app_fun)usr_reset_isr;
                __set_MSP(*(__IO uint32_t*)USER_APP_ADDR);
                user_application();
        }
}

u8 iap_software_check(u8* soft_ver)
{
        return (1);
}

void iap_done(void)
{
        eeprom_write_update_done();
}


void iap_write_app_data(u16 offset, u8 *appbuf, u16 appsize)
{
        u16 app_word_size = 0;
        u8 *dfu = appbuf;
        for(u16 i = 0; i < appsize; i += 2) {   
                iapbuf[app_word_size] = (u16)( ((dfu[1] << 8) & 0xFF00) | dfu[0] );
                app_word_size++;
                dfu += 2;
        }                
                
        STMFLASH_Write(UPDATE_CACHE_ADDR + offset, iapbuf, app_word_size);
}

void iap_copy_app_cache(void)
{
        u16 totoal_secotr = (UPDATE_CACHE_LEN / 2) / 1024;
        u16 remain_size   = (UPDATE_CACHE_LEN / 2) % 1024;
        u32 flash_addr = UPDATE_CACHE_ADDR;
        u32 usr_app_loff = 0;
        for (u32 i = 0; i < totoal_secotr; i++) {
                STMFLASH_Read(flash_addr, iapbuf, 1024);
                STMFLASH_Write(USER_APP_ADDR + usr_app_loff, iapbuf, 1024);
                flash_addr += 1024 * 2;
                usr_app_loff += 1024 * 2;
        }
        
        if (remain_size > 0) {
                STMFLASH_Read(flash_addr, iapbuf, remain_size);
                STMFLASH_Write(USER_APP_ADDR + usr_app_loff, iapbuf, remain_size);
        }
        
        iap_done();
}
