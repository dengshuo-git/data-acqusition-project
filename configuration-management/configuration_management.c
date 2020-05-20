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
#include "logfns.h"
#include "debug.h"

#define     SFLAG_RD_PART       0x0004
#define     SFLAG_WR_PART       0x0040

pthread_barrier_t p_barrier;
struct saw_fd fdmap;

/****************************************************************/
static int handle_msg(uint8_t *in_buf, uint8_t *out_buf);

static void handle_recv_close(struct saw_fd *fdmap)
{      
	if(fdmap->epfd > 0){
        struct epoll_event ev;
        ev.data.fd = fdmap->socket_fd;
        epoll_ctl(fdmap->epfd, EPOLL_CTL_DEL, fdmap->socket_fd, &ev);
	}      

	close(fdmap->socket_fd);
	return;
}


static void tcp_write(int socket, uint8_t *send_buf, int data_len)
{
	uint8_t *data = send_buf;
	int packet_len = data_len;
	int nbytes_write = 0;

	while(1){
		nbytes_write = send(socket, data, packet_len, 0);	
		if(nbytes_write < 0){
			if((errno == EAGAIN) || (errno == EWOULDBLOCK) || (errno == EINTR)) continue;

            if(fdmap.epfd > 0){
                struct epoll_event ev;
                ev.data.fd = fdmap.socket_fd;
                epoll_ctl(fdmap.epfd, EPOLL_CTL_DEL, fdmap.socket_fd, &ev);
            }      

            close(fdmap.socket_fd);
            return;
		}

		packet_len = packet_len - nbytes_write;
		data += nbytes_write;

		if(packet_len == 0) break;
	}

	return;

}
/*
 * 网络接收函数，接受网络端来的报文；
 */
static int tcp_read(struct saw_fd *fdmap,uint8_t *recv_buf)
{
    int ret;
    int socket_fd = fdmap->socket_fd;
    struct Header_t header;
    char *buffer = recv_buf;
	struct epoll_event ev;

    ret = recv(socket_fd , (char *)&header, sizeof(struct Header_t), MSG_PEEK);
    if(ret < 0){
        if((errno == EAGAIN) || (errno == EWOULDBLOCK) ||(errno == EINTR)){
            return -1;
        }

        printf("Socket(fd = %d) recv error\n", socket_fd);
        handle_recv_close(fdmap);

        return -1;

    }else if(ret == 0){

        printf("Socket(fd = %d) recv close\n", socket_fd);
        handle_recv_close(fdmap);

        return -1;
    }else{

        if((size_t)ret < offsetof(struct Header_t, frame_len) + sizeof(header.frame_len)){
            return -1;
        }

        fdmap->bytes2rd =  header.frame_len;
        if((fdmap->bytes2rd > MAX_MSG_LEN) || (fdmap->bytes2rd < sizeof(struct Header_t))){
            return -1;
        }

        fdmap->rdStartByte = 0; 
        fdmap->flags |= SFLAG_RD_PART;

    }

	while(1){
		ret = recv(socket_fd, buffer + fdmap->rdStartByte, fdmap->bytes2rd, 0);
		if(ret < 0){
			if((errno == EAGAIN) || (errno == EWOULDBLOCK) ||(errno == EINTR)){
				return -1;
			}

			handle_recv_close(fdmap);

			return -1;
		}else if(ret == 0){

			if(fdmap->bytes2rd != 0){
				printf("err\n");
			    handle_recv_close(fdmap);

				/*
				 * 记录日志
				 */

				return -1;
			}
			else{
				fdmap->flags &= ~SFLAG_RD_PART;
				return 0;                        
			}

		}else{
			fdmap->rdStartByte += ret;
			fdmap->bytes2rd -= ret;
		}

		if( fdmap->bytes2rd == 0){
			fdmap->flags &= ~SFLAG_RD_PART;

			return 0;      
		}
	}
}


void* tcp_handle(void* arg)
{
	struct epoll_event ev;
	int err;
	int epfd;

	int cpu_no = 0;
	cpu_set_t mask;
    int tmp_fd;

    int ret;

    uint8_t recv_buf[4096];
    uint8_t send_buf[4096];

	epfd = epoll_create(10);
	if(epfd < 0){
		perror("create epoll fd error");
		return 0;
	}

    struct Result_t *result;

	CPU_SET(cpu_no,&mask);
	if(sched_setaffinity(0,sizeof(mask),&mask) == -1){
		printf("could not set CPU affinity\n"); 
		return NULL;
	}

	pthread_barrier_wait(&p_barrier);

    fdmap.epfd = epfd;

    while( 1 ) {
        err = epoll_wait(epfd, &ev, 1, 60000);
        if (err < 0) {
            perror("epoll_wait()");
            goto out;

        }else if (err == 0) {
            printf("No data input in FIFO within 60 seconds.\n");

        }else {
            tmp_fd = ev.data.fd;

			if(ev.events  & (EPOLLHUP | EPOLLERR)){
                printf("disconnect!\n");
				/*when net-peer close the socket by signal, 
				  epoll will return both EPOLLIN and (EPOLLHUP|EPOLLERR) */
                handle_recv_close(&fdmap);
                continue;              
            }

            if(fdmap.socket_fd == tmp_fd){

                /* 从上位机接受命令*/
                ret = tcp_read(&fdmap, recv_buf);
				if(ret != 0) continue;

                handle_msg(recv_buf, send_buf);

                result = (struct Result_t *)send_buf;

                /*返回处理结*/
				tcp_write(fdmap.socket_fd, send_buf, result->frame_len);

			}else{
                printf("unknown epoll event(%x) happened on fd(%d), the fd is not listen by epoll", ev.events, tmp_fd);
                continue;
            }
        }
    }
out:

	err = epoll_ctl(epfd, EPOLL_CTL_DEL, fdmap.socket_fd, &ev);
	if (err < 0){
		perror("epoll_ctl()");
    }

    close(fdmap.socket_fd);
    close(epfd);

    return 0;

	return NULL;
}



int main(void)
{
    int udpfd;
    int ret;

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

    fdmap.flags = 0;

    int listen_port = 20001;
	pthread_t ntid;

	CPU_ZERO(&mask);
	CPU_SET(cpu_no,&mask); 

	if(sched_setaffinity(0, sizeof(mask), &mask) == -1){
		printf("could not set CPU affinity\n"); 
		return 0;
	}

	if(pthread_barrier_init(&p_barrier, NULL, 2) != 0){
		perror("pthread_barrier_init failed");
		return 0;
	}

    /* 设置调试级别 */
    set_debug_level(DEBUG_LEVEL_DEBUG);

    /*
     * 解析配置文件
     */
    const char config_file[] = "/var/daq/config.ini";
    listen_port = ini_getl("General", "CONFIG_MANAGEMENT_TCP_PORT", -1, config_file);

    /*
     * 监听UDP套接字
     */
    udpfd = UDPInitial_old((char *)LOOP_ADDRESS, CONFIG_MANAGMENT_PORT);
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

    /*
     * 创建TCP处理线程
     */
	if(pthread_create(&ntid, NULL, (void *)tcp_handle, NULL)){
		printf("pthread create thread error.\n");
		return -1;
	}
	pthread_detach(ntid);

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

	pthread_barrier_wait(&p_barrier);

    while( 1 ) {
        err = epoll_wait(epfd, &ev, 1, 60000);
        if (err < 0) {
            perror("epoll_wait()");
            goto out;

        }else if (err == 0) {
            printf("No data input in FIFO within 60 seconds.\n");

        }else {
            tmp_fd = ev.data.fd;

			if(ev.events  & (EPOLLHUP|EPOLLERR)){
				/*when net-peer close the socket by signal, 
				  epoll will return both EPOLLIN and (EPOLLHUP|EPOLLERR) */
                handle_recv_close(&fdmap);
                continue;              
            }
            if(tmp_fd == listen_fd){
                newfd = accept_new_connect(listen_fd);
                if(newfd > 0){
                    fdmap.socket_fd = newfd;
                    add_fd_epollset(fdmap.epfd, newfd, EPOLLIN);
                }

            }else if(udpfd == tmp_fd){
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

                addr.sin_family = AF_INET;
                addr.sin_addr.s_addr = inet_addr(LOOP_ADDRESS); 
                addr.sin_port = htons(MENU_PORT);

                sendto(udpfd, (void *)&send_buf, result->frame_len, 0, (struct sockaddr *)&addr, addr_len);

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

    struct Set_Tcp_Port_t *set_port = (struct Set_Tcp_Port_t *)in_buf;
    struct Start_Daq_t *start_daq_op = (struct Start_Daq_t *)in_buf;

    struct Heart_Beat_t   *heart_beat = (struct Heart_Beat_t*)in_buf;
    struct Heart_Result_t *heart_ret = (struct Heart_Result_t*)out_buf;

    int ret;
    int fd;

    const char config_file[] = "/var/daq/config.ini";

    switch(header->cmd){
        case START_DAQ:

            fd = Tspi_OpenDevice();
            if(fd < 0){		
                printf("open failed!\n");
                ret = 1;
                break;
            }

            set_daq_paramter(fd, start_daq_op->packet_type, start_daq_op->daq_rate, start_daq_op->ch_en, start_daq_op->ch_cnt);

            start_daq(fd);                 

            Tspi_CloseDevice(fd);

            write_log("start daq",sizeof("start_daq")); 

            debug("启动采集\n");

            ret = 0;
            break;

        case STOP_DAQ:

            fd = Tspi_OpenDevice();
            if(fd < 0){		
                printf("open failed!\n");
                ret = 1;
                break;
            }

            stop_daq(fd);                 

            Tspi_CloseDevice(fd);

            debug("停止采集\n");
            
            ret = 0;
            break;
        
        case SET_TCP_PORT:

            if(set_port->port_type == CONFIG_TCP_PORT){
                ini_putl("General", "CONFIG_MANAGEMENT_TCP_PORT", set_port->port, config_file);
                ret = 0;
            }else if(set_port->port_type == DAQ_TCP_PORT){
                ini_putl("General", "DAQ_TCP_PORT", set_port->port, config_file);
                ret = 0;
            }else{
                ret = 1;
            }

            write_log("set tcp port",sizeof("set tcp port")); 

            break;

        case HEARTBEAT:            
            if(heart_beat->ask = 0x1234){

                debug("接收到心跳请求!\n");

                heart_ret->result.tag = header->tag;
                heart_ret->result.frame_len = sizeof(struct Heart_Result_t); 
                heart_ret->result.cmd = header->cmd;
                heart_ret->result.result = 0; 
                heart_ret->answer = 0x5678;
                return 0;
            }
            ret = 1;

            break;

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
