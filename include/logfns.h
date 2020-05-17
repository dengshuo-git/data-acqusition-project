/*************************************************
 *
 *  Copyright (C), 2013, CyberXingan
 *
 *  @file udpfns.h
 *  @author 马阁
 *  @brief UDP通信接口
 *
 *  History:
 *  <sn>  <mofifier>     <date>     <Description>
 *    1			马阁			 2013-06-03		   Create
 *
 ************************************************/
#ifndef _LOGFNS_H
#define _LOGFNS_H
/// 引用非标准库的头文件
/// 告诉编译器为C编译
#if defined(__cplusplus)
extern "C" {
#endif
#include <sys/socket.h>


int write_log(char *log, int log_len);




#if defined(__cplusplus)
}
#endif///< end defined(__cplusplus)
#endif
