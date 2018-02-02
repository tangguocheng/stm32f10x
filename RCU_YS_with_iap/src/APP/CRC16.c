#include "stm32f10x.h"

u16 crc16_cal(u8* high_byte, u8* low_byte, u8 *DAT, u8 len)
{
        u16 CRC1 = 0xFFFF;
        u8 i,j;
        for(i=0;i<len;i++)
        {
                CRC1 = CRC1 ^ DAT[i];
                for(j=0;j<8;j++)
                {
                        if(CRC1&0x01)
                        {
                                CRC1 = CRC1>>1;
                                CRC1 = CRC1^0xA001;
                        }
                        else
                        {
                                CRC1 = CRC1>>1;
                        }
                }
        }
        *high_byte = ((CRC1 >> 8) & 0xFF);
        *low_byte = (CRC1 & 0xFF);
        return CRC1;
}
