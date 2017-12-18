#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "mb.h"

/* ----------------------- Defines ------------------------------------------*/
#define MB_POLL_CYCLETIME       100     /* Poll cycle time is 100ms */
#define SYS_MBOX_SIZE           (16)
#define MS_TO_TICKS( ms )       (portTickType)((portTickType)(ms) / portTICK_RATE_MS )
#define TICKS_TO_MS( ticks )    (unsigned portLONG)((portTickType)(ticks) * portTICK_RATE_MS )
#define SYS_ARCH_TIMEOUT        0xFFFFFFFF

/* ----------------------- Static variables ---------------------------------*/
static QueueHandle_t xMailBox = NULL;
static eMBEventType eMailBoxEvent;

/* ----------------------- Start implementation -----------------------------*/
BOOL xMBPortEventInit( void )
{
        eMailBoxEvent = EV_READY;
        xMailBox = xQueueCreate( SYS_MBOX_SIZE, sizeof( void * ) );
        return xMailBox != NULL ? TRUE : FALSE;
}

void vMBPortEventClose( void )
{
        void *ret_msg;
        if( xMailBox != NULL ) {
                while( uxQueueMessagesWaiting( xMailBox ) != 0 )
                {
                        xQueueReceive( xMailBox, &ret_msg, MS_TO_TICKS(1) );
                }
                vQueueDelete( xMailBox );
        }
}

BOOL xMBPortEventPost( eMBEventType eEvent )
{
        eMailBoxEvent = eEvent;
        void *data = &eMailBoxEvent;
        xQueueSend( xMailBox, &data, 0 );
        return TRUE;
}

u32 sys_arch_mbox_fetch( QueueHandle_t mbox, void **msg, u32 timeout )
{
    void           *ret_msg;
    portBASE_TYPE   xStatus;
    portTickType    xTicksStart, xTicksEnd, xTicksElapsed;
    u32          timespent;

    xTicksStart = xTaskGetTickCount(  );
    if( timeout == 0 )
    {
        do
        {
            xStatus = xQueueReceive( mbox, &ret_msg, MS_TO_TICKS( 100 ) );
        }
        while( xStatus != pdTRUE );
    }
    else
    {
        xStatus = xQueueReceive( mbox, &ret_msg, MS_TO_TICKS( timeout ) );
    }

    if( xStatus == pdTRUE )
    {
        if( msg )
        {
            *msg = ret_msg;
        }
        xTicksEnd = xTaskGetTickCount(  );
        xTicksElapsed = xTicksEnd - xTicksStart;
        timespent = TICKS_TO_MS( xTicksElapsed );
    }
    else
    {
        if( msg )
        {
            *msg = NULL;
        }
        timespent = SYS_ARCH_TIMEOUT;
    }
    return timespent;
}

BOOL xMBPortEventGet( eMBEventType * eEvent )
{
        void *peMailBoxEvent;
        BOOL xEventHappend = FALSE;
        u32  uiTimeSpent;

        uiTimeSpent = sys_arch_mbox_fetch( xMailBox, &peMailBoxEvent, MB_POLL_CYCLETIME );
        if( uiTimeSpent != SYS_ARCH_TIMEOUT ) {
                *eEvent = *( eMBEventType * ) peMailBoxEvent;
                eMailBoxEvent = EV_READY;
                xEventHappend = TRUE;
        }
        return xEventHappend;
}
