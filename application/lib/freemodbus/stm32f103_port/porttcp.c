/*
 * FreeModbus Libary: lwIP Port
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: porttcp.c,v 1.2 2006/09/04 14:39:20 wolti Exp $
 */

/* ----------------------- System includes ----------------------------------*/
#include <stdio.h>
//#include "w5500_port.h"
#include "socket.h"
#include "port.h"
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- MBAP Header --------------------------------------*/
#define MB_TCP_UID          6
#define MB_TCP_LEN          4
#define MB_TCP_FUNC         7

/* ----------------------- Defines  -----------------------------------------*/
#define MB_TCP_DEFAULT_PORT 502 /* TCP listening port. */
#define MB_TCP_BUF_SIZE     ( 256 + 7 ) /* Must hold a complete Modbus TCP frame. */

/* ----------------------- Prototypes ---------------------------------------*/
void vMBPortEventClose(void);
/* ----------------------- Static variables ---------------------------------*/

static UCHAR    aucTCPBuf[MB_TCP_BUF_SIZE];
static USHORT   usTCPBufPos;

/* ----------------------- Static functions ---------------------------------*/

/* ----------------------- Begin implementation -----------------------------*/
BOOL xMBTCPPortInit(USHORT usTCPPort )
{
        return TRUE;
}


void vMBTCPPortClose(void)
{
        close(0);
        vMBPortEventClose();
}

void vMBTCPPortDisable( void )
{

}


BOOL xMBTCPPortGetRequest( UCHAR ** ppucMBTCPFrame, USHORT * usTCPLength )
{
        *ppucMBTCPFrame = &aucTCPBuf[0];
        *usTCPLength = usTCPBufPos;

        /* Reset the buffer. */
        usTCPBufPos = 0;
        return TRUE;
}

BOOL xMBTCPPortSendResponse( const UCHAR * pucMBTCPFrame, USHORT usTCPLength )
{
        send(0, (UCHAR * )pucMBTCPFrame, usTCPLength);
        return TRUE;
}
