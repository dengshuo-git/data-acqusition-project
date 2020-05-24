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
#include <iostream>

#include "protocal.h"
#include "socketfns.h"

using namespace std;

int net_recv(int socket, uint8_t *buf, uint32_t *len)
{
	struct epoll_event ev;
	int err;
	int epfd;

    int tmp_fd;
    int flags = 0;

    int ret;

	epfd = epoll_create(10);
	if(epfd < 0){
		perror("create epoll fd error");
		return 0;
	}

    struct Result_t *result;
    add_fd_epollset(epfd, socket, EPOLLIN);

    while( 1 ) {
        err = epoll_wait(epfd, &ev, 1, 60000);
        if (err < 0) {
            perror("epoll_wait()");
            goto out;

        }else if (err == 0) {

        }else {
            tmp_fd = ev.data.fd;

			if(ev.events  & (EPOLLHUP | EPOLLERR)){
                printf("disconnect!\n");
				/*when net-peer close the socket by signal, 
				  epoll will return both EPOLLIN and (EPOLLHUP|EPOLLERR) */
                ret = -1;
                break;
            }

            if(socket == tmp_fd){

                ret = tcp_read(socket, buf, &flags, epfd);
				if(ret != 0) continue;

                result = (struct Result_t *)buf;

                *len = result->frame_len;

                ret = 0;

                break;

			}else{
                printf("unknown epoll event(%x) happened on fd(%d), the fd is not listen by epoll", ev.events, tmp_fd);
                continue;
            }
        }
    }
out:

	err = epoll_ctl(epfd, EPOLL_CTL_DEL, socket, &ev);
	if (err < 0){
		perror("epoll_ctl()");
    }

    close(epfd);

    return 0;

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
    cout << "+----------------------------------+" << endl;
	cout << "|         上位机功能测试           |" << endl;
    cout << "+----------------------------------+" << endl; 
    cout << "|           1.启动采集             |" << endl;
    cout << "|           2.停止采集             |" << endl;
    cout << "|           3.心跳测试             |" << endl;
    cout << "|           0.退出程序             |" << endl;
    cout << "+----------------------------------+" << endl; 
    cout << endl;
    cout << "请输入您的选择:" << endl;
    return;
}

int main(int argc, char*argv[])
{
    int s;
    int ret;
    struct sockaddr_in server_addr;
    int i;

    int sel;
    char cmd[256] = {0};
    uint8_t recv_buf[4096];
    uint8_t send_buf[4096];

    struct Header_t *header = (struct Header_t *)send_buf;
    struct Heart_Result_t *result;

    struct Heart_Beat_t *heart = (struct Heart_Beat_t*)send_buf;
    struct Heart_Result_t *heart_result;


    s = socket(AF_INET, SOCK_STREAM, 0);

    uint8_t *data = NULL;

    struct Header_t *pHeader;
    struct Result_t *pResult;
    struct Start_Daq_t *start_daq_op; 

    uint32_t daq_rate = 0; 
    uint32_t ch_cnt = 0;
    uint32_t ch_en = 0; 
    uint32_t packet_type = 0;

    uint32_t recv_len = 0;


    if(argv[1] == NULL){
        cout << "请输入服务端ip" << endl;
        return 0;
    }

    if(argv[2] == NULL){
        cout << "请输入服务端port" << endl;
        return 0;
    }


    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(atoi(argv[2]));

    inet_pton(AF_INET,argv[1],&server_addr.sin_addr);

    ret = connect(s, (struct sockaddr*)&server_addr, sizeof(struct sockaddr));
    if(!ret){
        printf("****************connect sucess!*****************\n");
    }else{
        printf("****************connect failed!*****************\n");
        close(s);
        return 0;
    }

    while(1){

        display_root_menu();	

        sel = get_input_value();

        switch(sel){

            case 0://退出程序
                exit(1);
                break;

            case 1: //启动采集

                cout << endl;

                cout << "输入ch_cnt:" << endl;
                cin >> ch_cnt;
                cout << endl;

                cout << "输入daq_rate:" << endl;
                cin >> daq_rate;
                cout << endl;

                cout << "输入packet_type:" << endl;
                cin >> packet_type;
                cout << endl;

                cout << "输入ch_en:" << endl;
                cin >> ch_en;
                cout << endl;

                start_daq_op = (struct Start_Daq_t *)send_buf;
                start_daq_op->header.tag        = TAG;
                start_daq_op->header.frame_len  = sizeof(struct Start_Daq_t); 
                start_daq_op->header.cmd        = START_DAQ; 

                start_daq_op->daq_rate          = daq_rate; 
                start_daq_op->ch_cnt            = ch_cnt; 
                start_daq_op->ch_en             = (2 << (ch_en -1)) - 1; 
                start_daq_op->packet_type       = packet_type; 

                tcp_write(s, send_buf, start_daq_op->header.frame_len, 0);

                net_recv(s, recv_buf, &recv_len);

                pResult = (struct Result_t *)recv_buf;
                if(pResult->result == 0){
                    cout << "启动采集成功!" << endl;
                    cout << endl;
                }else{
                    cout << "启动采集失败!" << endl;
                    cout << endl;
                }

                break;

            case 2: //停止采集

                pHeader = (struct Header_t *)send_buf;

                pHeader->tag        = TAG;
                pHeader->frame_len  = sizeof(struct Header_t);
                pHeader->cmd        = STOP_DAQ;

                tcp_write(s, send_buf, pHeader->frame_len, 0);

                net_recv(s, recv_buf, &recv_len);

                pResult = (struct Result_t *)recv_buf;
                if(pResult->result == 0){
                    cout << "启动采集成功!" << endl;
                    cout << endl;
                }else{
                    cout << "启动采集失败!" << endl;
                    cout << endl;
                }

            case 3: //心跳功能测试

                for(i = 0;i < 20;i++){
                    heart->header.tag        = TAG;
                    heart->header.frame_len  = sizeof(struct Heart_Beat_t); 
                    heart->header.cmd        = HEARTBEAT; 
                    heart->ask               = 0x1234; 

                    tcp_write(s, send_buf, heart->header.frame_len, 0);

                    net_recv(s, recv_buf, &recv_len);

                    heart_result = (struct Heart_Result_t *)recv_buf;
                    if(heart_result->answer == 0x5678){
                        printf("recv a reply!\n");
                    }else{
                        printf("recv a error reply!,answer = %x\n", result->answer);
                    }
                    sleep(1);
                }

                break;

            default:
                cout<<"		选项错误，请重新输入："<<endl;
                break;
        }
    }

    sleep(3);
    close(s);

    return 0;
}
