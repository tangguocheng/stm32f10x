#ifndef _PROJ_CONF_H
#define _PROJ_CONF_H

#define DEBUG_OUTPUT_ENABLE     1
#define LOG_INFO        "[Info] "
#define LOG_ERR         "[ Err] "
#define LOG_WARN        "[Warn] "

#if DEBUG_OUTPUT_ENABLE == 1
#include <stdio.h>
//        extern void  App_Printf(char *format, ...);
        #define WSY_LOG_OUT(x,...)  printf(x,##__VA_ARGS__) //App_Printf(x,##__VA_ARGS__) 
#else
        #define WSY_LOG_OUT(x,...)  
#endif
        
extern void printf_simple_mutex(char* , ...);

#endif

