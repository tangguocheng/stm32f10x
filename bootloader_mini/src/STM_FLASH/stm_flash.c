#include "stm32f10x.h"
#include "stm_flash.h"
#include "bsp_delay.h"

static u16 STMFLASH_BUF[STM_SECTOR_SIZE / 2];

void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead)
{
        for(u16 i = 0; i < NumToRead; i++) {
                pBuffer[i] =* (u16*)ReadAddr;//读取2个字节.
                ReadAddr += 2;
        }
}

void STMFLASH_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)
{
        for(u16 i = 0; i < NumToWrite; i++) {
                FLASH_ProgramHalfWord(WriteAddr,pBuffer[i]);
                WriteAddr += 2;
        }
}

void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)
{
        u32 secpos,offaddr;
        u16 secoff,secremain,i;

        if (WriteAddr < FLASH_BASE || (WriteAddr >= (FLASH_BASE + 1024 * STM32_FLASH_SIZE)))
                return;

        FLASH_Unlock();
        offaddr = WriteAddr - FLASH_BASE;		                // 实际偏移地址.
        secpos = (u16)(offaddr / STM_SECTOR_SIZE);			// 扇区地址  0~127 for STM32F103RBT6
        secoff = (u16)((offaddr % STM_SECTOR_SIZE) / 2);		// 在扇区内的偏移(2个字节为基本单位.)
        secremain = (u16)(STM_SECTOR_SIZE / 2 - secoff);		// 扇区剩余空间大小
        if (NumToWrite <= secremain)
                secremain = NumToWrite;

        while(1) {
                STMFLASH_Read(secpos * STM_SECTOR_SIZE + FLASH_BASE, STMFLASH_BUF, STM_SECTOR_SIZE / 2);
                for (i=0; i < secremain; i++) {
                        if(STMFLASH_BUF[secoff + i] != 0XFFFF)
                                break;
                }
                
                if (i<secremain) {
                        FLASH_ErasePage(secpos * STM_SECTOR_SIZE + FLASH_BASE);
                        for(i=0; i<secremain; i++) {
                                STMFLASH_BUF[i + secoff] = pBuffer[i];
                        }

                        STMFLASH_Write_NoCheck(secpos * STM_SECTOR_SIZE + FLASH_BASE, STMFLASH_BUF, STM_SECTOR_SIZE / 2);
                } else {
                        STMFLASH_Write_NoCheck(WriteAddr,pBuffer,secremain);
                }

                if (NumToWrite == secremain) {
                        break;
                } else {
                        secpos++;
                        secoff = 0;
                        pBuffer += secremain;
                        WriteAddr += secremain;
                        NumToWrite -= secremain;
                        if ( NumToWrite > (STM_SECTOR_SIZE / 2))
                                secremain = STM_SECTOR_SIZE / 2;
                        else
                                secremain = NumToWrite;
                }
        };
        FLASH_Lock();
}














