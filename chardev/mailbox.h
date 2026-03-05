#ifndef MAILBOX_H
#define MAILBOX_H

#include <linux/kernel.h>


#define CHANNELS_NUM 5
#define FIFO_LIMIT 16
#define MESSAGE_SIZE 256



typedef struct {
	size_t length;
	char text[MESSAGE_SIZE];
} mb_msg;


#endif
