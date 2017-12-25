/*
*********************************************************************************************************
*
*	模块名称 : 串行EEPROM 24xx02驱动模块
*	文件名称 : bsp_eeprom_24xx.h
*	版    本 : V1.0
*	说    明 : 头文件
*
*	修改记录 :
*		版本号  日期       作者    说明
*		v1.0    2012-10-12 armfly  ST固件库版本 V2.1.0
*
*	Copyright (C), 2012-2013, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#ifndef _BSP_EEPROM_24XX_H
#define _BSP_EEPROM_24XX_H

#define AT24C08

#ifdef AT24C02
	#define EE_MODEL_NAME		"AT24C02"
	#define EE_DEV_ADDR             0xA0		/* 设备地址 */
	#define EE_PAGE_SIZE		8               /* 页面大小(字节) */
	#define EE_SIZE                 256             /* 总容量(字节) */
	#define EE_ADDR_BYTES		1               /* 地址字节个数 */
#endif

#ifdef AT24C128
	#define EE_MODEL_NAME		"AT24C128"
	#define EE_DEV_ADDR             0xA0		/* 设备地址 */
	#define EE_PAGE_SIZE		64              /* 页面大小(字节) */
	#define EE_SIZE                 (16*1024)	/* 总容量(字节) */
	#define EE_ADDR_BYTES		2               /* 地址字节个数 */
#endif

#ifdef AT24C08
	#define EE_MODEL_NAME		"AT24C08"
	#define EE_DEV_ADDR             0xA0		/* 设备地址 */
	#define EE_PAGE_SIZE		16              /* 页面大小(字节) */
	#define EE_SIZE                 (1 * 1024)	/* 总容量(字节) */
	#define EE_ADDR_BYTES		2               /* 地址字节个数 */
#endif

u8 ee_check_connect(void);
u8 ee_read_byts(u8 *_pReadBuf, u16 _usAddress, u16 _usSize);
u8 ee_write_byts(u8 *_pWriteBuf, u16 _usAddress, u16 _usSize);

#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
