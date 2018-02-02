#ifndef _DEVICE_MANANGER_H
#define _DEVICE_MANANGER_H

typedef void (*modbus_api_fun)(u8* data, u8 len);
struct modbus_data_t{
        u8 type;        // reg or coli
        u8 mode;        // rw
        u16 addr;
        u8 len;
        u8* data;
        modbus_api_fun cb;
};

#define MODBUS_REG              0x01
#define MODBUS_COLI             0x02
#define MODBUS_MODE_R           0x01
#define MODBUS_MODE_W           0x02
#define MODBUS_MODE_API         0x04
#define MODBUS_MODE_RW          (MODBUS_MODE_R | MODBUS_MODE_W)
#define MODBUS_MODE_RWAPI       (MODBUS_MODE_R | MODBUS_MODE_W | MODBUS_MODE_API)

#define DEVICE_NOT_SUPPORT      0x00
#define DEVICE_OK               0x01

#define DAYS_TO_TICK(x)         (TickType_t)((60 * 1000 * 60 * 24) * (x))
#define TICKS_TO_DAYS(x)        (u16)((x) / (60 * 1000 * 60 * 24))

void devic_runtime_inc(void);
void devic_info_init(void);
void get_device_info(u8 *data, u8* len);
void update_device_info(u8* data);
u8 device_check_locate(u8 building_num,u8 floor_num,u8 room_num);
void device_monitor(u8* buff, u16 *len, u8 *changed);
u8 device_data_process(u8 type, u8 mode, u16 addr, u8* data, u8 len);
void devic_runtime_inc(void);
#endif
