#include "mb.h"

static volatile u8 write_unlock = 0;

eMBErrorCode eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress,
                               USHORT usNRegs )
{
        return (MB_ENOERR);
}


eMBErrorCode eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress,
                              USHORT usNRegs, eMBRegisterMode eMode )
{
        return (MB_ENOERR);
}

eMBErrorCode eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress,
                            USHORT usNCoils, eMBRegisterMode eMode )
{
        return (MB_ENOERR);
}

eMBErrorCode eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress,
                               USHORT usNDiscrete )
{
        return (MB_ENOERR);
}

#include "mbframe.h"
#include "IAP.h"

#define MB_PASSWD_CODE          0x65
#define MB_DOWNLOAD_CODE        0x66
#define MB_UPDATE_OP_CODE       0x67
        #define MB_UPDATE_START_SUB_CODE        0x01
        #define MB_UPDATE_DONE_SUB_CODE         0x02
#define MB_REBOOT_CODE          0x68
#define MB_PASSWD_ADDR          0xFFF0
#define MB_PASSWD_DATA          {0x00,0x00,0x00,0x00}
// user define
eMBException eMBFuncUserDefine( UCHAR * pucFrame, USHORT * usLen )
{
        switch (pucFrame[MB_PDU_FUNC_OFF]) {
        case MB_PASSWD_CODE: 
                if (*usLen == (1 + 2 + 4)) {
                        u16 addr = pucFrame[MB_PDU_DATA_OFF] << 8u;
                        addr |= pucFrame[MB_PDU_DATA_OFF + 1];
                        if (addr == MB_PASSWD_ADDR) {                   // todo: check password data
                                pucFrame[MB_PDU_DATA_OFF] = 0x01;
                                write_unlock = 1;                       // write enable
                        } else {
                                pucFrame[MB_PDU_DATA_OFF] = 0x00;
                        }
                        *usLen = 2;
                }
        break;
        case MB_DOWNLOAD_CODE:
                break;

        case MB_UPDATE_OP_CODE:
                if (*usLen == 2) {
                        if (pucFrame[MB_PDU_DATA_OFF] == 0x01)
                                iap_start();
                        else if (pucFrame[MB_PDU_DATA_OFF] == 0x02) {
                                iap_done();
                                pucFrame[MB_PDU_DATA_OFF + 1] = 0x01;
                                *usLen = 3;
                        }
                }
                break;

        case MB_REBOOT_CODE:
                iap_soft_reset();
                break;
        }

        return MB_EX_NONE;
}
