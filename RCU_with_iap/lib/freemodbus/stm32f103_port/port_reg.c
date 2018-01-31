#include "mb.h"
#include "eeprom_mem.h"
#include "mbframe.h"
#include "IAP.h"
#include "device_manager.h"

u8 device_info[24] = {0,};

static u8 write_unlock = 0;
static u8 update_enable = 0;
// ‰»Îºƒ¥Ê∆˜£¨÷ª∂¡
eMBErrorCode eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress,
                            USHORT usNRegs )
{
        return ((eMBErrorCode)device_data_process(MODBUS_REG, MODBUS_MODE_R, usAddress, pucRegBuffer, usNRegs));
}

// ±£≥÷ºƒ¥Ê∆˜£¨∂¡–¥
eMBErrorCode eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress,
                              USHORT usNRegs, eMBRegisterMode eMode )
{
        u8 mode = ((eMode == MB_REG_READ) ? MODBUS_MODE_R : MODBUS_MODE_W);
        return ((eMBErrorCode)device_data_process(MODBUS_REG, mode, usAddress, pucRegBuffer, usNRegs));
}

// œﬂ»¶£¨∂¡–¥
eMBErrorCode eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress,
                            USHORT usNCoils, eMBRegisterMode eMode )
{
        u8 mode = ((eMode == MB_REG_READ) ? MODBUS_MODE_R : MODBUS_MODE_W);
        return ((eMBErrorCode)device_data_process(MODBUS_COLI, mode, usAddress, pucRegBuffer, usNCoils));
}

// ¿Î…¢¡ø ‰»Î,÷ª∂¡
eMBErrorCode eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress,
                               USHORT usNDiscrete )
{        
        return ((eMBErrorCode)device_data_process(MODBUS_COLI, MODBUS_MODE_R, usAddress, pucRegBuffer, usNDiscrete));
}

#define MB_PASSWD_CODE          0x65
#define MB_DOWNLOAD_CODE        0x66
#define MB_UPDATE_OP_CODE       0x67
        #define MB_UPDATE_START_SUB_CODE        0x01
        #define MB_UPDATE_DONE_SUB_CODE         0x02
#define MB_REBOOT_CODE          0x68
#define MB_PASSWD_ADDR          0xFFF0
#define MB_PASSWD_DATA          {0x00,0x00,0x00,0x00}

eMBException eMBFuncUserDefine( UCHAR *pucFrame, USHORT *usLen )
{
        switch (pucFrame[MB_PDU_FUNC_OFF]) {
        case MB_PASSWD_CODE: 
                if (*usLen == (1 + 2 + 4)) {
                        u16 addr = ((pucFrame[MB_PDU_DATA_OFF + 1] << 8u) & 0xFF00) | pucFrame[MB_PDU_DATA_OFF];
                        if (addr == MB_PASSWD_ADDR) {                   // todo: check password data
                                pucFrame[MB_PDU_DATA_OFF] = 0x01;
                                write_unlock = 1;                       // write enable
                        } else {
                                pucFrame[MB_PDU_DATA_OFF] = 0x00;
                        }
                        *usLen = 2;
                }
        break;
                
        case MB_DOWNLOAD_CODE: {
                        if (write_unlock == 0)
                                break;
                        
                        u16 addr = ((pucFrame[MB_PDU_DATA_OFF + 1] << 8u) & 0xFF00) | pucFrame[MB_PDU_DATA_OFF];
                        u16 update_len = ((pucFrame[MB_PDU_DATA_OFF + 3] << 8u) & 0xFF00) | pucFrame[MB_PDU_DATA_OFF + 2];
                        if ((*usLen == (update_len + 1 + 2 + 2)) && (update_enable == 1)) {
                                taskENTER_CRITICAL();
                                iap_write_app_data(addr, &pucFrame[MB_PDU_DATA_OFF + 4], update_len);
                                taskEXIT_CRITICAL();
                        } else {
                                pucFrame[MB_PDU_DATA_OFF + 2] = 0;
                                pucFrame[MB_PDU_DATA_OFF + 3] = 0;
                        } 
                        
                        *usLen = 1 + 2 + 2;
                }
                break;

        case MB_UPDATE_OP_CODE:
                if ((*usLen == 2) || (*usLen == 5)) {
                        if (pucFrame[MB_PDU_DATA_OFF] == 0x01) {
                                if (iap_software_check(&pucFrame[MB_PDU_DATA_OFF + 1])) {
                                        *usLen = 6;
                                        update_enable = 1;
                                }
                        } else if (pucFrame[MB_PDU_DATA_OFF] == 0x02) {
                                iap_done();
                                write_unlock = 0;
                                update_enable = 0;
                                pucFrame[MB_PDU_DATA_OFF + 1] = 0x01;
                                eeprom_write_update_done();
                                *usLen = 3;
                        }
                }
                break;

        case MB_REBOOT_CODE:
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                iap_soft_reset();
                break;
        }

        return MB_EX_NONE;
}
