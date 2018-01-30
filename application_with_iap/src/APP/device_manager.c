#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "bsp_w5500_port.h"
#include "eeprom_mem.h"
#include "device_manager.h"
#include "mb.h"

static u8 device_dhcp[4] = {192,168,1,1};
static u8 building_num = 1;
static u8 floor_num = 1;
static u8 room_num = 101;
static u8 software_version[3] = {0x01,0x00,0x00};       // zz.yyxx
volatile u32 runtime = DAYS_TO_TICK(2);
static u8 monitor_data_changed = 0;

static u8 card_type;
static u8 today_use_energy;
static u8 EXP[2];                 // big endding
static u8 light_fan_state[4];
static u8 curtain_state;
static u8 service_info;
static u8 guest_room_airc_ctl[2];  // 客房空调控制字
static u8 living_room_airc_ctl[2];  // 客厅空调控制字
static u8 study_room_airc_ctl[2];  // 书房空调控制字
static u8 scene_mode;
static u8 guest_room_tv_ctl;
static u8 living_room_tv_ctl;

static u8 gate_mag_delay;
static u8 wc_infrade_delay;
static u8 card_out_delay;
static u8 fan_open_interval;
static u8 fan_open_time;
static u8 airc_temp_thres_max;
static u8 daily_settlement_energy;
static u8 card_in_correlation;          // 插卡关联
static u8 card_out_correlation;         // 拔卡关联
static u8 season;
static u8 summer_mode;
static u8 summer_temp;
static u8 winter_mode;
static u8 winter_temp;

static u8 light_fan_coli[40];
static u8 curtain_state_coli[8];
static u8 service_info_coli[8];
static u8 airc1_ctl_coli[8];
static u8 airc2_ctl_coli[8];
static u8 airc3_ctl_coli[8];
static u8 scene_mode_coli[8];
static u8 guest_room_tv_ctl_coli[8];
static u8 living_room_tv_ctl_coli[8];

void device_data_changed(u8* data, u8 len)
{
        monitor_data_changed = 1;
}

const struct modbus_data_t modbus_data[] = {
        {MODBUS_REG,    MODBUS_MODE_R,        0x0000,   0x01,   &card_type,             device_data_changed},
        {MODBUS_REG,    MODBUS_MODE_R,        0x0001,   0x01,   &today_use_energy,      device_data_changed},
        {MODBUS_REG,    MODBUS_MODE_R,        0x0002,   0x02,   EXP,                    device_data_changed},
        {MODBUS_REG,    MODBUS_MODE_RWAPI,    0x0004,   0x01,   &light_fan_state[0],    device_data_changed},
        {MODBUS_REG,    MODBUS_MODE_RWAPI,    0x0005,   0x01,   &light_fan_state[1],    device_data_changed},
        {MODBUS_REG,    MODBUS_MODE_RWAPI,    0x0006,   0x01,   &light_fan_state[2],    device_data_changed},
        {MODBUS_REG,    MODBUS_MODE_RWAPI,    0x0007,   0x01,   &light_fan_state[3],    device_data_changed},
        {MODBUS_REG,    MODBUS_MODE_RWAPI,    0x0008,   0x01,   &curtain_state,         device_data_changed},
        {MODBUS_REG,    MODBUS_MODE_RWAPI,    0x0009,   0x01,   &service_info,          device_data_changed},
        {MODBUS_REG,    MODBUS_MODE_RWAPI,    0x000A,   0x02,   guest_room_airc_ctl,    device_data_changed},
        {MODBUS_REG,    MODBUS_MODE_RWAPI,    0x000C,   0x02,   living_room_airc_ctl,   device_data_changed},
        {MODBUS_REG,    MODBUS_MODE_RWAPI,    0x000E,   0x02,   study_room_airc_ctl,    device_data_changed},
        {MODBUS_REG,    MODBUS_MODE_W,        0x0010,   0x01,   &scene_mode,            device_data_changed},
        {MODBUS_REG,    MODBUS_MODE_W,        0x0011,   0x01,   &guest_room_tv_ctl,     NULL},
        {MODBUS_REG,    MODBUS_MODE_W,        0x0012,   0x01,   &living_room_tv_ctl,    NULL},
        
        {MODBUS_REG,    MODBUS_MODE_W,        0x0050,   0x01,   &gate_mag_delay,        NULL},
        {MODBUS_REG,    MODBUS_MODE_W,        0x0051,   0x01,   &wc_infrade_delay,      NULL},
        {MODBUS_REG,    MODBUS_MODE_W,        0x0052,   0x01,   &card_out_delay,        NULL},
        {MODBUS_REG,    MODBUS_MODE_W,        0x0053,   0x01,   &fan_open_interval,     NULL},
        {MODBUS_REG,    MODBUS_MODE_W,        0x0054,   0x01,   &fan_open_time,         NULL},
        {MODBUS_REG,    MODBUS_MODE_W,        0x0055,   0x01,   &airc_temp_thres_max,   NULL},
        {MODBUS_REG,    MODBUS_MODE_W,        0x0056,   0x01,   &daily_settlement_energy,    NULL},
        {MODBUS_REG,    MODBUS_MODE_W,        0x0057,   0x01,   &card_in_correlation,           NULL},
        {MODBUS_REG,    MODBUS_MODE_W,        0x0058,   0x01,   &card_out_correlation,          NULL},
        {MODBUS_REG,    MODBUS_MODE_W,        0x0059,   0x01,   &season,         NULL},
        {MODBUS_REG,    MODBUS_MODE_W,        0x005A,   0x01,   &summer_mode,    NULL},
        {MODBUS_REG,    MODBUS_MODE_W,        0x005B,   0x01,   &summer_temp,    NULL},
        {MODBUS_REG,    MODBUS_MODE_W,        0x005C,   0x01,   &winter_mode,    NULL},
        {MODBUS_REG,    MODBUS_MODE_W,        0x005D,   0x01,   &winter_temp,    NULL},
        
        {MODBUS_REG,    MODBUS_MODE_RW,       0x0100,   0x04,   device_dhcp,          NULL},
        {MODBUS_REG,    MODBUS_MODE_RW,       0x0104,   0x04,   w5500_eth_info.gw,    NULL},
        {MODBUS_REG,    MODBUS_MODE_RW,       0x0108,   0x04,   w5500_eth_info.ip,    NULL},
        {MODBUS_REG,    MODBUS_MODE_RW,       0x010C,   0x04,   w5500_eth_info.sn,    NULL},
        {MODBUS_REG,    MODBUS_MODE_RW,       0x0110,   0x02,   EXP,                  NULL},
        {MODBUS_REG,    MODBUS_MODE_R,        0x0112,   0x01,   &building_num,        NULL},
        {MODBUS_REG,    MODBUS_MODE_R,        0x0113,   0x01,   &floor_num,           NULL},
        {MODBUS_REG,    MODBUS_MODE_R,        0x0114,   0x01,   &room_num,            NULL},
        {MODBUS_REG,    MODBUS_MODE_R,        0x0115,   0x03,   software_version,     NULL},
        
        {MODBUS_COLI,   MODBUS_MODE_RW,       0x0100,   0x03,   light_fan_coli,        NULL},
        {MODBUS_COLI,   MODBUS_MODE_RW,       0x0130,   0x03,   curtain_state_coli,    NULL},
        {MODBUS_COLI,   MODBUS_MODE_RW,       0x0138,   0x03,   service_info_coli,     NULL},
        {MODBUS_COLI,   MODBUS_MODE_RW,       0x0140,   0x03,   airc1_ctl_coli,        NULL},
        {MODBUS_COLI,   MODBUS_MODE_RW,       0x0148,   0x03,   airc2_ctl_coli,        NULL},
        {MODBUS_COLI,   MODBUS_MODE_RW,       0x0150,   0x03,   airc3_ctl_coli,        NULL},
        {MODBUS_COLI,   MODBUS_MODE_RW,       0x0158,   0x03,   scene_mode_coli,       NULL},
        {MODBUS_COLI,   MODBUS_MODE_RW,       0x0160,   0x03,   guest_room_tv_ctl_coli,     NULL},
        {MODBUS_COLI,   MODBUS_MODE_RW,       0x0168,   0x03,   living_room_tv_ctl_coli,    NULL},
        
};

#define MODBUS_DATA_LEN         (sizeof(modbus_data)/sizeof(struct modbus_data_t))
     
void devic_runtime_inc(void)
{
        static u16 last_day = 2;
        if (runtime)
                runtime--;
        
        u16 day = TICKS_TO_DAYS(runtime);
        
        if (day != last_day) {
                last_day = day;
                EXP[0] = (u8)(day & 0xFF);
                EXP[1] = (u8)((day >> 8) & 0xFF);
                device_data_changed(NULL,0);
        }
}

void devic_info_init(void)
{
        eeprom_init();        
        u8 eeprom_data[6];
        ee_read_bytes_os(eeprom_data,MEM_INITFL_ADDR,MEM_INITFL_LEN);
        if ((eeprom_data[0] == MEM_INITFL_DATA1) && (eeprom_data[1] == MEM_INITFL_DATA2)) {
                // restore system info
                ee_read_bytes_os(device_dhcp,MEM_DHCP_ADDR,0x04);
                ee_read_bytes_os(w5500_eth_info.gw,MEM_GATEWAY_ADDR,0x04);
                ee_read_bytes_os(w5500_eth_info.ip,MEM_LOCAL_IP_ADDR,0x04);
                ee_read_bytes_os(w5500_eth_info.sn,MEM_SUBNET_MASK_ADDR,0x04);
                ee_read_bytes_os(eeprom_data,MEM_HOME_ADDRESS_ADDR,0x03);
                building_num = eeprom_data[0];
                floor_num = eeprom_data[1];
                room_num = eeprom_data[2];
                ee_read_bytes_os(software_version,MEM_FIRMWARE_VER_ADDR,0x03);                
        } else {
                eeprom_data[0] = MEM_INITFL_DATA1;
                eeprom_data[1] = MEM_INITFL_DATA2;
                ee_write_bytes_os(eeprom_data,MEM_INITFL_ADDR,MEM_INITFL_LEN);
                ee_write_bytes_os(device_dhcp,MEM_DHCP_ADDR,0x04);
                ee_write_bytes_os(w5500_eth_info.gw,MEM_GATEWAY_ADDR,0x04);
                ee_write_bytes_os(w5500_eth_info.ip,MEM_LOCAL_IP_ADDR,0x04);
                ee_write_bytes_os(w5500_eth_info.sn,MEM_SUBNET_MASK_ADDR,0x04);
                eeprom_data[0] = building_num;
                eeprom_data[1] = floor_num;
                eeprom_data[2] = room_num;
                ee_write_bytes_os(eeprom_data,MEM_HOME_ADDRESS_ADDR,0x03);
                ee_write_bytes_os(software_version,MEM_FIRMWARE_VER_ADDR,0x03);
        }        
}

void get_device_info(u8 *data, u8* len)
{
        u8 idx = 0;
        // dhcp
        for (u8 i = 0; i < 4; i++) {
                data[idx] = device_dhcp[i];
                idx++;
        }        
        
        // gw 
        for (u8 i = 0; i < 4; i++) {
                data[idx] = w5500_eth_info.gw[i];
                idx++;
        }        
        
        // ip
        for (u8 i = 0; i < 4; i++) {
                data[idx] = w5500_eth_info.ip[i];
                idx++;
        }     

        // sn
        for (u8 i = 0; i < 4; i++) {
                data[idx] = w5500_eth_info.sn[i];
                idx++;
        } 
        
        // run time
        u16 days = (u16)TICKS_TO_DAYS(runtime);    
        data[idx] = (u8)((days >> 8) & 0xFF);
        idx++;
        
        data[idx] = (u8)(days & 0xFF);  
        idx++;    
        
        // building number
        data[idx] = building_num;
        idx++;
        
        // floor number
        data[idx] = floor_num;
        idx++;
        
        // room number
        data[idx] = room_num;
        idx++;
        
        // main version
        data[idx] = software_version[0];
        idx++;
        
        // subversion
        data[idx] = software_version[1];
        idx++;
        data[idx] = software_version[2];
        idx++;
        
        *len = idx;
}

void update_device_info(u8* data)
{
        u8 idx = 0;
        // dhcp
        for (u8 i = 0; i < 4; i++) {
                device_dhcp[i] = data[idx];
                idx++;
        }        
        
        // gw 
        for (u8 i = 0; i < 4; i++) {
                w5500_eth_info.gw[i] = data[idx];
                idx++;
        }        
        
        // ip
        for (u8 i = 0; i < 4; i++) {
                w5500_eth_info.ip[i] = data[idx] ;
                idx++;
        }     

        // sn
        for (u8 i = 0; i < 4; i++) {
                w5500_eth_info.sn[i] = data[idx] ;
                idx++;
        } 
        
        // run time        
        runtime = DAYS_TO_TICK(((data[idx] << 8) & 0xFF00) | data[idx + 1]);  
        
        ee_write_bytes_os(device_dhcp,MEM_DHCP_ADDR,0x04);
        ee_write_bytes_os(w5500_eth_info.gw,MEM_GATEWAY_ADDR,0x04);
        ee_write_bytes_os(w5500_eth_info.ip,MEM_LOCAL_IP_ADDR,0x04);
        ee_write_bytes_os(w5500_eth_info.sn,MEM_SUBNET_MASK_ADDR,0x04);
        w5500_ip_assign();
}

u8 device_check_locate(u8 bd_num,u8 fl_num,u8 rm_num)
{
        return (( (bd_num == building_num) && (fl_num == floor_num) && (rm_num == room_num) ) ? 1 : 0);
}

static u8 get_modbus_data(u8 type, u16 addr, u16 *idx)
{
        u16 i;
        for (i = 0; i < MODBUS_DATA_LEN; i++) {
                if ((addr >= modbus_data[i].addr) && (addr <= modbus_data[i].addr + modbus_data[i].len) &&
                    (type == modbus_data[i].type))
                        break;
        }
        
        *idx = i;
        return ((i >= MODBUS_DATA_LEN) ? DEVICE_NOT_SUPPORT : DEVICE_OK);
}

//typedef enum
//{
//    MB_ENOERR,                  /*!< no error. */
//    MB_ENOREG,                  /*!< illegal register address. */
//    MB_EINVAL,                  /*!< illegal argument. */
//    MB_EPORTERR,                /*!< porting layer error. */
//    MB_ENORES,                  /*!< insufficient resources. */
//    MB_EIO,                     /*!< I/O error. */
//    MB_EILLSTATE,               /*!< protocol stack in illegal state. */
//    MB_ETIMEDOUT                /*!< timeout error occurred. */
//} eMBErrorCode;

u8 device_data_process(u8 type, u8 mode, u16 addr, u8* data, u8 len)
{
        u8 rtl = DEVICE_OK; 
        u16 rtl_idx;
        
        rtl = get_modbus_data(type, addr, &rtl_idx);
        if (rtl != DEVICE_OK)
                return (MB_ENOREG);
        
        // test mode
        if (!(modbus_data[rtl_idx].mode & mode))
                return (MB_EINVAL);
        
        u8 offset = addr - modbus_data[rtl_idx].addr;
        rtl = MB_ENOERR;
        if ((type == MODBUS_REG) || (type == MODBUS_COLI)) {
                switch (mode) {
                case MODBUS_MODE_R:
                        for (u8 i = 0; i < len; i++)
                                 data[i] = modbus_data[rtl_idx].data[i + offset];
                        break;
                
                case MODBUS_MODE_W:
                        for (u8 i = 0; i < len; i++)
                                 modbus_data[rtl_idx].data[i] = data[i + offset];
                        break;
                
                case MODBUS_MODE_API:
                        for (u8 i = 0; i < len; i++)
                                 modbus_data[rtl_idx].data[i] = data[i + offset];
                        
                        if (modbus_data[rtl_idx].cb != NULL)
                                modbus_data[rtl_idx].cb(modbus_data[rtl_idx].data,modbus_data[rtl_idx].len);
                        
                        break;
                
                default:
                        rtl = DEVICE_NOT_SUPPORT;
                        break;
                }
        } else {
                rtl = MB_EINVAL;
        }
        
        return (rtl);
}

void device_monitor(u8* buff, u16 *len, u8 *changed)
{
        u8 idx = 0;
        if (monitor_data_changed == 0) {
                *changed = 0;
                return;
        }
        
        monitor_data_changed = 0;
        
        *changed = 1;
        *len = 17;

        buff[idx++] = 0x00;
        buff[idx++] = 0x00;
        buff[idx++] = 0x00;
        buff[idx++] = 0x00;
        buff[idx++] = (u8)(((*len + 1) >> 8) & 0xFF);
        buff[idx++] = (u8)((*len + 1) & 0xFF);
        buff[idx++] = 0x01;
        buff[idx++] = 0x03;
        buff[idx++] = 0x00;
        buff[idx++] = 0x00;
        buff[idx++] = 0x00;
        buff[idx++] = 0x11;
        
        buff[idx++] = card_type;
        buff[idx++] = today_use_energy;
        buff[idx++] = EXP[0];
        buff[idx++] = EXP[1];
        buff[idx++] = light_fan_state[0];
        buff[idx++] = light_fan_state[1];
        buff[idx++] = light_fan_state[2];
        buff[idx++] = light_fan_state[3];
        
        buff[idx++] = curtain_state;
        buff[idx++] = service_info;
        buff[idx++] = guest_room_airc_ctl[0];
        buff[idx++] = guest_room_airc_ctl[1];
        buff[idx++] = living_room_airc_ctl[0];
        buff[idx++] = living_room_airc_ctl[1];
        buff[idx++] = study_room_airc_ctl[0];
        buff[idx++] = study_room_airc_ctl[1];
        buff[idx++] = scene_mode;
        
        *len = idx;
}

