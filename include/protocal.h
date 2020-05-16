#ifndef _PROTOCAL_H_
#define _PROTOCAL_H_
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

#define TAG                 0x1234

#define QUEUE_DEPTH         0x8
#define QUEUE_NODE_SIZE     0x1000

#define MAX_MSG_LEN         0x1000
#define MSG_HEAD_LEN        12 

#define     START_DAQ               0x0101
#define     STOP_DAQ                0x0102
#define     REBOOT                  0x0103
#define     SET_TIME                0x0104
#define     HEARTBEAT               0x0106
#define     WRITE_LOG               0x0107
#define     SET_TCP_PORT            0x0108
#define     SET_PARAMTER            0x0109

#define     CONFIG_TCP_PORT             0x1
#define     DAQ_TCP_PORT                0x2


#define     SYSTEM_QUIT                 0
#define     SET_SYSTEM_PARAMTER         1
#define     SET_SIMULATION_SCRIPT       2
#define     QUERY_LOG                   3
#define     SET_NETWORK                 4

#define     LOOP_ADDRESS         "127.0.0.1"

struct Data_Packet_t{
    uint16_t packet_len;
    uint16_t packet_type;
	uint32_t packet_code;;        
    uint32_t rate;
    uint64_t channel_enable;
    uint16_t year;
    uint8_t  month;
    uint8_t  day;
    uint8_t  hour;
    uint8_t  minitue;
    uint16_t m_sec;
    uint16_t channel_num;
    uint16_t point_num;
    uint8_t  data[0];
}__attribute__((packed));  //数据包头 32字节

struct saw_fd{

    int         socket_fd;
    int         epfd;
	int         flags;//SFLAG_RD_PART or SFLAG_WR_PART      

	int         rdStartByte;
	int         bytes2rd;

	int         bytes2wr;
	int         wrStartByte;
};

struct Header_t{
    uint32_t tag;
    uint32_t frame_len;
    uint32_t cmd;
};

struct Result_t{
    uint32_t tag;
    uint32_t frame_len;
    uint32_t cmd;
    uint32_t result;
};

struct Start_Daq_t{
    struct Header_t header;
    uint32_t daq_rate;
    uint32_t ch_cnt;
    uint32_t ch_en;
    uint32_t packet_type;
};

struct Start_Daq_Result_t{
    struct Result_t result; 
};

struct Stop_Daq_t{
    struct Header_t header;
};

struct Stop_Daq_Result_t{
    struct Result_t result; 
};

struct Reboot_t{
    struct Header_t header;
};

struct Reboot_Result_t{
    struct Result_t result; 
};

struct Set_Time_t{
    struct Header_t header;
    uint32_t year;
    uint32_t month;
    uint32_t day;
    uint32_t hour;
    uint32_t miniute;
    uint32_t second;
};

struct Set_Time_Result_t{
    struct Result_t result; 
};

struct Set_Ip_t{
    struct Header_t header;
    uint32_t ip;
};

struct Set_Ip_Result_t{
    struct Result_t result; 
};

struct Heart_Beat_t{
    struct Header_t header;
    uint32_t ask;
};

struct Heart_Result_t{
    struct Result_t result; 
    uint32_t answer;
};


struct Write_Log_t{
    struct Header_t header;
    uint32_t log_len;
    uint8_t log[0];
};

struct Write_Log_Result_t{
    struct Result_t result;
};

struct Set_Tcp_Port_t{
    struct Header_t header;
    uint32_t port_type;
    uint32_t port;
};

struct Set_Tcp_Port_Result_t{
    struct Result_t result;
};

struct Set_Daq_Paramter_t{ 
    uint32_t cmd;
    uint32_t daq_rate;
    uint32_t ch_cnt;
    uint32_t pt_cnt;
};
#endif
