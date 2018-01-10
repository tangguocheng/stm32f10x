#ifndef __BSP_LED_DISPLAY_H
#define __BSP_LED_DISPLAY_H

#define  LED_TYPE_NUM           0x01
#define  LED_TYPE_INFO          0x02
#define  ERR_VALUE_0            0x00
#define  ERR_VALUE_1            0x01
#define  ERR_VALUE_2            0x02
#define  ERR_VALUE_3            0x03
#define  ERR_VALUE_4            0x04
#define  ERR_VALUE_5            0x05
#define  ERR_VALUE_6            0x06
#define  ERR_VALUE_7            0x07
#define  ERR_VALUE_8            0x08
#define  UPDATE_VALUE           0x10
#define  APP_VALUE              0x20
void led_display_init(void);
void led_display_test(void);
void set_led_content(u8 type, u8 value);
void led_display(void);
void time_cnt_down(void);
#endif
