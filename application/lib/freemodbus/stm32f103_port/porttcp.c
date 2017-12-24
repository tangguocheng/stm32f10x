#include <stdio.h>
#include "socket.h"
#include "port.h"
#include "mb.h"
#include "mbport.h"
#include "w5500_port.h"
#include "task_w5500.h"

extern void vMBPortEventClose(void);

BOOL xMBTCPPortInit(USHORT usTCPPort )
{
        return TRUE;
}

void vMBTCPPortClose(void)
{
        close(SOCK_TCP);
        vMBPortEventClose();
}

void vMBTCPPortDisable(void)
{
        return;
}

BOOL xMBTCPPortGetRequest(UCHAR **ppucMBTCPFrame, USHORT *usTCPLength)
{
        get_tcp_rev_data(ppucMBTCPFrame,usTCPLength);
        return TRUE;
}

BOOL xMBTCPPortSendResponse( const UCHAR * pucMBTCPFrame, USHORT usTCPLength )
{
        send(SOCK_TCP, (UCHAR * )pucMBTCPFrame, usTCPLength);
        return TRUE;
}
