#ifndef __STMFLASH_H__
#define __STMFLASH_H__

#define STM32_FLASH_SIZE        256 	 		// 所选STM32的FLASH容量大小(单位为K)

#if STM32_FLASH_SIZE < 256
#define STM_SECTOR_SIZE         1024
#else
#define STM_SECTOR_SIZE	        2048
#endif

void STMFLASH_Write(u32 WriteAddr, u16 *pBuffer, u16 NumToWrite);
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead);

#endif

















