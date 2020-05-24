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

#include "port.h"
#include "protocal.h"

int write_log(char *log, int log_len)
{
    int udpfd; 
	struct sockaddr_in addr;
    uint8_t data[4096] = {0};
    struct Write_Log_t *write_log = (struct Write_Log_t *)data;

    udpfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpfd == -1)
    {
        printf("Init UDP socket for Log error(%d): %s \n", errno, strerror(errno));
        return 0;
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(LOOP_ADDRESS); 
    addr.sin_port = htons(LOG_PORT);

    write_log->header.tag = TAG;
    write_log->header.frame_len = sizeof(struct Write_Log_t) + log_len;
    write_log->header.cmd = WRITE_LOG ;
    write_log->log_len = log_len;
    memcpy(write_log->log, log, log_len);

    sendto(udpfd, data, write_log->header.frame_len, 0, (struct sockaddr*)&addr, sizeof(addr));

    close(udpfd);

    return 0;

}
