#ifndef _IAP_H
#define _IAP_H

typedef void (*usr_app_fun)(void);
#define USER_APP_ADDR                   0x08007800                 // 126k app flash
#define UPDATE_CACHE_ADDR               0x08027000                 // 100k flash cache
#define UPDATE_CACHE_LEN                (100 * 1024)

u8 iap_software_check(u8* soft_ver);
void iap_soft_reset(void);
void iap_done(void);
void iap_init(void);
void iap_write_app_data(u16 offset, u8 *appbuf, u16 appsize);
void start_user_app(void);
#endif

