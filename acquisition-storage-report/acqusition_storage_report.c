#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>
#include <sys/epoll.h>
#include <strings.h>
#include <signal.h>

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
#include <stddef.h>


#define __USE_GNU

#include <sched.h>
#include <pthread.h>

#include "protocal.h"
#include "port.h"
#include "minIni.h"
#include "socketfns.h"
#include "libdaq.h"


sem_t sem_full;
sem_t sem_empty;

int storage_flag = 0; // 0: 采集数据不存储  1：采集数据存储在数据库中
int daq_socket_fd = 0; 

uint8_t daq_data[QUEUE_DEPTH][4096] = {0};

/****************************************************************/
static int handle_msg(uint8_t *in_buf, uint8_t *out_buf);


static void *acqusition_data_thread(void *arg)
{
    int ret;
    int queue_head = 0;
    uint8_t *data = NULL;

	struct epoll_event ev;
	int err;
	int epfd;
    
	int cpu_no = 1; 
	cpu_set_t mask;

    int card_fd;
    int fd;
    int nbytes = 0;

    card_fd = Tspi_OpenDevice();
    if(card_fd < 0){
		printf("Open Device failed!\n");
        return NULL;
    }
    printf("card_fd = %d\n",card_fd);

	CPU_ZERO(&mask);
	CPU_SET(cpu_no,&mask); 

	if(sched_setaffinity(0, sizeof(mask), &mask) == -1){
		printf("could not set CPU affinity\n"); 
		return 0;
	}

	epfd = epoll_create(10);
	if(epfd < 0){
		printf("create epoll fd error!\n");
		return NULL;
	}      

	bzero(&ev, sizeof(struct epoll_event));
	ev.events = EPOLLIN | EPOLLPRI;
    ev.data.fd = card_fd;

	err = epoll_ctl(epfd, EPOLL_CTL_ADD, card_fd, &ev);
	if (err < 0) {
		perror("epoll_ctl()");
        goto out1;
	}

	while( 1 ) {

		ret = epoll_wait(epfd, &ev, 1, -1);
		if(ret > 0){
			fd = ev.data.fd;
			if((ev.events == EPOLLIN) && (fd == card_fd)){

	            sem_wait(&sem_full);

                data = daq_data[queue_head]; 
                
                nbytes = read(fd, data, 4096);
                if(nbytes < 0){
                    continue;
                }	   

                printf("recv a daq packet!\n");
                
            //    fill_packet_header(card_fd, data);
                
                queue_head = (queue_head + 1) % QUEUE_DEPTH;

	            sem_post(&sem_empty);

	        }
        }
    }

out:
    err = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev);
    if (err < 0)
        perror("epoll_ctl()");
out1:
    close(epfd);
    Tspi_CloseDevice(card_fd);
	return 0;
}

static void *data_deal_thread(void *arg)
{
	int nbytes_write = 0;
	int cpu_no = 2; 
	cpu_set_t mask;
    int queue_tail = 0;
    uint8_t *data = NULL;
    int packet_len = 0;

    struct Data_Packet_t *packet = NULL;


	CPU_ZERO(&mask);
	CPU_SET(cpu_no,&mask); 

	if(sched_setaffinity(0,sizeof(mask),&mask) == -1){
		printf("could not set CPU affinity\n"); 
		return NULL;
	}

    /*
     * 将采集数据发送至上位机
     */

    while( 1 ){

        sem_wait(&sem_empty);

        data = daq_data[queue_tail]; 

        packet = (struct Data_Packet_t *)data;
        
        packet_len = packet->packet_len;

        if(packet_len > MAX_MSG_LEN) continue; 

#if 0        
        while(1){

            nbytes_write = send(daq_socket_fd, data, packet_len, 0);
            if(nbytes_write < 0){

                /*
                 * 记录日志
                 */

                break;

            }
            
            packet_len -= nbytes_write; 
            data = data + nbytes_write;

            if(packet_len == 0){
                break;
            }
        }
#endif        
        
        printf("send a daq data via net\n");

        queue_tail = (queue_tail + 1) % QUEUE_DEPTH;

        sem_post(&sem_full);

    }

out:
    return 0;
}

static int init_service(void)
{
	pthread_t ntid;

    /* 创建数据采集线程 */
	if(pthread_create(&ntid, NULL, (void *)acqusition_data_thread, NULL)){
		printf("pthread create thread error.\n");
		return -1;
	}
	pthread_detach(ntid);

    /* 创建数据处理(上传、存储)线程 */
	if(pthread_create(&ntid, NULL, (void *)data_deal_thread, NULL)){
		printf("pthread create thread error.\n");
		return -1;
	}
	pthread_detach(ntid);

	return 0;
}

int main(void)
{
    int udpfd;
    int ret;

    char str[100];
    long n;
    int s, k;
    char section[50];

	struct epoll_event ev;
	int err;
	int epfd;

    uint8_t recv_buf[4096];
    uint8_t send_buf[4096];

    int addr_len = 0, nread = 0;
    struct sockaddr_in addr;
    
	int cpu_no = 0; 
	cpu_set_t mask;
    int listen_fd;

    int tmp_fd = 0;
    int newfd = 0;

	CPU_ZERO(&mask);
	CPU_SET(cpu_no,&mask); 

	if(sched_setaffinity(0, sizeof(mask), &mask) == -1){
		printf("could not set CPU affinity\n"); 
		return 0;
	}

    int listen_port = 20002;

    /*
     * 解析配置文件
     */
    const char config_file[] = "/var/daq/config.ini";
    listen_port = ini_getl("General", "DAQ_TCP_PORT", -1, config_file);
    printf("listen_port = %d\n", listen_port);

	/* 初始化工作线程 */ 
	ret = init_service();
	if(ret < 0){
		return -1;
	}

    
    /* 初始化信号里 */
	sem_init(&sem_full, 0, QUEUE_DEPTH);
	sem_init(&sem_empty, 0, 0);


    /*
     * 监听UDP套接字
     */
    udpfd = UDPInitial_old((char *)LOOP_ADDRESS, DATA_ACQUSITION_PORT);
    if (udpfd == -1){
        printf("Init UDP socket for Log error(%d): %s \n", errno, strerror(errno));
        return 0;
    }

    /*
     * 监听TCP套接字，与上位机建立连接，用于上传采集数据
     */
	listen_fd = create_listen_socket(INADDR_ANY, listen_port);
	if(listen_fd < 0){
		fprintf(stderr, "create listen socket on port %d error\n", listen_port);
		return  0;
	}else{
		struct in_addr addr;
		addr.s_addr = htonl(INADDR_ANY);
		printf("Create listening-socket(fd = %d) on %s:%d success\n", listen_fd, inet_ntoa(addr), listen_port);
	}

	epfd = epoll_create(10);
	if(epfd < 0){
		perror("create epoll fd error");
		return 0;
	}

	ret = add_fd_epollset(epfd, listen_fd, EPOLLIN);
    if(ret < 0){
        close(listen_fd);
        return 0;
    }

	ret = add_fd_epollset(epfd, udpfd, EPOLLIN);
    if(ret < 0){
        close(udpfd);
        return 0;
    }

    while( 1 ) {
        err = epoll_wait(epfd, &ev, 1, 60000);
        if (err < 0) {
            perror("epoll_wait()");
            goto out;

        }else if (err == 0) {
            printf("No data input in FIFO within 60 seconds.\n");

        }else {
            tmp_fd = ev.data.fd;
            if(ev.events != EPOLLIN ){
                printf("unknown epoll event(%x) happened on listen_fd(%d)", ev.events, tmp_fd);
                continue;;              
            }
            if(tmp_fd == listen_fd){
                printf("recv clinet connect!\n");
                newfd = accept_new_connect(listen_fd);
                printf("newfd = %d\n", newfd);
                if(newfd > 0){
                    printf("recv clinet connect!\n");
                    daq_socket_fd = newfd;
                    add_fd_epollset(epfd, newfd, EPOLLIN);
                }

            }else if(udpfd == tmp_fd){
#if 0
                nread = ReceiveUDPData(udpfd, recv_buf, MAX_MSG_LEN, (struct sockaddr *)&addr, &addr_len, 0);
                if (nread <= 0) {
                    printf("Recvice Data Error!\n");
                    continue;
                }

                if ((nread < MSG_HEAD_LEN) && (nread > MAX_MSG_LEN)) {
                    continue;
                }

                handle_msg(recv_buf, send_buf);
                struct Result_t *result = (struct Result_t *)send_buf;
                sendto(udpfd, (void *)&send_buf, result->frame_len, 0, (struct sockaddr *)&addr, addr_len);
#endif                

            }else if(newfd == tmp_fd){ //只处理上位机建联操作

                /*
                 * 从上位机接受命令
                 */

               // tcp_read(&fdmap, recv_buf);

                handle_msg(recv_buf, send_buf);

                /*
                 * 返回处理结果
                 */

    //            tcp_write(&fdmap, send_buf);


            }else{
                 
                printf("unknown epoll event(%x) happened on fd(%d), the fd is not listen by epoll", ev.events, tmp_fd);
                continue;

            }


        }
    }

out:
	err = epoll_ctl(epfd, EPOLL_CTL_DEL, udpfd, &ev);
	if (err < 0){
		perror("epoll_ctl()");
    }

	err = epoll_ctl(epfd, EPOLL_CTL_DEL, listen_fd, &ev);
	if (err < 0){
		perror("epoll_ctl()");
    }

    close(listen_fd);
    CleanupUDPChannel(udpfd);

    close(epfd);

    return 0;
}


int handle_msg(uint8_t *in_buf, uint8_t *out_buf)
{
    struct Header_t *header = (struct Header_t *)in_buf;
    struct Result_t *result = (struct Result_t *)out_buf;
    int ret;
    int fd;

    switch(header->cmd){

        default:

            ret = 1;
            break;
    }

    result->tag = header->tag;
    result->frame_len = 16; 
    result->cmd = header->cmd;
    result->result = ret; 

    
    return 0;
}
