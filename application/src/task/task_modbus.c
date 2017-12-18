#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "mb.h"
#include "w5500_port.h"

void task_modbus(void *param)
{
        (void)param;
        
        eMBTCPInit(SOCK_TCP);   
        eMBEnable();	
        
        while (1) {
                eMBPoll();
        }        
}
