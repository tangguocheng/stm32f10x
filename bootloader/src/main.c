#include "stm32f10x.h"
#include "proj_conf.h"
#include <stdarg.h>
#include "bsp_led.h"
#include "w5500_port.h"

void assert_failed(unsigned char *file, unsigned int line)
{
        LOG_OUT(LOG_ERR "ASSERT Failed -- [%s]:[%d]\r\n",file,line);
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

        RCC_ClocksTypeDef rcc_clock_freq;

        RCC_GetClocksFreq(&rcc_clock_freq);

        LOG_OUT("SYSCLK\t%d\r\n", rcc_clock_freq.SYSCLK_Frequency);
        LOG_OUT("HCLK\t%d\r\n", rcc_clock_freq.HCLK_Frequency);
        LOG_OUT("PCLK1\t%d\r\n", rcc_clock_freq.PCLK1_Frequency);
        LOG_OUT("PCLK2\t%d\r\n", rcc_clock_freq.PCLK2_Frequency);
        
        LOG_OUT(LOG_INFO "System Start\r\n");
        LOG_OUT(LOG_INFO "FreeRTOS Kernel V10.0.0\r\n");
        LOG_OUT(LOG_INFO "STM32LIB V3.5.0\r\n");

        led_init();
        
        while (1) {
                w5500_socket_process(); 
        }

}


