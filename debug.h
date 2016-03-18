#ifndef __DEBUG_H__
#define __DEBUG_H__


#define DEBUG_PREFIX "--Tomato--"

//Debug information verbosity: lower values indicate higher urgency
#define DEBUG_OFF 0
#define DEBUG_ERR 1
#define DEBUG_WRN 2
#define debug_TRC 3
#define DEBUG_INF 4

extern int DEBUG_LEVEL;

#define DEBUG_off(msg...) \
do{ \
    if (DEBUG_LEVEL>=DEBUG_OFF) { \
        printf(DEBUG_PREFIX msg); \
    } \
 }while(0)
            
#define debug_err(msg...) \
do{ \
    if (DEBUG_LEVEL>=DEBUG_ERR) { \
        printf(DEBUG_PREFIX msg); \
    } \
 }while(0)

#define debug_wrn(msg...) \
do{ \
    if (DEBUG_LEVEL>=DEBUG_WRN) { \
        printf(DEBUG_PREFIX msg); \
    } \
 }while(0)

#define debug_trc(msg...) \
do{ \
    if (DEBUG_LEVEL>=DEBUG_TRC) { \
        printf(DEBUG_PREFIX msg); \
    } \
 }while(0)

#define debug_inf(msg...) \
do{ \
    if (DEBUG_LEVEL>=DEBUG_INF) { \
        printf(DEBUG_PREFIX msg); \
    } \
 }while(0)

#define debug(level, msg...) debug_##level(msg)

#undef assert
#define assert(x) { \
    if (!(x)) { \
        printf(__FILE__ ":%d assert " #x " failed\n", __LINE__);    \
    }\
}

#endif
//Usage:
//just include this file and define debug_LEVEL like
//
//int DEBUG_LEVEL=DEBUG_ERR
//
//in the program use the command by
//DEBUG(off|err|wrn|trc|inf, "test %d with %s", 100, "this");