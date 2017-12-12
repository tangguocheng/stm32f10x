#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stm32f10x.h"
#include "proj_conf.h"

#include <stdarg.h>
#include "semphr.h"

static SemaphoreHandle_t  xMutex = NULL;

// thread safe printf
void  App_Printf(char *format, ...)
{
        char  buf_str[200 + 1];
        va_list   v_args;

        va_start(v_args, format);
        (void)vsnprintf((char       *)&buf_str[0],
                        (size_t      ) sizeof(buf_str),
                        (char const *) format,
                        v_args);
        va_end(v_args);

        if (xMutex == NULL) {
                printf("%s", buf_str);
        } else {
                if (xSemaphoreTake(xMutex, 0)) {
                        printf("%s", buf_str);
                        xSemaphoreGive(xMutex);
                }
        }
}


void assert_failed(unsigned char *file, unsigned int line)
{
        WSY_LOG_OUT(LOG_ERR "ASSERT Failed -- [%s]:[%d]\r\n",file,line);
}

void bsp_init(void)
{

}

void init_task(void * param)
{
        WSY_LOG_OUT(LOG_INFO "Init Task Start...\r\n");

        bsp_init();

        xMutex = xSemaphoreCreateMutex();

        vTaskDelete( NULL );
}

void create_init_task( void )
{
        TaskHandle_t xHandle = NULL;

        xTaskCreate( init_task, "init_task", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES, &xHandle );
        configASSERT( xHandle );

        if( xHandle == NULL ) {
                WSY_LOG_OUT(LOG_ERR "Cannot Create Init Task...\r\n");
                vTaskDelete( xHandle );
        }
}



void vApplicationTickHook (void)
{
	
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

        WSY_LOG_OUT("SYSCLK\t%d\r\n", rcc_clock_freq.SYSCLK_Frequency);
        WSY_LOG_OUT("HCLK\t%d\r\n", rcc_clock_freq.HCLK_Frequency);
        WSY_LOG_OUT("PCLK1\t%d\r\n", rcc_clock_freq.PCLK1_Frequency);
        WSY_LOG_OUT("PCLK2\t%d\r\n", rcc_clock_freq.PCLK2_Frequency);
        
        WSY_LOG_OUT(LOG_INFO "System Start\r\n");
        WSY_LOG_OUT(LOG_INFO "FreeRTOS Kernel V10.0.0\r\n");
        WSY_LOG_OUT(LOG_INFO "STM32LIB V3.5.0\r\n");
        WSY_LOG_OUT(LOG_INFO "Xunjian Robot Initialization...\r\n");

        create_init_task();

        vTaskStartScheduler();

        while (1) {
                WSY_LOG_OUT(LOG_ERR "FreeRTOS Start Failed, Maybe Stack Memeory Too Small\r\n");
        }

}


