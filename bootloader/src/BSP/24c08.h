#ifndef __24c16_H
#define __24c16_H

#include "stm32f10x.h"

#define I2C_SCK	        GPIO_Pin_6  //out
#define I2C_SDA	        GPIO_Pin_7
void at24c16_init(void);
void at24c16_write(u16 addr, unsigned char val);
unsigned char at24c16_read(u16 addr);

void eep_block_write(u16 eepAddr, u8* dat, u16 index, u16 len);

void erase_eeprom(u16 startAddr, u16 len);

#endif /* __MAIN_H */




