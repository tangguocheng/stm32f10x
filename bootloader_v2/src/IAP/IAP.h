#ifndef _IAP_H
#define _IAP_H

typedef void (*usr_app_fun)(void);
#define USER_APP_ADDR 0x800A000                 // bootloader 40K

u8 iap_software_check(u8* soft_ver);
void iap_soft_reset(void);
void iap_done(void);
void iap_init(void);
void iap_write_app_data(u16 offset, u8 *appbuf, u16 appsize);
void start_user_app(void);
#endif

