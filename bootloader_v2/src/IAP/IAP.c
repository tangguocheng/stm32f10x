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
                
        STMFLASH_Write(USER_APP_ADDR + offset, iapbuf, app_word_size);
        
}
