#include <getopt.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <sstream>
#include <sys/epoll.h>
#include <sys/reboot.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <sys/inotify.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <syslog.h>
#include <termios.h>   
#include <ctype.h> 
#include <sys/wait.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <set>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <sys/types.h>
#include <sys/sysinfo.h>


using namespace std;
#include "protocal.h"
#include "socketfns.h"
#include "port.h"

/*COLOR DEFINITION*/
#define LOG_GREEN "\033[40;32m"
#define LOG_YELLOW "\033[40;33m"
#define LOG_LIGHTBLUE "\033[40;36m"
#define LOG_RED "\033[40;31m"
#define LOG_RESET "\033[0m"

#define LOG_FILE     "/var/daq/log.txt"


static int get_input_value(void);
static void handle_code_port(void);


void display_code_port(void)
{
	cout << endl;
    cout <<"+-----------------------------------+" << endl;
	cout <<"|       请选择配置项目              |" << endl;
    cout <<"+-----------------------------------+" << endl;
	cout <<"|                                   |" << endl;
	cout <<"|       1.配置管理程序端口          |" << endl;
	cout <<"|       2.数据采集程序端口          |" << endl;
	cout <<"|       0.返回上级目录              |" << endl;
    cout <<"+-----------------------------------+" << endl;
	cout << endl;
	cout <<"        请输入您的选择:";
	return;
}

int get_port()
{
	int port;

	while(1){
		cout << endl;
		cout <<"        请输入设置的端口号（10000 ～ 65536）:";
		port = get_input_value();
		cout << endl;
		if(port < 10000|| port > 65536){
			cout << "        端口号输入错误，请重新输入:" << endl;
			continue;
		}else{
			break;
		}
	}
	return port;
}

void config_script(int udpfd, struct sockaddr_in addr)
{
	int num = 0;
	int port = 0;
	char cmd[256] = {0};

    uint8_t recv_buf[4096];
    uint8_t send_buf[4096];

    struct Set_Tcp_Port_t *set_port;
    struct Set_Tcp_Port_Result_t  *set_port_result;

    int nread = 0;
    int addr_len = 0;

	while(1){

		display_code_port();
		num = get_input_value();

		switch(num){
			case 1://配置管理程序TCP端口
				port = get_port();
                set_port = (struct Set_Tcp_Port_t *)send_buf;

                set_port->header.tag        = TAG;
                set_port->header.frame_len  = sizeof(struct Set_Tcp_Port_t);
                set_port->header.cmd        = SET_TCP_PORT;
                set_port->port_type         = CONFIG_TCP_PORT;
                set_port->port              = port; 

                sendto(udpfd, send_buf, set_port->header.frame_len, 0, (struct sockaddr*)&addr, sizeof(addr));

                nread = ReceiveUDPData(udpfd, recv_buf, MAX_MSG_LEN, NULL, NULL, 0);
                if (nread <= 0) {
                    cout << "        设置配置管理程序端口失败" << endl;
                    cout << endl;
                    break;
                }

                set_port_result = (struct Set_Tcp_Port_Result_t *)recv_buf;
                if(set_port_result->result.result == 0){ 
                    cout << "        设置配置管理程序端口成功" << endl;
                    cout << endl;
                }else{
                    cout << "        设置配置管理程序端口失败" << endl;
                    cout << endl;
                }

				break;

			case 2://配置数据采集程序TCP端口

				port = get_port();
                set_port = (struct Set_Tcp_Port_t *)send_buf;

                set_port->header.tag        = TAG;
                set_port->header.frame_len  = sizeof(struct Set_Tcp_Port_t);
                set_port->header.cmd        = SET_TCP_PORT;
                set_port->port_type         = DAQ_TCP_PORT;
                set_port->port              = port; 

                sendto(udpfd, send_buf, set_port->header.frame_len, 0, (struct sockaddr*)&addr, sizeof(addr));

                nread = ReceiveUDPData(udpfd, recv_buf, MAX_MSG_LEN, NULL, NULL, 0);
                if (nread <= 0) {
                    cout << "        设置配置管理程序端口失败" << endl;
                    cout << endl;
                    break;
                }

                set_port_result = (struct Set_Tcp_Port_Result_t *)recv_buf;
                if(set_port_result->result.result == 0){ 
                    cout << "        设置数据采集程序端口成功" << endl;
                    cout << endl;
                }else{
                    cout << "        设置数据采集程序端口失败" << endl;
                    cout << endl;
                }

				break;

			case 0://返回上级菜单

                system("clear");
				goto done;
				break;

			default:

				cout <<"		错误选项，请重新输入："<<endl;
				break;
		}
	}
done:
	return;
}

int get_input_value(void)
{
	int val;
	char input[256] = {0};
	int i = 0;
	int j = 0;

    while(scanf("%s", input) != EOF){

        val = 0;

        for(i = 0; i < strlen(input); i++){
            if(input[i] >= '0' && input[i] <= '9'){
                val = 10 * val + (input[i] - '0');
                j++;
            }
        }


        if(i == j && i!=0 && j!=0){
            return val;
            break;
        }else{
            cout << endl;
            cout <<"        请输入数字!" << endl;
            cout << endl;
            cout <<"        请输入您的选择:";
            cin.clear();
            cin.ignore(1000,'\n');
            j = 0;
            continue;
        }
    }

    return val;
}

void display_root_menu(void)
{
    cout << endl;
    cout <<"+-----------------------------------+" << endl;
    cout <<"|       系统配置管理                |" << endl;
    cout <<"+-----------------------------------+" << endl;
    cout <<"|       1.设置仿真脚本              |" << endl;
    cout <<"|       2.日志查询                  |" << endl;
    cout <<"|       3.启动采集                  |" << endl;
    cout <<"|       4.停止采集                  |" << endl;
    cout <<"|       0.退出程序                  |" << endl;
    cout <<"+-----------------------------------+" << endl;
    cout << endl;
    cout <<"        请输入您的选择:";

    return;
}

int main(int ac,char *av[])
{
    int sel;
    char cmd[256] = {0};

	struct sockaddr_in addr;
    int addr_len = 0, nread = 0;
    int udpfd;

    uint8_t recv_buf[4096];
    uint8_t send_buf[4096];

    struct Header_t *pHeader;
    struct Result_t *pResult;
    struct Start_Daq_t *start_daq_op; 

    uint32_t daq_rate = 0; 
    uint32_t ch_cnt = 0;
    uint32_t ch_en = 0; 
    uint32_t packet_type = 0;

    printf("size = %ld\n",sizeof(struct Data_Packet_t));

    udpfd = UDPInitial_old((char *)LOOP_ADDRESS, MENU_PORT);
    if (udpfd == -1)
    {
        printf("Init UDP socket for Log error(%d): %s \n", errno, strerror(errno));
        return 0;
    }


    while(1){

        display_root_menu();	

        sel = get_input_value();

        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(LOOP_ADDRESS); 
        addr.sin_port = htons(CONFIG_MANAGMENT_PORT);

        switch(sel){

            case 0://退出程序
                exit(1);
                break;

            case 1://设置仿真脚本

                system("clear");
                config_script(udpfd, addr);
                break;

            case 2: //查询日志
                system("clear");
                sprintf(cmd, "cat %s", LOG_FILE);
                system(cmd);
                break;

            case 3: //启动采集

                cout << endl;
	            cout << "输入daq_rate:";
                scanf("%d", &daq_rate);
                cout << endl;
	            cout << "输入ch_cnt:" << endl;
                scanf("%d", &ch_cnt);
                cout << endl;
	            cout << "输入packet_type:" << endl;
                scanf("%d", &packet_type);
                cout << endl;
	            cout << "输入ch_en:" << endl;
                scanf("%d", &ch_en);

                start_daq_op = (struct Start_Daq_t *)send_buf;
                start_daq_op->header.tag        = TAG;
                start_daq_op->header.frame_len  = sizeof(struct Header_t);
                start_daq_op->header.cmd        = START_DAQ; 

                start_daq_op->daq_rate          = daq_rate; 
                start_daq_op->ch_cnt            = ch_cnt; 
                start_daq_op->ch_en             = ch_en; 
                start_daq_op->packet_type       = packet_type; 

                sendto(udpfd, send_buf, sizeof(struct Start_Daq_t), 0, (struct sockaddr*)&addr, sizeof(addr));

                nread = ReceiveUDPData(udpfd, recv_buf, MAX_MSG_LEN, NULL, NULL, 0);
                if (nread <= 0) {
                    cout << "        启动采集失败!" << endl;
                    cout << endl;
                    break;
                }

                cout << endl;
                pResult = (struct Result_t *)recv_buf;
                if(pResult->result == 0){
                    cout << "        启动采集成功!" << endl;
                    cout << endl;
                }else{
                    cout << "        启动采集失败!" << endl;
                    cout << endl;
                }

                break;

            case 4: //停止采集

                pHeader = (struct Header_t *)send_buf;

                pHeader->tag        = TAG;
                pHeader->frame_len  = sizeof(struct Header_t);
                pHeader->cmd        = STOP_DAQ;

                sendto(udpfd, send_buf, pHeader->frame_len, 0, (struct sockaddr*)&addr, sizeof(addr));

                nread = ReceiveUDPData(udpfd, recv_buf, MAX_MSG_LEN, NULL, NULL, 0);
                if (nread <= 0) {
                    cout << "        停止采集失败!" << endl;
                    cout << endl;
                    break;
                }

                cout << endl;
                pResult = (struct Result_t *)recv_buf;
                if(pResult->result == 0){
                    cout << "        停止采集成功!" << endl;
                    cout << endl;
                }else{
                    cout << "        停止采集失败!" << endl;
                    cout << endl;
                }

                break;

            default:
                cout<<"		选项错误，请重新输入："<<endl;
                break;
        }
    }

    return 0;
}
