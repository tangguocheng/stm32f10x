#ifndef _PORT_H
#define _PORT_H

#undef INLINE
#define INLINE                  inline

#define assert(x)              configASSERT(x)

#define PR_BEGIN_EXTERN_C       extern "C" {
#define PR_END_EXTERN_C         }


#ifndef TRUE
#define TRUE                    1
#endif

#ifndef FALSE
#define FALSE                   0
#endif

#ifdef __cplusplus
PR_BEGIN_EXTERN_C
#endif
#define MB_TCP_DEBUG            1       /* Debug output in TCP module. */
/* ----------------------- Type definitions ---------------------------------*/
typedef char    BOOL;

typedef unsigned char UCHAR;
typedef char    CHAR;

typedef unsigned short USHORT;
typedef short   SHORT;

typedef unsigned long ULONG;
typedef long    LONG;


#define ENTER_CRITICAL_SECTION()       
#define EXIT_CRITICAL_SECTION()        

#ifdef __cplusplus
PR_END_EXTERN_C
#endif
#endif
