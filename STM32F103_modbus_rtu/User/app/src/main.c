/*
*********************************************************************************************************
*
*	模块名称 : 主程序入口
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : 串口通信例子，和PC机超级终端软件进行交互
*	修改记录 :
*		版本号  日期       作者    说明
*		V1.1    2016.5.27 armfly  	首发,GXP,2016年5月27日14:58:17，ADD,FreeRTOS
*
*	Copyright (C), 2015-2016, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "list.h"
#include "portable.h"

#include "bsp.h"				/* 底层硬件驱动 */

#include "kfifo.h" //add，2016年8月17日10:06:35


/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"


/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"STM32F103ZET6+FreeRTOS V8.2.3+kfifo(巧夺天工)+FreeModbus-V1.5.0"
#define EXAMPLE_DATE	"2016年11月10日13:28:26"
#define DEMO_VER			"1.3"

static void vModbusTask( void *pvParameters );


#define MAX_TASK_NUM        5
TaskStatus_t pxTaskStatusArray[MAX_TASK_NUM];

/* 保存任务句柄 */
TaskHandle_t LCDTask = NULL, LCD_Send_CMDTask = NULL;


//创建一个 FIFO 的结构体

struct KFIFO *test_kifo_buffer=NULL;

uint8_t test_fifo_write_buff[10]= {0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39};
uint8_t test_fifo_read_buff[10]= {0};
/*---------------------------------
   函数名：  void delay_ms(unsigned int delay_nms)
   作者：    GXP(没有经过测试 不知道准不准)
   版本：    V1.0
   日期：    2016年6月13日08:55:01
   说明：    非精确延时 ms
   更改记录：无
-----------------------------------*/

//毫秒级的延时
void delay_ms(unsigned int delay_ms)
{
        unsigned int i=0;
        while(delay_ms--) {
                i=12000;  //自己定义
                while(i--) ;
        }
}


/*
*********************************************************************************************************
*	函 数 名: main
*	功能说明: c程序入口
*	形    参：无
*	返 回 值: 错误代码(无需处理)
*********************************************************************************************************
*/
int main(void)
{

        /*
        	ST固件库中的启动文件已经执行了 SystemInit() 函数，该函数在 system_stm32f4xx.c 文件，主要功能是
        配置CPU系统的时钟，内部Flash访问时序，配置FSMC用于外部SRAM
        */

        bsp_Init();		/* 硬件初始化 */

        xTaskCreate( vModbusTask, "ModbusTask", configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY+1,NULL);

        /* Start the scheduler. */
        vTaskStartScheduler();

        while(1);

}

/*-----------------------------------------------------------*/

static void vModbusTask( void *pvParameters )
{
        /* Select either ASCII or RTU Mode. */
        ( void )eMBInit( MB_RTU, 0x0A, 0, 9600, MB_PAR_EVEN );

        /* Enable the Modbus Protocol Stack. */
        ( void )eMBEnable(  );

        printf( "vModbusTask正在运行中！\r\n");

        for( ;; ) {
                /* Call the main polling loop of the Modbus protocol stack. */
                ( void )eMBPoll(  );

                //vTaskDelay( 10 / portTICK_PERIOD_MS );
        }
}

