#include <stdio.h>
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

#include "protocal.h"
#include "port.h"
#include "socketfns.h"


int handle_msg(uint8_t *in_buf, uint8_t *out_buf);

int main(void)
{
    int udpfd;

	struct epoll_event ev;
	int err;
	int epfd;

    uint8_t recv_buf[4096];
    uint8_t send_buf[4096];

    int addr_len = 0, nread = 0;
    struct sockaddr_in addr;

	epfd = epoll_create(10);
	if(epfd < 0){
		printf("create epoll fd error!\n");
		return 0;
	}      
    
    udpfd = UDPInitial_old((char *)LOOP_ADDRESS, LOG_PORT);
    if (udpfd == -1)
    {
        printf("Init UDP socket for Log error(%d): %s \n", errno, strerror(errno));
        return 0;
    }

	bzero(&ev, sizeof(struct epoll_event));
	ev.events = EPOLLIN | EPOLLPRI;

	err = epoll_ctl(epfd, EPOLL_CTL_ADD, udpfd, &ev);
	if (err < 0) {
		perror("epoll_ctl()");
        CleanupUDPChannel(udpfd);
        return 0;
	}

    while( 1 ) {
        err = epoll_wait(epfd, &ev, 1, 60000);
        if (err < 0) {
            perror("epoll_wait()");
        }else if (err == 0) {
        }else {
            nread = ReceiveUDPData(udpfd, recv_buf, MAX_MSG_LEN, (struct sockaddr *)&addr, &addr_len, 0);
            if (nread <= 0) {
                printf("Recvice Log Data Error!\n");
                struct Write_Log_Result_t *write_log_result = (struct Write_Log_Result_t *)send_buf;
                write_log_result->result.tag = TAG;
                write_log_result->result.frame_len = 16;
                write_log_result->result.cmd = WRITE_LOG;
                write_log_result->result.result = 2;
            }

            if ((nread >= MSG_HEAD_LEN) && (nread <= MAX_MSG_LEN)) {
                handle_msg(recv_buf, send_buf);
            }

            sendto(udpfd, (void *)&send_buf, 16, 0, (struct sockaddr *)&addr, addr_len);

        }
    }

out:
	err = epoll_ctl(epfd, EPOLL_CTL_DEL, udpfd, &ev);
	if (err < 0){
		perror("epoll_ctl()");
    }

    CleanupUDPChannel(udpfd);

    return 0;
}

int handle_msg(uint8_t *in_buf, uint8_t *out_buf)
{
    char log_file_path[]="/var/daq/log.txt";

    struct Write_Log_t *write_log_cmd = (struct Write_Log_t *)in_buf;
    struct Write_Log_Result_t *write_log_result = (struct Write_Log_Result_t *)out_buf;

    
	FILE *fp = fopen(log_file_path, "a+");
	if(NULL == fp){
		fprintf(stderr, "can not open state file %s\n", log_file_path);
		return -1;
	}

    fwrite(write_log_cmd->log, sizeof(uint8_t), write_log_cmd->log_len, fp);
    fputs("\n", fp);

	fclose(fp);

    write_log_result->result.tag = TAG;
    write_log_result->result.frame_len = 16;
    write_log_result->result.cmd = WRITE_LOG;
    write_log_result->result.result = 0;

    return 0;
}

