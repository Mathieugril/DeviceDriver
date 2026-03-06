#ifndef MAILBOX_H
#define MAILBOX_H

#include <linux/cdev.h>
#include <linux/wait.h>
#include <linux/kernel.h>
#include <linux/spinlock.h/>


#define CHANNELS_NUM 5
#define FIFO_LIMIT 16
#define MESSAGE_SIZE 256


// single message that will be stored in fifo buffer
typedef struct {
	size_t length;
	char text[MESSAGE_SIZE];
} mb_msg;


//circular fifo buffer needed
typedef 
struct {
	mb_msg messages[FIFO_LIMIT];
	int head,tail,count,capacity;
} mb_circ_fifo;


// channel for each mailbox
tyepdef struct {
	mb_circ_fifo fifo;
	
	wait_queue_head_t read_queue; // reader sleepssss when fifo empty
	wait_queue_head_t write_queue; // writers sleep here when fifo full
	spinlock_t lock;
	
	// proc stats needed
	// gpio for leds needed
} mb_channel;


// build that registers chardev and gets major/minor numbers
typedef struct {
	mb_channel channels[CHANNELS_NUM];
	struct class *dev_class;
	struct cdev cdev; 	
	dev_t dev_num;
} mb_build;

// declared in main, extern here for other files
extern mb_build mb_build;

#endif
