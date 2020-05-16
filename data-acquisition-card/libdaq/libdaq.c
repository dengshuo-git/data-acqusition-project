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
#include <sys/syscall.h>

#include "protocal.h"
#include "libdaq.h"

uint32_t  packet_len = 0;
uint32_t  packet_code = 0;
uint32_t  packet_type = 0;

uint32_t  daq_rate = 0; 

uint32_t  channel_enable = 0;
uint16_t  channel_cnt = 0;
uint16_t  point_cnt = 0;

int Tspi_OpenDevice(void)
{  
	int dev_fd;
    int i;
    
    dev_fd = open("/dev/data_acquisition_card", O_RDWR);
    if(dev_fd < 0){		
        return -1;
    }

	return dev_fd;
}

int32_t Tspi_CloseDevice(int dev_fd) 
{
    close(dev_fd);
	return 0;
}


/**********************************
功能：开始采集
dev_fd          设备描述符

返回值：0 成功    非0 失败
***********************************/
int start_daq(int dev_fd)
{
    int ret;
    int cmd = START_DAQ;
    ret = write(dev_fd, &cmd, 0);
    if(ret != 0) ret = -1;
    return ret;
}

/**********************************
功能：停止采集
dev_fd          设备描述符

返回值：0 成功    非0 失败
***********************************/
int stop_daq(int dev_fd)
{
    int ret;
    int cmd = STOP_DAQ;
    ret = write(dev_fd, &cmd, 0);
    if(ret != 0) ret = -1;
    return ret;
}

int fill_packet_header(int dev_fd, uint8_t *data)
{
    time_t timer;
    struct tm *local;

    static int packet_no = 0;

    struct Data_Packet_t * packet = (struct Data_Packet_t *)packet;

    time(&timer);
    local = localtime(&timer);

    packet->packet_len  = packet_len;
	packet->packet_code = packet_no;    
    packet->packet_type = packet_type;
    packet->rate        = daq_rate ;
    packet->year        = local->tm_year + 1900;
    packet->month       = local->tm_mon + 1;
    packet->day         = local->tm_mday;
    packet->hour        = local->tm_hour;;
    packet->minitue     = local->tm_min;
    packet->m_sec       = 0;
    packet->channel_enable = channel_enable;
    packet->channel_num = channel_cnt;
    packet->point_num   = point_cnt;

    packet_no++;
    return 0;
}

int set_daq_paramter(int dev_fd, uint32_t type, uint32_t rate, uint32_t ch_en, int ch_num)
{
    uint8_t send[32] = {0};
    struct Set_Daq_Paramter_t * set_para = (struct Set_Daq_Paramter_t *)send;

    packet_type     =   type;
    daq_rate        =   rate;

    channel_enable  =   ch_en;
    channel_cnt     =   ch_num; 
    point_cnt       =   (QUEUE_NODE_SIZE - sizeof(struct Data_Packet_t)) / ch_num / 2;// 数据包最大4K (4K - 32（header)) / 通道数 / 2（每个采样点2个字节)

    packet_len      =   sizeof(struct Data_Packet_t) + point_cnt * 2 * ch_num; 

    set_para->cmd       =   SET_PARAMTER;
    set_para->daq_rate  =   rate;
    set_para->ch_cnt    =   ch_num;
    set_para->pt_cnt    =   point_cnt;

    printf("packet_type     = %d\n", packet_type);
    printf("daq_reate       = %d\n", daq_rate);
    printf("channel_enable  = %x\n", channel_enable);
    printf("channel_cnt     = %d\n", channel_cnt);
    printf("point_cnt       = %d\n", point_cnt);
    printf("packet_len      = %d\n", packet_len);
        
    write(dev_fd, send, 16);

    return 0;
}

