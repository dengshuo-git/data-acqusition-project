/*************************************************
 *
 *  Copyright (C), 2013, CyberXingan
 *
 *  @file udpfns.c
 *  @author dengshuo 
 *  @brief socket通信接口
 *
 *  History:
 *  <sn>  <mofifier>     <date>         <Description>
 *   1    dengshuo       2020-04-25     socet comutication library 
 *
 ************************************************/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include <sys/errno.h>

#include "socketfns.h"
#define IP_ADD_SOURCE_MEMBERSHIP    39
#define IP_DROP_SOURCE_MEMBERSHIP   40
#define MAXSEG                      1000

struct ip_mreq_source
{
    /* IP multicast address of group.  */
    struct in_addr imr_multiaddr;

    /* IP address of source.  */
    struct in_addr imr_interface;

    /* IP address of interface.  */
    struct in_addr imr_sourceaddr;
};



/**
 *@brief 信号接收函数
 *@param	pid  信号
 *@return	void
 */
static void sig_funcs(int pid)
{
    printf("Catch signal \n");
    return; 
}

/**
 *@brief 初始化UDP套接字函数
 *@param	addr  该套接字要绑定的IP地址
 *@param	port  端口
 *@return	0 :正常     -1:异常
 */
int UDPInitial_old(char *addr, int port)
{
    int ret = 0;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    int value = 1;
    struct sockaddr_in svraddr;

    if(sockfd < 0)
    {
        ret = -1;
        goto err;
    }

    bzero(&svraddr, sizeof(svraddr));

    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(port);

    if(addr == NULL)
        svraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    else
        svraddr.sin_addr.s_addr = inet_addr(addr);


    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&value, sizeof(value)) < 0) 
    {
        ret = -1;
        goto err;
    }

    if(bind(sockfd, (struct sockaddr*)&svraddr, sizeof(svraddr)) < 0)
    {
        ret = -1;
        goto err;
    }

    return sockfd;

err:
    if(sockfd >= 0)
        close(sockfd);
    return ret;
}

/**
 *@brief 初始化UDP套接字函数带TTL
 *@param	addr  该套接字要绑定的IP地址
 *@param	port  端口
 *@param	ttl   TTL值
 *@return	0 :正常     -1:异常
 */
int UDPInitial(char *addr, int port, unsigned char ttl)
{
    int ret = 0;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    int value = 1;
    struct sockaddr_in svraddr;

    if(sockfd < 0)
    {
        ret = -1;
        goto err;
    }

    bzero(&svraddr, sizeof(svraddr));

    svraddr.sin_family = AF_INET;
    svraddr.sin_port = htons(port);

    if(addr == NULL)
        svraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    else
        svraddr.sin_addr.s_addr = inet_addr(addr);


    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&value, sizeof(value)) < 0) 
    {
        ret = -1;
        goto err;
    }

    if (ttl>0)
    {
        if (setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0) 
        {
            ret = -1;
            goto err;
        }
    }
    if(bind(sockfd, (struct sockaddr*)&svraddr, sizeof(svraddr)) < 0)
    {
        ret = -1;
        goto err;
    }

    return sockfd;

err:
    if(sockfd >= 0)
        close(sockfd);
    return ret;
}

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
int MultiSrcInitial_old(char *addr, char *maddr, char *saddr, char *baddr, char *taddr, char *caddr, unsigned char ifsrc, int port)
{
    int ret = 0;
    int sockfd = -1;
    struct sockaddr_in peeraddr;
    struct ip_mreq_source mreq;

    if (!maddr)
        return -1;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0)
        return -1;

    bzero(&mreq, sizeof(mreq));

    if (inet_aton(maddr, &mreq.imr_multiaddr) < 0)
    {
        printf("inet_aton() error \n");
        ret = -1;
        goto err;
    }

    if (!addr)
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);    // 可指定空地址
    else
        mreq.imr_interface.s_addr = inet_addr(addr);

    // ifsrc = 0时表示指定源
    if (!ifsrc && saddr && baddr && taddr && caddr)
    {
        // 把一平面主KDE地址加入组播源地址
        mreq.imr_sourceaddr.s_addr = inet_addr(saddr);
        if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP, &mreq, sizeof(mreq)) == -1)
        {
            printf("setsockopt() error(%d): %s \n", errno, strerror(errno));
            ret = -1;
            goto err;
        }

        // 把一平面备KDE地址加入组播源地址
        if (strcmp(saddr, baddr) != 0)
        {
            mreq.imr_sourceaddr.s_addr = inet_addr(baddr);
            if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP, &mreq, sizeof(mreq)) == -1)
            {
                printf("setsockopt() error(%d): %s \n", errno, strerror(errno));
                ret = -1;
                goto err;
            }
        }

        // 把二平面主KDE地址加入组播源地址
        if (strcmp(saddr, taddr) != 0)
        {
            mreq.imr_sourceaddr.s_addr = inet_addr(taddr);
            if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP, &mreq, sizeof(mreq)) == -1)
            {
                printf("setsockopt() error(%d): %s \n", errno, strerror(errno));
                ret = -1;
                goto err;
            }
        }

        // 把二平面备KDE地址加入组播源地址
        if (strcmp(saddr, caddr) != 0)
        {
            mreq.imr_sourceaddr.s_addr = inet_addr(caddr);
            if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP, &mreq, sizeof(mreq)) == -1)
            {
                printf("setsockopt() error(%d): %s \n", errno, strerror(errno));
                ret = -1;
                goto err;
            }
        }
    }
    else
    {
        if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP, &mreq, sizeof(mreq)) == -1)
        {
            printf("setsockopt() error(%d): %s \n", errno, strerror(errno));
            ret = -1;
            goto err;
        }
    }

    memset(&peeraddr, 0, sizeof(struct sockaddr_in));

    peeraddr.sin_family = AF_INET;
    peeraddr.sin_port = htons(port);
    peeraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    /*if (inet_pton(AF_INET, maddr, &peeraddr.sin_addr) <= 0)
      {
      printf("Wrong destination IP address \n");
      ret = -1;
      goto err;
      }*/

    if (bind(sockfd, (struct sockaddr *)&peeraddr, sizeof(struct sockaddr_in)) == -1)
    {
        printf("Bind error \n");
        ret = -1;
        goto err;
    }

    return sockfd;

err:
    if(sockfd >= 0)
        close(sockfd);
    return ret;
}

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
int MultiSrcInitial(char *ezaddr, char *eoaddr, char *maddr, char *saddr, char *baddr, char *taddr, char *caddr, unsigned char ifsrc, int port)
{
    int ret = 0;
    int sockfd = -1;
    struct sockaddr_in peeraddr;
    struct ip_mreq_source mreq;

    if (!maddr)
        return -1;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0)
        return -1;

    bzero(&mreq, sizeof(mreq));

    if (inet_aton(maddr, &mreq.imr_multiaddr) < 0)
    {
        printf("inet_aton() error \n");
        ret = -1;
        goto err;
    }

    if (!ezaddr)
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);    // 可指定空地址
    else
        mreq.imr_interface.s_addr = inet_addr(ezaddr);

    // ifsrc = 0时表示指定源
    if (!ifsrc && saddr && baddr)
    {
        // 把一平面主KDE地址加入组播源地址
        mreq.imr_sourceaddr.s_addr = inet_addr(saddr);
        if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP, &mreq, sizeof(mreq)) == -1)
        {
            printf("setsockopt() error(%d): %s \n", errno, strerror(errno));
            ret = -1;
            goto err;
        }

        // 把一平面备KDE地址加入组播源地址
        if (strcmp(saddr, baddr) != 0)
        {
            mreq.imr_sourceaddr.s_addr = inet_addr(baddr);
            if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP, &mreq, sizeof(mreq)) == -1)
            {
                printf("setsockopt() error(%d): %s \n", errno, strerror(errno));
                ret = -1;
                goto err;
            }
        }
    }
    else
    {
        if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP, &mreq, sizeof(mreq)) == -1)
        {
            printf("setsockopt() error(%d): %s \n", errno, strerror(errno));
            ret = -1;
            goto err;
        }
    }

    if (!eoaddr)
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);    // 可指定空地址
    else
        mreq.imr_interface.s_addr = inet_addr(eoaddr);

    // ifsrc = 0时表示指定源
    if (!ifsrc && taddr && caddr)
    {
        // 把二平面主KDE地址加入组播源地址
        mreq.imr_sourceaddr.s_addr = inet_addr(taddr);
        if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP, &mreq, sizeof(mreq)) == -1)
        {
            printf("setsockopt() error(%d): %s \n", errno, strerror(errno));
            ret = -1;
            goto err;
        }

        // 把二平面备KDE地址加入组播源地址
        if (strcmp(taddr, caddr) != 0)
        {
            mreq.imr_sourceaddr.s_addr = inet_addr(caddr);
            if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP, &mreq, sizeof(mreq)) == -1)
            {
                printf("setsockopt() error(%d): %s \n", errno, strerror(errno));
                ret = -1;
                goto err;
            }
        }
    }
    else
    {
        if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP, &mreq, sizeof(mreq)) == -1)
        {
            printf("setsockopt() error(%d): %s \n", errno, strerror(errno));
            ret = -1;
            goto err;
        }
    }

    memset(&peeraddr, 0, sizeof(struct sockaddr_in));

    peeraddr.sin_family = AF_INET;
    peeraddr.sin_port = htons(port);
    peeraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&peeraddr, sizeof(struct sockaddr_in)) == -1)
    {
        printf("Bind error \n");
        ret = -1;
        goto err;
    }

    return sockfd;

err:
    if(sockfd >= 0)
        close(sockfd);
    return ret;
}

/**
 *@brief 发送UDP数据
 *@param	sockfd  绑定了目的主机IP和端口的套接字
 *@param	buf     发送缓冲区
 *@param	buflen  要发送的长度
 *@param	tout    发送超时时间
 *@return	0 :正常     -1:异常
 */
int SendUDPData(int sockfd, void *buf, int buflen, int tout)
{
    int nret = 0;

    signal(SIGALRM, sig_funcs);
    alarm(tout);
    nret = send(sockfd, buf, buflen, 0);
    alarm(0);

    return nret;
}

/**
 *@brief 发送UDP数据
 *@param	sockfd  绑定了目的主机IP和端口的套接字
 *@param	saddr   该套接字要绑定的IP地址
 *@param	port    端口
 *@param	s       发送缓冲区
 *@param	slen    要发送的长度
 *@return	0 :正常     -1:异常
 */
int udp_send_process(int sockfd, char *saddr, unsigned short port, char *s, unsigned short slen)
{
    struct sockaddr_in sin;

    bzero(&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = inet_addr(saddr);

    return sendto(sockfd, s, slen, 0, (struct sockaddr *)&sin, sizeof(sin));
}

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
int ReceiveUDPData(int sockfd, void *buf, int buflen, struct sockaddr *addr, int *addr_len, int tout)
{
    int nret = 0;

    signal(SIGALRM, sig_funcs);
    alarm(tout);
    nret = recvfrom(sockfd, buf, buflen, 0, addr, (socklen_t *)addr_len);
    alarm(0);

    return nret;
}

/**
 *@brief 连接远程主机
 *@param	fd      初始化的UDP套接字
 *@param	toadr   远程主机IP地址
 *@param	port    远程主机监听端口
 *@param	tout    接收超时时间
 *@return	0 :正常     -1:异常
 */
int UDPConnect(int fd, char *toadr, unsigned short port, int tout)
{
    int nret = 0;
    struct sockaddr_in addr;

    bzero((char*)&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(toadr);   

    signal(SIGALRM, sig_funcs);
    alarm(tout);
    nret = connect(fd, (struct sockaddr *)&addr, sizeof(addr));
    alarm(0);

    return nret;
}

/**
 *@brief UDP是否有数据变化
 *@param	dsfd    初始化的UDP套接字
 *@param	rorw    读写标识 0为写 1为读
 *@param	tout    接收超时时间
 *@return	0 :正常     -1:异常
 */
int IsUDPRWable(int dsfd, int rorw, int tout)
{
    fd_set rset;
    int nret = 0;

    FD_ZERO(&rset);
    FD_SET(dsfd, &rset);

    signal(SIGALRM, sig_funcs);

    alarm(tout);
    nret = rorw?select(dsfd+1, &rset, NULL, NULL, NULL):select(dsfd+1, NULL, &rset, NULL, NULL);
    alarm(0);

    return nret;
}

/**
 *@brief 关闭UDP套接字
 *@param	udpfd   UDP套接字
 *@return	0 :正常     -1:异常
 */
int CleanupUDPChannel(int udpfd)
{
    int ret = 0;

    if (udpfd < 0)
        goto err;

    if(close(udpfd) < 0)
        goto err;

    return ret;

err:
    return -1;
}

/**
 * add_fd_epollset-- 添加文件描述符fd到epoll描述符epfd进行监听 
 * @epfd: epoll描述符
 * @fd:待监听的文件描述符fd
 * @epoll_ev:监听的事件
 *
 * @return：无
 */
int add_fd_epollset(int epfd,  int fd,unsigned int epoll_ev)	
{
	struct epoll_event ev;

	ev.data.fd = fd;
	ev.events = epoll_ev;

	int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
	if(ret < 0 ){
		return -1;
	}
    return 0;
}

/**
 * exec_shell_cmd-- 执行字符串描述的shell命令 
 * @argv-- 字符串，用于描述shell命令
 *
 * @返回值：0 -- 成功， 1 -- 失败
 */
int exec_shell_cmd(const char **argv)
{
    int child;
    int status;

    if((child = fork()) == 0){
        execv(argv[0], (char *const *)argv);
        exit(1);//would never excute to this point except execv() fails
    }

    if(!WIFEXITED(status) || WEXITSTATUS(status)){
        return 1;
    }
    return 0;
}

int create_listen_socket(int listen_addr, short listen_port)
{
	int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);	
	if(fd < 0){
		printf("can not open new socket");
		return -1;
	}

	int one = 1;
	if(setsockopt(fd, IPPROTO_TCP, TCP_NODELAY,(char *)&one, sizeof(one)) < 0){
		printf("set listen_fd(%d) TCP_NODELAY failed", fd);
	}

	int mss = 0;
	socklen_t len = sizeof(mss);
	if(getsockopt(fd, IPPROTO_TCP,TCP_MAXSEG, &mss, &len) < 0){
		printf("get TCP_MAXSEG value failed");
	}

	if(mss > MAXSEG){
		mss = MAXSEG;
		if(setsockopt(fd, IPPROTO_TCP, TCP_MAXSEG, &mss, sizeof(mss)) < 0){
			printf("set TCP_MAXSEG value error");
		}else{
			printf("set TCP_MAXSEG = %d", mss);
		}
	}

	one = 1;
	if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(one)) < 0) {
		printf("cannot do so_reuseaddr");
	}

	struct linger nolinger;
	nolinger.l_onoff = 1;
	nolinger.l_linger = 0;
	setsockopt(fd, SOL_SOCKET, SO_LINGER, (struct linger *)&nolinger, sizeof(struct linger));

	struct sockaddr_in bindaddr;
	bindaddr.sin_family = AF_INET;
	bindaddr.sin_addr.s_addr = htonl(listen_addr);
	bindaddr.sin_port = htons(listen_port);

	if(bind(fd, (struct sockaddr *)&bindaddr, sizeof(bindaddr)) < 0) {
		printf("bind socket fd(%d) on port %u error", fd, listen_port);
		close(fd);
		return -1;
	}

	if(listen(fd, 10) < 0) {
		printf("set socket(%d)to listen error", fd);
		close(fd);
		return -1;
	}

	return fd;
}

int accept_new_connect(int listenfd)
{
	struct sockaddr_in cli;
	socklen_t len = sizeof(cli);

	int newfd = accept(listenfd, (struct sockaddr*)&cli, &len);
	if(newfd < 0){
		printf("accept new connect error");
		return -1;
	}

	struct linger nolinger;
	nolinger.l_onoff = 0;
	nolinger.l_linger = 0;
	setsockopt(newfd, SOL_SOCKET, SO_LINGER, (struct linger *) &nolinger, sizeof(struct linger));

	int lowat = 8;
	setsockopt(newfd, SOL_SOCKET, SO_RCVLOWAT, &lowat, sizeof(lowat));

	struct sockaddr_in local_addr;
	len = sizeof(struct sockaddr);
	getsockname(newfd, (struct sockaddr*)&local_addr, &len);

    printf("newfd = %d\n", newfd);

	return newfd;
}

