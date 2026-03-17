#define DEBUG

#ifndef MAILBOX_H
#define MAILBOX_H



 #define PROCFS_MAX_SIZE 2048UL

#define CHANNELS_NUM 5
#define DEFAULT_FIFO_LIMIT 16
#define MESSAGE_SIZE 256

#define MB_FLUSH _IO('m', 1)
#define MB_FLUSH_ALL _IO('m',2)
#define MB_GET_COUNT _IOR('m',3 ,int)
#define MB_SET_MAX _IOW('m',4 ,int)

#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stddef.h>
#endif

// single message that will be stored in fifo buffer
typedef struct {
	size_t length;
	char text[MESSAGE_SIZE];
} mb_msg_s;

typedef struct mb_msg_node {
	mb_msg_s msg;
	struct mb_msg_node *next;
	struct mb_msg_node *prev;
} mb_msg_node;

typedef struct {
	int channel;
	int queued;
	int capacity;
	unsigned long sent;
	unsigned long received;
} channel_stats_s;



#ifdef __KERNEL__ //every thing in it is just for kernal space
#include <linux/cdev.h>
#include <linux/wait.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>

#include <linux/module.h>
#include <linux/proc_fs.h>  
#include <linux/sched.h>  
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/slab.h>


#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
 
#include <linux/minmax.h>  
#endif
 
// #define PROCFS_ENTRY_FILENAME "mailbox"  
static struct proc_dir_entry *our_proc_file;
 
static char procfs_buffer[PROCFS_MAX_SIZE];  
static unsigned long procfs_buffer_size = 0;

// channel for each mailbox
typedef struct {
	int count,capacity,sent,received;
	//mb_circ_fifo_s fifo;

	mb_msg_node *head;
	mb_msg_node *tail;
	
	wait_queue_head_t read_queue; // reader sleepssss when fifo empty
	wait_queue_head_t write_queue; // writers sleep here when fifo full
	spinlock_t lock;
	bool is_open;
	
	// proc stats needed
	// gpio for leds needed
} mb_channel_s;


// build that registers chardev and gets major/minor numbers
typedef struct {
	mb_channel_s channels[CHANNELS_NUM];
	struct class *dev_class;
	struct cdev cdev; 	
	dev_t dev_num;
} mb_build_s;

#endif
/* declared in main, extern here for other files
extern mb_build_s mb_build;
extern mb_channel_s channels[CHANNELS_NUM];
*/
#endif
