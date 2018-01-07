#include "stm32f10x.h"
#include "stm_flash.h"
#include "IAP.h"

void start_user_app(void)
{
        if(((*(vu32*)APPLICATION_ADDRESS) & 0x2FFE0000) == 0x20000000) {
                u32 usr_app_addr = *(__IO u32*) (APPLICATION_ADDRESS + 4);
                usr_app_fun user_application = (usr_app_fun) usr_app_addr;
                __set_MSP(*(__IO uint32_t*)APPLICATION_ADDRESS);
                user_application();
        }
}

u16 iapbuf[1024]; 
void iap_update_app(u8 *appbuf,u32 appsize)
{
	u16 t;
	u16 i = 0;
	u16 temp;
	u32 fwaddr = APPLICATION_ADDRESS;//当前写入的地址
	u8 *dfu=appbuf;
	for(t = 0; t < appsize; t+=2)
	{						    
		temp=(u16)dfu[1]<<8;
		temp+=(u16)dfu[0];	  
		dfu+=2;//偏移2个字节
		iapbuf[i++]=temp;	    
		if(i==1024)
		{
			i = 0;
			STMFLASH_Write(fwaddr,iapbuf,1024);	
			fwaddr+=2048;//偏移2048  16=2*8.所以要乘以2.
		}
	}
	if(i)  STMFLASH_Write(fwaddr,iapbuf,i);//将最后的一些内容字节写进去.  
}

