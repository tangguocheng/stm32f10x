#include "stm32f10x.h"
#include "stm_flash.h"
#include "eeprom_mem.h"
#include "IAP.h"

void iap_soft_reset(void)
{
        __set_FAULTMASK(1);      
        NVIC_SystemReset();
}

void iap_init(void)
{
        NVIC_SetVectorTable(NVIC_VectTab_FLASH, APP_VETOR_TABLE_ADDR);
}

u8 iap_software_check(u8* soft_ver)
{
        return (1);
}

void iap_done(void)
{
        eeprom_write_update_done();
}

void iap_start(void)
{
        eeprom_write_update_start();
}



