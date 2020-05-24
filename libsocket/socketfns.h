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
#ifndef _UDPFNS_H
#define _UDPFNS_H
/// 引用非标准库的头文件
/// 告诉编译器为C编译
#if defined(__cplusplus)
extern "C" {
#endif
#include <sys/socket.h>
/** @defgroup udp udp类 */
/*******************************************udp类********************************************************/
/** @{ */
/**
*@brief 初始化UDP套接字函数带TTL
*@param	addr  该套接字要绑定的IP地址
*@param	port  端口
*@param	ttl   TTL值
*@return	0 :正常     -1:异常
*/
extern int UDPInitial(char *addr, int port, unsigned char ttl);
/**
*@brief 初始化UDP套接字函数
*@param	addr  该套接字要绑定的IP地址
*@param	port  端口
*@return	0 :正常     -1:异常
*/
extern int UDPInitial_old(char *addr, int port);

/**
*@brief 发送UDP数据
*@param	sockfd  绑定了目的主机IP和端口的套接字
*@param	buf     发送缓冲区
*@param	buflen  要发送的长度
*@param	tout    发送超时时间
*@return	0 :正常     -1:异常
*/

extern int SendUDPData(int sockfd, void *buf, int buflen, int tout);

/**
*@brief 发送UDP数据
*@param	sockfd  绑定了目的主机IP和端口的套接字
*@param	saddr   该套接字要绑定的IP地址
*@param	port    端口
*@param	s       发送缓冲区
*@param	slen    要发送的长度
*@return	0 :正常     -1:异常
*/
extern int udp_send_process(int sockfd, char *saddr, unsigned short port, char *s, unsigned short slen);

/**
*@brief 接收UDP数据
*@param	sockfd    绑定了目的主机IP和端口的套接字
*@param	buf       发送缓冲区
*@param	buflen    要发送的长度
*@param	addr      保存数据源主机网络信息的结构体指针
*@param	addr_len  保存数据源主机网络信息的结构体长度
*@param	tout      接收超时时间
*@return	0 :正常     -1:异常
*/
extern int ReceiveUDPData(int sockfd, void *buf, int buflen, struct sockaddr *addr, int *addr_len, int tout);

/**
*@brief 连接远程主机
*@param	fd      初始化的UDP套接字
*@param	toadr   远程主机IP地址
*@param	port    远程主机监听端口
*@param	tout    接收超时时间
*@return	0 :正常     -1:异常
*/

extern int UDPConnect(int fd, char *toadr, unsigned short port, int tout);

/**
*@brief 关闭UDP套接字
*@param	udpfd   UDP套接字
*@return	0 :正常     -1:异常
*/
extern int CleanupUDPChannel(int udpfd);

/**
*@brief UDP是否有数据变化
*@param	dsfd    初始化的UDP套接字
*@param	rorw    读写标识 0为写 1为读
*@param	tout    接收超时时间
*@return	0 :正常     -1:异常
*/

extern int IsUDPRWable(int dsfd, int rorw, int tout);


/**
*@brief 组播UDP指定组播源套接字函数（本地单网卡绑定）
*@param	addr  该套接字要绑定的IP地址
*@param	maddr 组播地址
*@param	saddr 一组播源（主）
*@param	baddr 一组播源（备）
*@param	taddr 二组播源（主）
*@param	caddr 二组播源（备）
*@param	ifsrc 0时表示指定源
*@param	port  端口
*@return	0 :正常     -1:异常
*/
extern int MultiSrcInitial_old(char *addr, char *maddr, char *saddr, char *baddr, char *taddr, char *caddr, unsigned char ifsrc, int port);
/**
*@brief 组播UDP指定组播源套接字函数（本地双网卡绑定）
*@param	ezaddr  该套接字要绑定的网卡1的IP地址
*@param	eoaddr  该套接字要绑定的网卡2的IP地址
*@param	maddr   组播地址
*@param	saddr   一组播源（主）
*@param	baddr   一组播源（备）
*@param	taddr   二组播源（主）
*@param	caddr   二组播源（备）
*@param	ifsrc   0时表示指定源
*@param	port    端口
*@return	0 :正常     -1:异常
*/
extern int MultiSrcInitial(char *ezaddr, char *eoaddr, char *maddr, char *saddr, char *baddr, char *taddr, char *caddr, unsigned char ifsrc, int port);

/**
 * add_fd_epollset      添加文件描述符fd到epoll描述符epfd进行监听 
 * @epfd:               epoll描述符
 * @fd:                 待监听的文件描述符fd
 * @epoll_ev            监听的事件
 *
 * @return              0:成功， -1:异常
 */
extern int add_fd_epollset(int epfd,  int fd,unsigned int epoll_ev);

/**
 * exec_shell_cmd   执行字符串描述的shell命令 
 * @argv            字符串，用于描述shell命令
 * @return          0:成功， -1:异常
 */
extern int exec_shell_cmd(const char **argv);


/**
 * create_listen_socket 创建TCP套接字
 * @listen_addr         绑定的Ip地址 
 * @listen_port         绑定的端口 
 * @return              0:成功， -1:异常
 */
extern int create_listen_socket(int listen_addr, short listen_port);

/**
 * accept_new_connect   监听tcp链接，返回连接fd 
 * @listen_fd           监听fd套接字 
 * @return              0:成功， -1:异常
 */

extern int accept_new_connect(int listenfd);

extern int tcp_read(int socket_fd, uint8_t *recv_buf, int *flags, int epfd);

extern void tcp_write(int socket, uint8_t *send_buf, int data_len, int epfd);

extern void handle_close(int socket_fd, int epfd);

/** @} */
#if defined(__cplusplus)
}
#endif///< end defined(__cplusplus)
#endif
