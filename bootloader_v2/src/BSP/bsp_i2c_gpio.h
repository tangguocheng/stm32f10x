/*
*********************************************************************************************************
*
*	模块名称 : I2C总线驱动模块
*	文件名称 : bsp_i2c_gpio.h
*	版    本 : V1.0
*	说    明 : 头文件。
*
*	Copyright (C), 2012-2013, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#ifndef _BSP_I2C_GPIO_H
#define _BSP_I2C_GPIO_H

#define I2C_WR	0		/* 写控制bit */
#define I2C_RD	1		/* 读控制bit */

void i2c_gpio_init(void);
void i2c_start(void);
void i2c_stop(void);
void i2c_send_byte(u8 _ucByte);
u8 i2c_read_byte(void);
u8 i2c_wait_ack(void);
void i2c_ack(void);
void i2c_nack(void);
u8 i2c_check_connect(u8 _Address);

#endif
