- port_reg.c 中四个操作函数实现
- RS485
- Modbus TCP 端口号502
- socket接收数据缓存到aucTCPBuf
- 那些轮询的数据项开机是要主动上报
#define USER_APP_ADDR 0x800A000    

bootloader -- 30K   >>>>>>>>>>>>>>>>>>  0x08000000 - 0x08007800
uar_app    -- 126k  >>>>>>>>>>>>>>>>>>  0x08007800 - 0x08027000    
app_data   -- 100K  >>>>>>>>>>>>>>>>>>  0x08027000 - 0x08040000