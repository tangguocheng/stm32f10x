#ifndef _IAP_H
#define _IAP_H

typedef void (*usr_app_fun)(void);
#define APPLICATION_ADDRESS     (u32)0x08010000 
#define APP1_VETOR_TABLE_ADDR   0x3000
#define APP2_VETOR_TABLE_ADDR   0x1C000

void iap_start(void);
void iap_soft_reset(void);
void iap_done(void);
void iap_init(void);
#endif

