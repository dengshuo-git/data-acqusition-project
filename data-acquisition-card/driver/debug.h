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


/* 颜色设置 */
#define GREEN       "\033[40;32m"
#define YELLOW      "\033[40;33m"
#define PURPLE      "\033[40;35m"
#define RED         "\033[40;31m"
#define RESET       "\033[0m"

#ifdef USER
#define output printf 
#else
#define output printk
#endif


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

//inline int set_debug_level(int level){debug_level = level;}
#define set_debug_level(level) do{debug_level = level;}while(0)

/*
 *  alert   --  用于打印错误信息，不可屏蔽
 *  info    --  用于打印基础信息，不可屏蔽
 *  notice  --  用于打印关键调试信息，可屏蔽
 *  debug   --  用于打印全部调试信息，可屏蔽
 */

#define alert(fmt,args...)                                                                  \
    do {                                                                                    \
        output("["RED"ALERT"RESET"]""[%s][%d]"fmt, __FUNCTION__, __LINE__, ##args);         \
    } while (0)                

#define info(fmt,args...)                                                                   \
    do {                                                                                    \
        output("["PURPLE"INFO"RESET"]""[%s][%d]"fmt, __FUNCTION__, __LINE__, ##args);    \
    } while (0)                

#define notice(fmt,args...)                                                                 \
    do {                                                                                    \
        if (debug_level >= DEBUG_LEVEL_NOTICE) {                                            \
            output("["YELLOW"NOTICE"RESET"]""[%s][%d]"fmt, __FUNCTION__, __LINE__, ##args); \
        }                                                                                   \
    } while (0)                

#define debug(fmt,args...)                                                                  \
    do {                                                                                    \
        if (debug_level >= DEBUG_LEVEL_DEBUG) {                                             \
            output("["GREEN"DEBUG"RESET"]""[%s][%d]"fmt, __FUNCTION__, __LINE__, ##args);   \
        }                                                                                   \
    }while (0)                

#define EnterFunction()	                                                                    \
    do{                                                                                     \
        if (debug_level >= DEBUG_LEVEL_DEBUG) {                                             \
		    output("Enter: %s\n",  __FUNCTION__);		                                    \
        }                                                                                   \
    } while (0)

#define LeaveFunction()                                                                     \
    do {                                                                                    \
        if (debug_level >= DEBUG_LEVEL_DEBUG) {                                             \
			output("Leave: %s\n",     __FUNCTION__);	                                    \
        }                                                                                   \
    } while (0)

#endif  /* _DEBUG_H_ */
