#include "stm32f10x.h"
#include "bsp_i2c_gpio.h"
#include "bsp_eeprom_24xx.h"

/*
*********************************************************************************************************
*	函 数 名: ee_check_connect
*	功能说明: 判断串行EERPOM是否正常
*	形    参:  无
*	返 回 值: 1 表示正常， 0 表示不正常
*********************************************************************************************************
*/
u8 ee_check_connect(void)
{
	if (i2c_check_connect(EE_DEV_ADDR) == 0)
	{
		return 1;
	}
	else
	{
		/* 失败后，切记发送I2C总线停止信号 */
		i2c_stop();
		return 0;
	}
}


/*
*********************************************************************************************************
*	函 数 名: ee_read_byts
*	功能说明: 从串行EEPROM指定地址处开始读取若干数据
*	形    参:  _usAddress : 起始地址
*			 _usSize : 数据长度，单位为字节
*			 _pReadBuf : 存放读到的数据的缓冲区指针
*	返 回 值: 0 表示失败，1表示成功
*********************************************************************************************************
*/
u8 ee_read_byts(u8 *read_buf, u16 addr, u16 buf_len)
{
	u16 i;

	/* 采用串行EEPROM随即读取指令序列，连续读取若干字节 */

	/* 第1步：发起I2C总线启动信号 */
	i2c_start();

	/* 第2步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
	i2c_send_byte(EE_DEV_ADDR | I2C_WR);	/* 此处是写指令 */

	/* 第3步：发送ACK */
	if (i2c_wait_ack() != 0)
	{
		goto cmd_fail;	/* EEPROM器件无应答 */
	}

	/* 第4步：发送字节地址，24C02只有256字节，因此1个字节就够了，如果是24C04以上，那么此处需要连发多个地址 */
	if (EE_ADDR_BYTES == 1)
	{
		i2c_send_byte((u8)addr);
		if (i2c_wait_ack() != 0)
		{
			goto cmd_fail;	/* EEPROM器件无应答 */
		}
	}
	else
	{
		i2c_send_byte(addr >> 8);
		if (i2c_wait_ack() != 0)
		{
			goto cmd_fail;	/* EEPROM器件无应答 */
		}

		i2c_send_byte(addr);
		if (i2c_wait_ack() != 0)
		{
			goto cmd_fail;	/* EEPROM器件无应答 */
		}
	}

	/* 第6步：重新启动I2C总线。下面开始读取数据 */
	i2c_start();

	/* 第7步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
	i2c_send_byte(EE_DEV_ADDR | I2C_RD);	/* 此处是读指令 */

	/* 第8步：发送ACK */
	if (i2c_wait_ack() != 0)
	{
		goto cmd_fail;	/* EEPROM器件无应答 */
	}

	/* 第9步：循环读取数据 */
	for (i = 0; i < buf_len; i++)
	{
		read_buf[i] = i2c_read_byte();	/* 读1个字节 */

		/* 每读完1个字节后，需要发送Ack， 最后一个字节不需要Ack，发Nack */
		if (i != buf_len - 1)
		{
			i2c_ack();	/* 中间字节读完后，CPU产生ACK信号(驱动SDA = 0) */
		}
		else
		{
			i2c_nack();	/* 最后1个字节读完后，CPU产生NACK信号(驱动SDA = 1) */
		}
	}
	/* 发送I2C总线停止信号 */
	i2c_stop();
	return 1;	/* 执行成功 */

cmd_fail: /* 命令执行失败后，切记发送停止信号，避免影响I2C总线上其他设备 */
	/* 发送I2C总线停止信号 */
	i2c_stop();
	return 0;
}

/*
*********************************************************************************************************
*	函 数 名: ee_write_byts
*	功能说明: 向串行EEPROM指定地址写入若干数据，采用页写操作提高写入效率
*	形    参:  _usAddress : 起始地址
*			 _usSize : 数据长度，单位为字节
*			 _pWriteBuf : 存放读到的数据的缓冲区指针
*	返 回 值: 0 表示失败，1表示成功
*********************************************************************************************************
*/
u8 ee_write_byts(u8 *write_buf, u16 addr, u16 buf_len)
{
	u16 i,m;
	u16 usAddr;

	/*
		写串行EEPROM不像读操作可以连续读取很多字节，每次写操作只能在同一个page。
		对于24xx02，page size = 8
		简单的处理方法为：按字节写操作模式，每写1个字节，都发送地址
		为了提高连续写的效率: 本函数采用page wirte操作。
	*/

	usAddr = addr;
	for (i = 0; i < buf_len; i++)
	{
		/* 当发送第1个字节或是页面首地址时，需要重新发起启动信号和地址 */
		if ((i == 0) || (usAddr & (EE_PAGE_SIZE - 1)) == 0)
		{
			/*　第０步：发停止信号，启动内部写操作　*/
			i2c_stop();

			/* 通过检查器件应答的方式，判断内部写操作是否完成, 一般小于 10ms
				CLK频率为200KHz时，查询次数为30次左右
			*/
			for (m = 0; m < 1000; m++)
			{
				/* 第1步：发起I2C总线启动信号 */
				i2c_start();

				/* 第2步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
				i2c_send_byte(EE_DEV_ADDR | I2C_WR);	/* 此处是写指令 */

				/* 第3步：发送一个时钟，判断器件是否正确应答 */
				if (i2c_wait_ack() == 0)
				{
					break;
				}
			}
			if (m  == 1000)
			{
				goto cmd_fail;	/* EEPROM器件写超时 */
			}

			/* 第4步：发送字节地址，24C02只有256字节，因此1个字节就够了，如果是24C04以上，那么此处需要连发多个地址 */
			if (EE_ADDR_BYTES == 1)
			{
				i2c_send_byte((u8)usAddr);
				if (i2c_wait_ack() != 0)
				{
					goto cmd_fail;	/* EEPROM器件无应答 */
				}
			}
			else
			{
				i2c_send_byte(usAddr >> 8);
				if (i2c_wait_ack() != 0)
				{
					goto cmd_fail;	/* EEPROM器件无应答 */
				}

				i2c_send_byte(usAddr);
				if (i2c_wait_ack() != 0)
				{
					goto cmd_fail;	/* EEPROM器件无应答 */
				}
			}
		}

		/* 第6步：开始写入数据 */
		i2c_send_byte(write_buf[i]);

		/* 第7步：发送ACK */
		if (i2c_wait_ack() != 0)
		{
			goto cmd_fail;	/* EEPROM器件无应答 */
		}

		usAddr++;	/* 地址增1 */
	}

	/* 命令执行成功，发送I2C总线停止信号 */
	i2c_stop();
	return 1;

cmd_fail: /* 命令执行失败后，切记发送停止信号，避免影响I2C总线上其他设备 */
	/* 发送I2C总线停止信号 */
	i2c_stop();
	return 0;
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
