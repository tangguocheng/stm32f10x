#include "stm32f10x.h"
#include "proj_conf.h"
#include <stdarg.h>
#include "bsp_led.h"
#include "eeprom_mem.h"
#include "bsp_led_display.h"
#include "bsp_timer.h"

typedef void (*usr_app_fun)(void);

void assert_failed(unsigned char *file, unsigned int line)
{
        LOG_OUT(LOG_ERR "ASSERT Failed -- [%s]:[%d]\r\n",file,line);
}

void start_user_app(u32 app_addr)
{
        if ( ((*(u32 *)app_addr) & 0x2FFE0000) == 0x20000000 ) {
                u32 usr_app_addr = *(__IO u32*)(app_addr + 4);
                __disable_irq(); 
                sys_timer_init(1,0,DISABLE); 
                NVIC_SetVectorTable(NVIC_VectTab_FLASH, app_addr - FLASH_BASE);
                usr_app_fun user_application = (usr_app_fun)usr_app_addr;
                __set_MSP(*(__IO uint32_t*)app_addr);
                user_application();
        }
}

int main(void)
{       
        RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA |
                                RCC_APB2Periph_GPIOB |
                                RCC_APB2Periph_GPIOC |
                                RCC_APB2Periph_GPIOD |
                                RCC_APB2Periph_GPIOE |
                                RCC_APB2Periph_GPIOF |
                                RCC_APB2Periph_GPIOG |
                                RCC_APB2Periph_AFIO,
                                ENABLE );

        // NVIC Priority Group In freeRTOS Must Set To NVIC_PriorityGroup_4
        NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
        
        u32 usr_app_addr = 0;   
        led_display_init();           
        sys_timer_init(1,0,ENABLE);        
        ee_recevor_usr_app(&usr_app_addr);
        set_led_content(LED_TYPE_INFO,APP_VALUE);
        time_cnt_down();
        usr_app_addr = (u32)(FLASH_BASE + 0x3000);
        start_user_app(usr_app_addr);
        set_led_content(LED_TYPE_INFO,ERR_VALUE_0);
        while (1) {
               
        }

}


