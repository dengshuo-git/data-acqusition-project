#ifndef _DATA_ACQUISITION_CARD_H__
#define _DATA_ACQUISITION_CARD_H__

#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/cdev.h>
#include <linux/list.h>
#include <linux/skbuff.h>

#define DEVICE_NAME             "data_acquisition_card"
#define GET_PHY_ADDR            3
#define QUEUE_DEPTH             8
#define NODE_SIZE               0x1000

#define     START_DAQ           0x0101
#define     STOP_DAQ            0x0102
#define     REBOOT              0x0103
#define     SET_PARAMTER        0x0109

struct queues_node{
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

struct queues{
    struct mutex queue_mutex;

    uint32_t queue_head;
    uint32_t queue_tail;
	uint32_t queue_depth;

    struct queues_node *node[QUEUE_DEPTH];
    int      data_len;

    wait_queue_head_t w_wait;   //对于发送队列:用户线程使用w_wait,发送线程使用r_wait
    wait_queue_head_t r_wait;   //对于接收队列:接收线程使用r_wait,w_wait未使用
}; 

struct DEVICE_INFO_ST{
    dev_t    dev_no;
    struct cdev     cdev;
    struct class    *device_class;
    struct device   *device;
    struct task_struct *task;
    struct queues data_queue;
};

int dev_open(struct inode *inode, struct file *filp);
int dev_release(struct inode *inode, struct file *filp);
ssize_t dev_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
ssize_t dev_write(struct file * filp,const char __user * buf,size_t count,loff_t * f_pos);
long dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

#endif
