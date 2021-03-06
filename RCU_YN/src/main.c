#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stm32f10x.h"
#include "proj_conf.h"
#include <stdarg.h>
#include "semphr.h"
#include "bsp_led.h"
#include "bsp_uart1.h"
#include "bsp_led_display.h"
#include "bsp_board_gpio.h"
#include "eeprom_mem.h"
#include "task_485.h"
#include "task_w5500.h"
#include "task_modbus.h"
#include "bsp_uart3.h"

void assert_failed(unsigned char *file, unsigned int line)
{
        LOG_OUT(LOG_ERR "ASSERT Failed -- [%s]:[%d]\r\n",file,line);
}



void bsp_init(void)
{
        led_init();
        Uart1_init(9600,1,0);
        board_gpio_init();
//        led_display_init();
//        eeprom_first_burn();
}

void init_task(void * param)
{
        TaskHandle_t xHandle = NULL;
        
        LOG_OUT(LOG_INFO "Init Task Start...\r\n");
        
        bsp_init();
        
        xTaskCreate( task_485_send, "task_485_send", configMINIMAL_STACK_SIZE, NULL, configMODBUS_PRIORITIES, &xHandle );
        configASSERT( xHandle );
        
        xTaskCreate( task_485_receive, "task_485_receive", configMINIMAL_STACK_SIZE, NULL, configMODBUS_PRIORITIES, &xHandle );
        configASSERT( xHandle );
        
        xTaskCreate( task_485_poll, "task_485_poll", configMINIMAL_STACK_SIZE, NULL, configMODBUS_PRIORITIES - 1, &xHandle );
        configASSERT( xHandle );
        
        vTaskDelete( NULL );
}

void create_init_task( void )
{
        TaskHandle_t xHandle = NULL;

        xTaskCreate( init_task, "init_task", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES, &xHandle );
        configASSERT( xHandle );
        
        if( xHandle == NULL ) {
                LOG_OUT(LOG_ERR "Cannot Create Init Task...\r\n");
                vTaskDelete( xHandle );
        }
}

#include "dhcp.h"
static u32 ms_cnt = 0;
extern volatile u8 rs485_data_in;
void vApplicationTickHook (void)
{        
        ms_cnt++;
        gpio_input_process();
        time_scheduler();
//        led_display();
        if ((ms_cnt % 1000) == 0) {
                STATE_LED_TOGGLE();
                if (rs485_data_in){
                        rs485_data_in = 0;
                        rs485_LED_TOGGLE();
                }
//                DHCP_time_handler();
        }
}

u32 get_current_time_ms(void)
{
        return (ms_cnt);
}

#define APP1_VETOR_TABLE_ADDR   0x3000
#define APP2_VETOR_TABLE_ADDR   0x1C000

int main(void)
{  
//        NVIC_SetVectorTable(NVIC_VectTab_FLASH, APP1_VETOR_TABLE_ADDR);
        
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
        LOG_OUT(LOG_INFO "Xunjian Robot Initialization...\r\n");

        create_init_task();

        vTaskStartScheduler();

        while (1) {
                LOG_OUT(LOG_ERR "FreeRTOS Start Failed, Maybe Stack Memeory Too Small\r\n");
        }

}


