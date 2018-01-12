#ifndef _IAP_H
#define _IAP_H

typedef void (*usr_app_fun)(void);
#define APP_VETOR_TABLE_ADDR   0xA000

void iap_start(void);
void iap_soft_reset(void);
void iap_done(void);
void iap_init(void);
u8 iap_software_check(u8* soft_ver);
#endif

