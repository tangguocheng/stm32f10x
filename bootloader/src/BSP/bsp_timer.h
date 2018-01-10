#ifndef __TIMER_H
#define __TIMER_H

void sys_timer_init(u8 pre_priority,u8 sub_priority,u8 enable);
extern volatile u32 sys_us;
#endif
