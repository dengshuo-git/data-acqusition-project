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
#include <sys/ioctl.h>

#define __USE_GNU

#include <sched.h>
#include <pthread.h>

#include "libdaq.h"

#define NUM_WORKER_THREADS  1


typedef void* (*jobthdfn)(void* arg);

struct saw_thdcb{
      pthread_t   tid;
      pthread_mutex_t  mutex;//use to protect fdMapSet
      jobthdfn	jobthd;//线程函数
};

pthread_barrier_t p_barrier_start;
pthread_barrier_t p_barrier_end;
pthread_barrier_t p_barrier_fun_finish;

struct saw_thdcb thdcbs_func;

#define DATA_ACQUSITION_DEVICE  "/dev/data_acquisition_card"

void* data_acquisition_test(void* arg);

void *data_acquisition_test(void *arg)
{
	struct epoll_event ev_globalfifo;
	int err;
	int epfd;
    int fd;
    uint8_t recv_buf[4096] = {0};
    int recv_count = 0;
    int ret;
    int cmd;

    fd = Tspi_OpenDevice();
	if(fd < 0){		
		return 0;
	}

	epfd = epoll_create(1);
	if (epfd < 0) {
		perror("epoll_create()");
		return NULL;
	}

	bzero(&ev_globalfifo, sizeof(struct epoll_event));
	ev_globalfifo.events = EPOLLIN | EPOLLPRI;

	err = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev_globalfifo);
	if (err < 0) {
		perror("epoll_ctl()");
		return NULL;
	}

    while(1){

		err = epoll_wait(epfd, &ev_globalfifo, 1, 20000);
		if (err < 0) {
			perror("epoll_wait()");
		}else if (err == 0) {
			printf("recv_count = %d\n",recv_count);
		}else {

        	ret = read(fd, recv_buf, 4096);
        	if(ret < 0){
            	printf("recv ret = %x\n",ret);
        	}	   
		
        	recv_count++;
			printf("recv_count = %d\n",recv_count);
	 	}
	}

    return NULL;
}


int main(int argc,char *argv[]) 
{
    int userdata = 0;
	int i;
    int fd;
    int cmd;
    int ret;

    int rate = 102400;
    int ch_cnt = 8;
    int ch_en = 0xffff;
    int packet_type = 1;

    uint8_t buf[36] ={0};

    /* 用于性能测试 */
    if(pthread_barrier_init(&p_barrier_start, NULL, 2 * NUM_WORKER_THREADS + 1) != 0){
        fprintf(stderr, "pthread_barrier_init failed\n");
        return -1;
    }

    if(pthread_barrier_init(&p_barrier_end, NULL, 2 * NUM_WORKER_THREADS + 1) != 0){
        fprintf(stderr, "pthread_barrier_init failed\n");
        return -1;
    }

    /* 用于功能测试 */
    if(pthread_barrier_init(&p_barrier_fun_finish, NULL, 2) != 0){ 
        fprintf(stderr, "pthread_barrier_init failed\n");
        return -1;
    }

    fd = Tspi_OpenDevice();
	if(fd < 0){		
        printf("open failed!\n");
		return 0;
    }

    /* 创建数据采集线程，用于功能测试 */
    pthread_mutex_init(&thdcbs_func.mutex, NULL);
    thdcbs_func.jobthd = data_acquisition_test;;

    pthread_create(&thdcbs_func.tid, NULL, data_acquisition_test, NULL);

    while(1){

	    printf("*************** Symm Alg Test **************\n\n");
	    printf("a. Start Data Acquisitionn Test \n");
	    printf("b. Stop Data Acquisitionn Test \n");
	    printf("c. Set Daq Parameter Test \n");
	    printf("q. Quit\n\n");
				
	    printf("\nPlease enter your choice: ");		
        userdata = 0;
        while(userdata == 0)
	    {   	
		    userdata = getchar();
            if(userdata == '\n')
		    {
			    userdata = 0;
			    continue;
		    }
	    }

	    switch(userdata)
	    {
		    case 'a':
                start_daq(fd);                 
			    break;

		    case 'b':
                stop_daq(fd);
			    break;

		    case 'c':
	            printf("\nPlease input rate: ");		
                scanf("%d", &rate);
	            printf("\nPlease input ch_cnt: ");		
                scanf("%d", &ch_cnt);
                set_daq_paramter(fd, packet_type, rate, ch_en, ch_cnt);
			    break;

		    case 'q':
                Tspi_CloseDevice(fd);
			    exit(0);			
			    break;
						
		    default:	
			    break;
	    }
    }

    return 0;
}



