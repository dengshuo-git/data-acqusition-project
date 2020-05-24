#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <asm/signal.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <linux/proc_fs.h>
#include <linux/semaphore.h>
#include <asm/errno.h>
#include <linux/unistd.h>
#include <linux/time.h>
#include <linux/wait.h>
#include <asm/uaccess.h>
#include <linux/sched.h>
#include <linux/mutex.h>
#include <linux/poll.h>
#include <linux/kthread.h>
#include <linux/string.h>
#include <linux/hash.h>
#include <linux/list.h>
#include <linux/time.h>
#include <linux/timex.h>
#include <linux/rtc.h>
#include <linux/delay.h>
#include <linux/swiotlb.h>
#include <linux/dma-mapping.h>
#include "driver.h"
#include "debug.h"


struct DEVICE_INFO_ST *data_acquisition_dev;

int acquisition_action = STOP_DAQ;  // 1: 开始采集     2：停止采集

int daq_rate = 0;
int ch_cnt = 0;
int pt_cnt = 0;

int utimes = 0;

static int setup_cdev(struct DEVICE_INFO_ST *dev_info, dev_t dev_no);

unsigned int dev_poll(struct file *filp, poll_table *wait)
{
	unsigned int mask = 0;
    struct queues* queue = &data_acquisition_dev->data_queue;

	poll_wait(filp, &queue->r_wait,  wait);
    
	mutex_lock(&queue->queue_mutex);
    if(queue->queue_head != queue->queue_tail){
 		mask |= POLLIN;
    }
	mutex_unlock(&queue->queue_mutex);
	
	return mask;
}

int dev_open(struct inode *inode, struct file *filp)
{
    return 0;
}

int dev_release(struct inode *inode, struct file *filp)
{
    return 0;
}

ssize_t dev_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    int queue_tail = 0;
    struct queues *queue = &data_acquisition_dev->data_queue; 
    uint8_t *src_buf;
    int32_t data_len;

    mutex_lock(&queue->queue_mutex);
    queue_tail = queue->queue_tail;
    mutex_unlock(&queue->queue_mutex);

    src_buf = (uint8_t *)&(queue->node[queue_tail]->data[0]);
    data_len = queue->data_len; 
    copy_to_user(buf, src_buf, data_len);

    queue_tail = (queue_tail + 1) % QUEUE_DEPTH;

    mutex_lock(&queue->queue_mutex);
    queue->queue_tail = queue_tail;
    mutex_unlock(&queue->queue_mutex);
    
    wake_up_interruptible(&(queue->w_wait));

	return 0;
}

ssize_t dev_write(struct file * filp,const char __user * buf,size_t count,loff_t * f_pos)
{
    struct queues *queue = &data_acquisition_dev->data_queue; 
    int cmd;

    copy_from_user(&cmd, buf, 4);

    switch(cmd){
        case START_DAQ: 
        case STOP_DAQ:
            mutex_lock(&queue->queue_mutex);
            acquisition_action = cmd;
            mutex_unlock(&queue->queue_mutex);

            if(daq_rate != 0){
                utimes = 1000000 / daq_rate;
            }else{
                utimes = 10000;
            }
                
            printk("utimes = %d\n",utimes);

            wake_up_interruptible(&(queue->w_wait));
            break;
        
        case SET_PARAMTER:
            
            copy_from_user(&daq_rate, &buf[4], 4);
            copy_from_user(&ch_cnt, &buf[8], 4);
            copy_from_user(&pt_cnt, &buf[12], 4);


            utimes = 1000000 / daq_rate;
            printk("utimes = %d\n", utimes);
            printk("ch_cnt = %d\n", ch_cnt);
            printk("pt_cnt = %d\n", pt_cnt);

            break;

        default:
            break;
    }
	return 0;
}

struct file_operations dev_fops =
{
	.owner              =   THIS_MODULE,
	.open               =   dev_open,
	.release            =   dev_release,
	.read               =   dev_read,
	.write              =   dev_write,
    .poll               =   dev_poll,
};


int data_acquisition_simulation(void *arg)
{
    int i;
    int j;
    int k;

    int mcnt = 0;
    int ucnt = 0;

    struct DEVICE_INFO_ST *this = data_acquisition_dev; 
    struct queues *queue = &this->data_queue; 

    int queue_head = 0;
    int queue_tail = 0;

    uint8_t *buf;

    int action = 0;

    int index = 0;

	DECLARE_WAITQUEUE(wait, current);
    add_wait_queue(&queue->w_wait, &wait);

    EnterFunction();

    while(1){
        mutex_lock(&queue->queue_mutex);
        queue_head = queue->queue_head;
        queue_tail = queue->queue_tail;
        action = acquisition_action;
        mutex_unlock(&queue->queue_mutex);

        if(kthread_should_stop()){
            debug("quit data_simulation!\n");
            goto out;
        }

        if((queue_head + 1) % QUEUE_DEPTH == queue_tail){ //队列已满或者停止采集
            __set_current_state(TASK_INTERRUPTIBLE);
            schedule();

        }else{
            if(action == START_DAQ){
                /*
                 * 填充数据
                 */
                buf = (uint8_t *)&(queue->node[queue_head]->data[0]);
                index = 0;
                for(i = 0; i <  pt_cnt; i++){
                    for(j = 0; j < ch_cnt; j++){
                        buf[index] = index;
                        index++;
                        buf[index] = index;
                        index++;
                    }

                    mcnt = utimes / 1000;
                    for(k = 0; k < mcnt; k++){
                        msleep(1);
                    }

                    ucnt = (utimes - mcnt * 1000);;
                    for(k = 0; k < ucnt; k++){
                        udelay(1);
                    }

                }

                mutex_lock(&queue->queue_mutex);
                queue->queue_head = (queue->queue_head + 1) % QUEUE_DEPTH;
                mutex_unlock(&queue->queue_mutex);

                wake_up_interruptible(&(queue->r_wait));

            }else{

                __set_current_state(TASK_INTERRUPTIBLE);
                schedule();
            }
        }
    }
out:
	remove_wait_queue(&queue->w_wait,&wait);
    LeaveFunction();
    return 0;
}

static int setup_cdev(struct DEVICE_INFO_ST *dev_info,dev_t dev_no)
{
	int ret;
	cdev_init(&dev_info->cdev, &dev_fops);
    dev_info->dev_no = dev_no;
	ret = cdev_add(&dev_info->cdev, dev_no, 1);
	return ret;
}

static void resource_cleanup(void)
{
    int i;
    struct DEVICE_INFO_ST *this = data_acquisition_dev; 
    struct queues *queue = &this->data_queue; 

    if(!data_acquisition_dev) return;

    /* 关闭task[i]线程 */
    wake_up_interruptible(&(queue->w_wait));
    kthread_stop(data_acquisition_dev->task);	
    data_acquisition_dev->task =  NULL;	

    /* 注销字符设备 */
    device_unregister(data_acquisition_dev->device);
    class_destroy(data_acquisition_dev->device_class);
	unregister_chrdev_region(data_acquisition_dev->dev_no, 1);  

    /* 释放队列 */

    queue = &data_acquisition_dev->data_queue;
    if(queue->node[0]){ 
        kfree(queue->node[0]);
    }

    for(i = 0; i < QUEUE_DEPTH; i++){
        queue->node[i] = NULL;
    }

    kfree(data_acquisition_dev); 

}

static int __init dev_driver_init(void)
{
    int i, retval;
    uint8_t *buff;
    struct queues *queue;
    struct class *device_class;
    struct device *device;
    dev_t dev_no;

	EnterFunction();

    data_acquisition_dev = (struct DEVICE_INFO_ST *)kmalloc(sizeof(struct DEVICE_INFO_ST), GFP_KERNEL);
    if(!data_acquisition_dev){
        info("alloced memory failed!\n");
        return -ENOMEM;
    }
    memset((void*)data_acquisition_dev, 0, sizeof(struct DEVICE_INFO_ST));

    /*
     * 1)创建字符设备
     */

    retval = alloc_chrdev_region(&dev_no, 0, 0, DEVICE_NAME);
    if (retval != 0) {
        alert("couldn't allocate device major number.\n ");
        goto failed;
    }

    retval = setup_cdev(data_acquisition_dev, dev_no);
    if(retval != 0){
        alert("couldn't bind dev_ops and cdev.\n ");
        goto failed;
    }
    data_acquisition_dev->dev_no = dev_no;
    
    device_class = class_create(THIS_MODULE, DEVICE_NAME);
    if (IS_ERR_OR_NULL(device_class)) {
        alert("class_create() failed.\n ");
        goto failed;
    }
    data_acquisition_dev->device_class = device_class;

    device = device_create(device_class, NULL, dev_no, NULL, DEVICE_NAME); //mknod /dev/data_acquisition
    if(IS_ERR(device)){
        alert("device_create() failed.\n");
        goto failed;
    }
    data_acquisition_dev->device = device;

    
    /*
     * 2)申请队列空间
     */
    buff = (uint8_t *)kmalloc(4096 * QUEUE_DEPTH, GFP_KERNEL);
    if(!buff){
        goto failed;
    }

    queue = &data_acquisition_dev->data_queue;
    queue->queue_head = 0; 
    queue->queue_tail = 0;

    queue->data_len = 4096;
    mutex_init(&queue->queue_mutex);

    init_waitqueue_head(&queue->w_wait);
    init_waitqueue_head(&queue->r_wait);
    
    for(i = 0; i < QUEUE_DEPTH; i++){
        queue->node[i] =  (struct queues_node *)(buff + 4096 * i);
    }



    /*
     * 3)创建数据产生线程，用于模拟数据采集动作
     */
#if 1    
    data_acquisition_dev->task = kthread_create(data_acquisition_simulation, NULL, "data_acquisition_simulation");
	if(!data_acquisition_dev->task){
        printk("create data_acquisition_simulation task error!\n");
	    retval = PTR_ERR(data_acquisition_dev->task);
	    goto failed;
	}
//    do_set_cpus_allowed(data_acquisition_dev->task,cpumask_set_cpu(1, &mask));
    wake_up_process(data_acquisition_dev->task);
#endif
    set_debug_level(DEBUG_LEVEL_DEBUG);

	LeaveFunction();
    
    return 0;

failed:
    resource_cleanup();
	LeaveFunction();

    return 0;
}

static void __exit dev_driver_exit(void)
{
	EnterFunction();

    if(data_acquisition_dev == NULL)
        return;

    resource_cleanup();

	LeaveFunction();

    return;
}


module_init(dev_driver_init);
module_exit(dev_driver_exit);


MODULE_LICENSE("GPL");
