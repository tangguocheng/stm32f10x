#include "stm32f10x.h"
#include "stm_flash.h"
#include "eeprom_mem.h"
#include "IAP.h"

void iap_soft_reset(void)
{
        __set_FAULTMASK(1);      
        NVIC_SystemReset();
}

static u16 iapbuf[1024];
static u32 current_app_addr = 0;

extern int main(void);
static u32 update_app_addr = 0;

void iap_init(void)
{
        u32 main_addr = (u32)((u32 *)main);
        if (main_addr >= FLASH_BASE + APP2_VETOR_TABLE_ADDR) {   // app2
                update_app_addr = FLASH_BASE + APP1_VETOR_TABLE_ADDR;
                NVIC_SetVectorTable(NVIC_VectTab_FLASH, APP2_VETOR_TABLE_ADDR);
        } else {
                update_app_addr = FLASH_BASE + APP2_VETOR_TABLE_ADDR;  
                NVIC_SetVectorTable(NVIC_VectTab_FLASH, APP1_VETOR_TABLE_ADDR);
        }
}

void iap_start(void)
{
        current_app_addr = update_app_addr;
}

void iap_done(void)
{
        ee_store_app_addr(&update_app_addr);
}

void iap_update_app(u8 *appbuf, u32 appsize)
{
        u16 t;
        u16 i = 0;
        u16 temp;
        u32 fwaddr = current_app_addr;                       // 当前写入的地址
        u8 *dfu = appbuf;
        for(t = 0; t < appsize; t += 2) {
                temp = (u16)dfu[1]<<8;
                temp += (u16)dfu[0];
                dfu += 2;                                       // 偏移2个字节
                iapbuf[i++] = temp;
                if (i == 1024) {
                        i = 0;
                        STMFLASH_Write(fwaddr,iapbuf,1024);
                        fwaddr += 2048;                         // 偏移2048  16=2*8.所以要乘以2.
                }
        }
        
        if(i)  STMFLASH_Write(fwaddr, iapbuf, i);               // 将最后的一些内容字节写进去.
        
        current_app_addr += (u32)(appsize / 2);
}

