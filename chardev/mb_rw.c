
#include <linux/fs.h>
#include "mailbox.h"

//static char text[MESSAGE_SIZE];

ssize_t mb_read (struct file *file, char __user *buffer, size_t length, loff_t *offset);
ssize_t mb_write (struct file *file,const char __user *buffer, size_t length, loff_t *offset); 


ssize_t mb_read (struct file *file, char __user *buffer, size_t length, loff_t *offset){

	mb_channel_s *channel = file->private_data;
	int msg_len;

	//block if empty
	if(wait_event_interruptible(channel->read_queue, channel->count > 0)) {
		return -ERESTARTSYS;
	}

	msg_len = strlen(channel->messages[channel->head].text);
	if(copy_to_user( buffer, channel->messages[channel->head].text, msg_len)){
		return -EFAULT;
	}

	pr_info("mailbox: read message - %s, count = %d\n",channel->messages[channel->head].text,channel->count);
	
	channel->head = (channel->head + 1) % FIFO_LIMIT;
	channel->count--;

	

	wake_up_interruptible(&channel->write_queue);
	return msg_len;
}

ssize_t mb_write (struct file *file, const char __user *buffer, size_t length, loff_t *offset) {

	mb_channel_s *channel = file->private_data;

	//prevent it from being too big
	if(length > MESSAGE_SIZE){
		length = MESSAGE_SIZE;
	}
	if (length == 0){
		return 0;
	}

	if(wait_event_interruptible(channel->write_queue, channel->count < FIFO_LIMIT)) {
		return -ERESTARTSYS;
	}

	if (copy_from_user(channel->messages[channel->tail].text, buffer, length)) {
		return -EFAULT;
	}
	channel->tail = (channel->tail + 1) % FIFO_LIMIT;
	channel->count++;

	pr_info( "mailbox: recieved message, count = %d\n", channel->count);
	wake_up_interruptible(&channel->read_queue);
	return length; 

}
