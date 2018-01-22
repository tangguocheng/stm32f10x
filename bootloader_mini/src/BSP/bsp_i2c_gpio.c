
/*
	应用说明：
	在访问I2C设备前，请先调用 i2c_CheckDevice() 检测I2C设备是否正常，该函数会配置GPIO
*/

#include "stm32f10x.h"
#include "bsp_i2c_gpio.h"
#include "bsp_delay.h"

#define RCC_I2C_PORT 		RCC_APB2Periph_GPIOB

#define PORT_I2C_SCL		GPIOB
#define PIN_I2C_SCL             GPIO_Pin_6

#define PORT_I2C_SDA		GPIOB
#define PIN_I2C_SDA             GPIO_Pin_7

#define I2C_SCL_PIN             GPIO_Pin_6
#define I2C_SDA_PIN             GPIO_Pin_7

#define I2C_SCL_1()  		do{ PORT_I2C_SCL->BSRR = I2C_SCL_PIN; } while(0)
#define I2C_SCL_0()  		do{ PORT_I2C_SCL->BRR = I2C_SCL_PIN; } while(0)

#define I2C_SDA_1()  		do{ PORT_I2C_SDA->BSRR = I2C_SDA_PIN; } while(0)
#define I2C_SDA_0()  		do{ PORT_I2C_SDA->BRR = I2C_SDA_PIN; } while(0)

#define I2C_SDA_READ_STATE()  (((PORT_I2C_SDA->IDR & I2C_SDA_PIN) != 0) ? 1 : 0)
#define I2C_SCL_READ_STATE()  (((PORT_I2C_SCL->IDR & I2C_SCL_PIN) != 0) ? 1 : 0)


void i2c_gpio_init(void)
{
        GPIO_InitTypeDef GPIO_InitStructure;

        RCC_APB2PeriphClockCmd(RCC_I2C_PORT, ENABLE);	/* 打开GPIO时钟 */

        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;	/* 开漏输出模式 */

        GPIO_InitStructure.GPIO_Pin = PIN_I2C_SCL;
        GPIO_Init(PORT_I2C_SCL, &GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin = PIN_I2C_SDA;
        GPIO_Init(PORT_I2C_SDA, &GPIO_InitStructure);

        /* 给一个停止信号, 复位I2C总线上的所有设备到待机模式 */
        i2c_stop();
}

/*
*********************************************************************************************************
*	函 数 名: i2c_Delay
*	功能说明: I2C总线位延迟，最快400KHz
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
static void i2c_Delay(void)
{
        delay_nus(2);
}

/*
*********************************************************************************************************
*	函 数 名: i2c_start
*	功能说明: CPU发起I2C总线启动信号
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void i2c_start(void)
{
        I2C_SDA_1();
        I2C_SCL_1();
        i2c_Delay();
        I2C_SDA_0();
        i2c_Delay();

        I2C_SCL_0();
        i2c_Delay();
}

/*
*********************************************************************************************************
*	函 数 名: i2c_start
*	功能说明: CPU发起I2C总线停止信号
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void i2c_stop(void)
{
        I2C_SDA_0();
        I2C_SCL_1();
        i2c_Delay();
        I2C_SDA_1();
        i2c_Delay();
}

/*
*********************************************************************************************************
*	函 数 名: i2c_send_byte
*	功能说明: CPU向I2C总线设备发送8bit数据
*	形    参:  _ucByte ： 等待发送的字节
*	返 回 值: 无
*********************************************************************************************************
*/
void i2c_send_byte(u8 _ucByte)
{
        u8 i;

        for (i = 0; i < 8; i++) {
                if (_ucByte & 0x80) {
                        I2C_SDA_1();
                } else {
                        I2C_SDA_0();
                }
                i2c_Delay();
                I2C_SCL_1();
                i2c_Delay();
                I2C_SCL_0();
                if (i == 7) {
                        I2C_SDA_1(); // 释放总线
                }
                _ucByte <<= 1;	
                i2c_Delay();
        }
}

/*
*********************************************************************************************************
*	函 数 名: i2c_read_byte
*	功能说明: CPU从I2C总线设备读取8bit数据
*	形    参:  无
*	返 回 值: 读到的数据
*********************************************************************************************************
*/
u8 i2c_read_byte(void)
{
        u8 i;
        u8 value;

        /* 读到第1个bit为数据的bit7 */
        value = 0;
        for (i = 0; i < 8; i++) {
                value <<= 1;
                I2C_SCL_1();
                i2c_Delay();
                if (I2C_SDA_READ_STATE()) {
                        value++;
                }
                I2C_SCL_0();
                i2c_Delay();
        }
        return value;
}

/*
*********************************************************************************************************
*	函 数 名: i2c_wait_ack
*	功能说明: CPU产生一个时钟，并读取器件的ACK应答信号
*	形    参:  无
*	返 回 值: 返回0表示正确应答，1表示无器件响应
*********************************************************************************************************
*/
u8 i2c_wait_ack(void)
{
        u8 re;

        I2C_SDA_1();	/* CPU释放SDA总线 */
        i2c_Delay();
        I2C_SCL_1();	/* CPU驱动SCL = 1, 此时器件会返回ACK应答 */
        i2c_Delay();
        if (I2C_SDA_READ_STATE()) {	/* CPU读取SDA口线状态 */
                re = 1;
        } else {
                re = 0;
        }
        I2C_SCL_0();
        i2c_Delay();
        return re;
}

/*
*********************************************************************************************************
*	函 数 名: i2c_ack
*	功能说明: CPU产生一个ACK信号
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void i2c_ack(void)
{
        I2C_SDA_0();	/* CPU驱动SDA = 0 */
        i2c_Delay();
        I2C_SCL_1();	/* CPU产生1个时钟 */
        i2c_Delay();
        I2C_SCL_0();
        i2c_Delay();
        I2C_SDA_1();	/* CPU释放SDA总线 */
}

/*
*********************************************************************************************************
*	函 数 名: i2c_nack
*	功能说明: CPU产生1个NACK信号
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void i2c_nack(void)
{
        I2C_SDA_1();	/* CPU驱动SDA = 1 */
        i2c_Delay();
        I2C_SCL_1();	/* CPU产生1个时钟 */
        i2c_Delay();
        I2C_SCL_0();
        i2c_Delay();
}

/*
*********************************************************************************************************
*	函 数 名: i2c_CheckDevice
*	功能说明: 检测I2C总线设备，CPU向发送设备地址，然后读取设备应答来判断该设备是否存在
*	形    参:  _Address：设备的I2C总线地址
*	返 回 值: 返回值 0 表示正确， 返回1表示未探测到
*********************************************************************************************************
*/
u8 i2c_check_connect(u8 _Address)
{
        u8 ucAck;

        i2c_gpio_init();

        if (I2C_SDA_READ_STATE() && I2C_SCL_READ_STATE()) {
                i2c_start();		/* 发送启动信号 */

                /* 发送设备地址+读写控制bit（0 = w， 1 = r) bit7 先传 */
                i2c_send_byte(_Address | I2C_WR);
                ucAck = i2c_wait_ack();	/* 检测设备的ACK应答 */

                i2c_stop();			/* 发送停止信号 */

                return ucAck;
        }
        return 1;	/* I2C总线异常 */
}


