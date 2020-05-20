/*
 * Name:    debug.h
 *
 * Purpose: general debug system
 *
 * Created By:         dengshuo_buaa@163.com 
 * Created Date:       2020-01-31
 *
 * ChangeList:
 * init version
 */

#ifndef _DEBUG_H_
#define _DEBUG_H_
#include <stdio.h>


/* 颜色设置 */
#define GREEN       "\033[40;32m"
#define YELLOW      "\033[40;33m"
#define PURPLE      "\033[40;35m"
#define RED         "\033[40;31m"
#define RESET       "\033[0m"


/* 
 * debug level,
 * DEBUG_LEVEL_DISABLE  --   禁止打印调试信息
 * DEBUG_LEVEL_NOTICE   --   打印关键调试信息 
 * DEBUG_LEVEL_DEBUG    --   打印所有DEBUG/NOTICE调试信息
 * ALERT/INFO           --   无法屏蔽ALERT/INFO信息
 */
enum level {
    DEBUG_LEVEL_DISABLE = 0,
    DEBUG_LEVEL_NOTICE,
    DEBUG_LEVEL_DEBUG
};

int debug_level;

#define set_debug_level(level)  do{debug_level = level;}while(0)

/*
 *  alert   --  用于打印错误信息，不可屏蔽
 *  info    --  用于打印基础信息，不可屏蔽
 *  notice  --  用于打印关键调试信息，可屏蔽
 *  debug   --  用于打印全部调试信息，可屏蔽
 */

#define alert(fmt,args...)                                                                  \
    do {                                                                                    \
        printf("["RED"ALERT"RESET"]""[%s][%d]"fmt, __FUNCTION__, __LINE__, ##args);         \
    } while (0)                

#define info(fmt,args...)                                                                   \
    do {                                                                                    \
        printf("["PURPLE"INFO"RESET"]""[%s][%d]"fmt, __FUNCTION__, __LINE__, ##args);    \
    } while (0)                

#define notice(fmt,args...)                                                                 \
    do {                                                                                    \
        if (debug_level >= DEBUG_LEVEL_NOTICE) {                                            \
            printf("["YELLOW"NOTICE"RESET"]""[%s][%d]"fmt, __FUNCTION__, __LINE__, ##args); \
        }                                                                                   \
    } while (0)                

#define debug(fmt,args...)                                                                  \
    do {                                                                                    \
        if (debug_level >= DEBUG_LEVEL_DEBUG) {                                             \
            printf("["GREEN"DEBUG"RESET"]""[%s][%d]"fmt, __FUNCTION__, __LINE__, ##args);   \
        }                                                                                   \
    }while (0)                

#endif  /* _DEBUG_H_ */
