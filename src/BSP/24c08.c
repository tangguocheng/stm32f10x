#include "stm32f10x.h"
#include "24c08.h"
#include "delay.h"


void at24c16_init(void)
{
        GPIO_InitTypeDef GPIO_InitStructure;

        //Port B bidirection SDA
        GPIO_InitStructure.GPIO_Pin = I2C_SDA;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
        GPIO_Init(GPIOB, &GPIO_InitStructure);

        //Port B output
        GPIO_InitStructure.GPIO_Pin = I2C_SCK;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void at24c16_start(void)
{
        GPIO_SetBits(GPIOB, I2C_SDA);
        GPIO_SetBits(GPIOB, I2C_SCK);
        delay_nus(1);
        GPIO_ResetBits(GPIOB, I2C_SDA);
        GPIO_ResetBits(GPIOB, I2C_SCK);
}

void at24c16_stop(void)
{
        GPIO_ResetBits(GPIOB, I2C_SDA);
        GPIO_SetBits(GPIOB, I2C_SCK);
        delay_nus(1);
        GPIO_SetBits(GPIOB, I2C_SDA);
}

void write_1byte(unsigned char val)
{
        unsigned char i, tmp;
        tmp=val;

        for (i=0; i<8; i++) {
                GPIO_ResetBits(GPIOB, I2C_SCK);
                delay_nus(1);
                if (((tmp<<i) & 0x80)==0x80)
                        GPIO_SetBits(GPIOB, I2C_SDA);
                else
                        GPIO_ResetBits(GPIOB, I2C_SDA);
                delay_nus(1);
                GPIO_SetBits(GPIOB, I2C_SCK);
                delay_nus(1);
        }
        GPIO_ResetBits(GPIOB, I2C_SCK);
        delay_nus(1);
        GPIO_SetBits(GPIOB, I2C_SDA);
        delay_nus(1);
}


unsigned char read_1byte(void)
{
        unsigned char i,j,k=0;

        GPIO_ResetBits(GPIOB, I2C_SCK);
        delay_nus(1);
        GPIO_SetBits(GPIOB, I2C_SDA);
        for (i=0; i<8; i++) {
                delay_nus(1);
                GPIO_SetBits(GPIOB, I2C_SCK);
                delay_nus(1);

                if(GPIO_ReadInputDataBit(GPIOB, I2C_SDA)==1) j=1; //???SDA becoms inout pin from output pin;
                else j=0;
                k=(k<<1) | j;
                GPIO_ResetBits(GPIOB, I2C_SCK);
        }
        delay_nus(1);
        return (k);
}

void clock(void)
{
        u16 i=0;

        GPIO_SetBits(GPIOB, I2C_SCK);
        delay_nus(1);

        while((GPIO_ReadInputDataBit(GPIOB, I2C_SDA)==1) && (i<255))
                i++;
        GPIO_ResetBits(GPIOB, I2C_SCK);
        delay_nus(1);
}

void at24c16_write(u16 addr, u8 val)
{
        u8 l_addr;

        __disable_irq();
        at24c16_start();
        //write_1byte(0xa0);
        //clock();
        //u8 h_addr=addr>>8;
        //write_1byte(h_addr);
        write_1byte(0xa0 | ((addr>>7 & 0xfe)));
        clock();
        l_addr=(addr%256);
        write_1byte(l_addr);
        clock();
        write_1byte(val);
        clock();
        at24c16_stop();
        __enable_irq();
        delay_nms(5);
}

unsigned char at24c16_read(u16 addr)
{
        //u8 high,low;
        u8 i;
        u8 low;
        low = addr & 0x00ff;
        //high=(addr & 0xff00)>>8;

        __disable_irq();
        at24c16_start();
        //write_1byte(0xa0);
        //clock();
        //write_1byte(high);
        write_1byte(0xa0 | ((addr>>7 & 0xfe)));
        clock();
        write_1byte(low);
        clock();
        at24c16_start();
        write_1byte(0xa1);
        clock();
        i=read_1byte();
        at24c16_stop();
        //delay_nus(5);
        __enable_irq();
        return(i);
}
//eep block write
//eepAddr: eeprom start address
//dat: data array to be saved to eeprom
//index: data array start index
//len: how long to be write
void eep_block_write(u16 eepAddr, u8* dat, u16 index, u16 len)
{
        u16 i;
        for(i=0; i<len; i++) {
                at24c16_write(eepAddr+i, dat[index+i]);
        }
}

void erase_eeprom(u16 startAddr, u16 len)
{
        u16 i;
        for(i=startAddr; i<startAddr+len; i++) {
                at24c16_write(i,0xff);
        }
}

