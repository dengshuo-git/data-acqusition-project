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

#define     SFLAG_RD_PART       0x0004
#define     SFLAG_WR_PART       0x0040

int cnt = 0;
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


/*
 * 网络接收函数，接受网络端来的报文；
 */
static void tcp_read(struct saw_fd *fdmap,uint8_t *recv_buf)
{
    int ret;
    int socket_fd = fdmap->socket_fd;
    struct Header_t header;
    char *buffer = recv_buf;
	struct epoll_event ev;

    ret = recv(socket_fd , (char *)&header, sizeof(struct Header_t), MSG_PEEK);
    if(ret < 0){
        if((errno == EAGAIN) || (errno == EWOULDBLOCK) ||(errno == EINTR)){
            return;
        }

        printf("Socket(fd = %d) recv error\n", socket_fd);
        handle_recv_close(fdmap);

        return;

    }else if(ret == 0){

        printf("Socket(fd = %d) recv close\n", socket_fd);
        handle_recv_close(fdmap);

        return;
    }else{

        if((size_t)ret < offsetof(struct Header_t, frame_len) + sizeof(header.frame_len)){
            return;
        }

        fdmap->bytes2rd =  header.frame_len;
        if((fdmap->bytes2rd > MAX_MSG_LEN) || (fdmap->bytes2rd < sizeof(struct Header_t))){
            return;
        }

        fdmap->rdStartByte = 0; 
        fdmap->flags |= SFLAG_RD_PART;

    }


//    printf("fdmap->bytes2rd = %d\n", fdmap->bytes2rd);
	while(1){
		ret = recv(socket_fd, buffer + fdmap->rdStartByte, fdmap->bytes2rd, 0);
		if(ret < 0){
			if((errno == EAGAIN) || (errno == EWOULDBLOCK) ||(errno == EINTR)){
				return;
			}

			handle_recv_close(fdmap);

			return;
		}else if(ret == 0){

			if(fdmap->bytes2rd != 0){
				printf("err\n");
				//     handle_recv_close(fdmap);

				/*
				 * 记录日志
				 */

				return;
			}
			else{
				fdmap->flags &= ~SFLAG_RD_PART;
				return;                        
			}

		}else{
			fdmap->rdStartByte += ret;
			fdmap->bytes2rd -= ret;
		}

		if( fdmap->bytes2rd == 0){
			fdmap->flags &= ~SFLAG_RD_PART;
			printf("recv %d cmd packet!\n", cnt);
			cnt++;

			return;      
		}
	}
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

    struct saw_fd fdmap;
    fdmap.flags = 0;

	CPU_ZERO(&mask);
	CPU_SET(cpu_no,&mask); 

	if(sched_setaffinity(0, sizeof(mask), &mask) == -1){
		printf("could not set CPU affinity\n"); 
		return 0;
	}

    int listen_port = 20001;

    /*
     * 解析配置文件
     */
//    const char config_file[] = "/var/daq/config.ini";
 //   listen_port = ini_getl("General", "CONFIG_MANAGEMENT_TCP_PORT", -1, config_file);
  //  printf("listen_port = %d\n", listen_port);
  //


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
                continue;              
            }
            if(tmp_fd == listen_fd){
                printf("recv clinet connect!\n");
                newfd = accept_new_connect(listen_fd);
                printf("newfd = %d\n", newfd);
                if(newfd > 0){
                    printf("add socket epoll_in!\n");
                    fdmap.socket_fd = newfd;
                    add_fd_epollset(epfd, newfd, EPOLLIN);
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

            }else if(newfd == tmp_fd){

				printf("recv a tcp packet\n");

                /*
                 * 从上位机接受命令
                 */
                tcp_read(&fdmap, recv_buf);

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

    struct Set_Tcp_Port_t *set_port = (struct Set_Tcp_Port_t *)in_buf;
    struct Start_Daq_t *start_daq_op = (struct Start_Daq_t *)in_buf;

    int ret;
    int fd;

    const char config_file[] = "/var/daq/config.ini";

    switch(header->cmd){
        case START_DAQ:

            printf("start daq!\n");

            fd = Tspi_OpenDevice();
            if(fd < 0){		
                printf("open failed!\n");
                ret = 1;
                break;
            }

            set_daq_paramter(fd, start_daq_op->packet_type, start_daq_op->daq_rate, start_daq_op->ch_en, start_daq_op->ch_cnt);

            start_daq(fd);                 

            Tspi_CloseDevice(fd);

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
