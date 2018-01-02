#ifndef _EEPROM_MEM_H
#define _EEPROM_MEM_H

#define EEPROM_ENTER_CRITICAL            vPortEnterCritical
#define EEPROM_EXIT_CRITICAL             vPortExitCritical

//	#define EE_PAGE_SIZE		16
//	#define EE_SIZE                 (1 * 1024)

#define MEM_START_ADDR                  0x0000

#define MEM_USER_APP_ADDR               MEM_START_ADDR + 2

#define MEM_ORIGIN_START                MEM_USER_APP_ADDR + 4 * 2

#define MEM_INITFL_ADDR                 MEM_ORIGIN_START + 2                    
#define MEM_INITFL_DATA1                0x5A
#define MEM_INITFL_DATA2                0xA5
#define MEM_INITFL_LEN                  0x02                            
#define MEM_SERVER_IP_ADDR              (MEM_INITFL_ADDR + MEM_INITFL_LEN)      // server infomation
#define MEM_SUBNET_MASK_ADDR            (MEM_SERVER_IP_ADDR + (4 + 2))
#define MEM_LOCAL_IP_ADDR               (MEM_SUBNET_MASK_ADDR + 4)
#define MEM_GATEWAY_IP_ADDR             (MEM_LOCAL_IP_ADDR + 4)
#define MEM_HOME_ADDRESS_ADDR           (MEM_GATEWAY_IP_ADDR + 4)
#define MEM_EXP_ADDR                    (MEM_HOME_ADDRESS_ADDR + 3)
#define MEM_SERIAL_NUM_ADDR             (MEM_EXP_ADDR + 2)
#define MEM_FIRMWARE_VER_ADDR           (MEM_SERIAL_NUM_ADDR + 4)
#define MEM_FIRMWARE_VER_DATA1          0xCA
#define MEM_FIRMWARE_VER_DATA2          0xCA
#define MEM_PASSWD_ADDR                 (MEM_FIRMWARE_VER_ADDR + 2)             // password

#define MEM_ORIGIN_END                  (MEM_PASSWD_ADDR + 2)

#define MEM_ORIGIN                      MEM_START_ADDR
#define MEM_BACKUP_1                    (MEM_ORIGIN + MEM_ORIGIN_END - MEM_ORIGIN_START)

void eeprom_first_burn(void);
void read_server_ip(u8 *ip,u16 *port);
void update_server_ip(u8 *ip, u16 port);
void ee_store_app_addr(u32 *usr_app);
void ee_recevor_app_addr(u32 *usr_app);
#endif

