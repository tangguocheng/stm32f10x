#ifndef _EEPROM_MEM_H
#define _EEPROM_MEM_H

void eeprom_first_burn(void);
void read_server_ip(u8 *ip,u16 *port);
void update_server_ip(u8 *ip, u16 port);

#endif

