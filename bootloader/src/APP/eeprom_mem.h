#ifndef _EEPROM_MEM_H
#define _EEPROM_MEM_H

void ee_recevor_usr_app(u32 *usr_app);
void read_server_ip(u8 *ip,u16 *port);
void update_server_ip(u8 *ip, u16 port);

#endif

