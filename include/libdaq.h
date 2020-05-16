#ifndef _ALG_API_H_
#define _ALG_API_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C"{
#endif


/**********************************
功能：打开设备
返回值：设备描述符
***********************************/			
extern 	int32_t Tspi_OpenDevice(void);

/**********************************
功能：关闭设备
        dev_fd[in]		    设备描述符
返回值：0 成功    非0 失败
***********************************/			
extern 	int32_t Tspi_CloseDevice(int dev_fd);

/**********************************
功能：开始采集
        dev_fd              设备描述符

返回值：0 成功    非0 失败
***********************************/
extern int start_daq(int dev_fd);

/**********************************
功能：停止采集
        dev_fd              设备描述符

返回值：0 成功    非0 失败
***********************************/
extern int stop_daq(int dev_fd);

/**********************************
功能:   填充数据包
        dev_fd[in] 	        设备描述符	
        data[in] 	        待填充的数据缓冲区 
返回值：0 成功    
***********************************/
extern int fill_packet_header(int dev_fd, uint8_t *data);

/**********************************
功能:   设置采集通用参数,与配置文件中相应参数对应 
        type[in] 	        数据包类型 0：震动数据 1：温度数据 
        rate[in] 	        采样速率
        ch_en[in] 	        通道使能 
        ch_num[in] 	        通道数量 
        pt_num[in] 	        每通道采样点 
返回值：0 成功    
***********************************/
extern int set_daq_paramter(int dev_fd, uint32_t type, uint32_t rate, uint32_t ch_en, int ch_num);

#endif


#ifdef __cplusplus
}
#endif
