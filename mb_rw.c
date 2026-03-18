
#include <linux/fs.h>
#include "mailbox.h"


ssize_t mb_read (struct file *file, char __user *buffer, size_t length, loff_t *offset);
ssize_t mb_write (struct file *file,const char __user *buffer, size_t length, loff_t *offset); 


ssize_t mb_read (struct file *file, char __user *buffer, size_t length, loff_t *offset){
	
	mb_channel_s *channel = file->private_data;
	int msg_len;
	mb_msg_node *node;
	unsigned long flags;

	//block if empty
	if(wait_event_interruptible(channel->read_queue, channel->count > 0)) {
		return -ERESTARTSYS;
	}


	spin_lock_irqsave(&channel->lock, flags);
	if(channel->count == 0){
		spin_unlock_irqrestore(&channel->lock, flags);
		return -EAGAIN;
	}


	node = channel->head;
	msg_len = node->msg.length;

	channel->head = node->next;
	if (channel->head)
		channel->head->prev = NULL;
	else
		channel->tail = NULL;

	channel->count--;
	channel->received++;

	spin_unlock_irqrestore(&channel->lock,flags);

	if(copy_to_user(buffer, node->msg.text, msg_len)) {
		kfree(node);
		return -EFAULT;
	}

	pr_info("mailbox: read message - %s, count = %d\n",node->msg.text,channel->count);
	kfree(node);

	wake_up_interruptible(&channel->write_queue);
	return msg_len;
}

ssize_t mb_write (struct file *file, const char __user *buffer, size_t length, loff_t *offset) {

	mb_channel_s *channel = file->private_data;
	mb_msg_node *node;
	unsigned long flags;

	//prevent it from being too big
	if(length > MESSAGE_SIZE){
		length = MESSAGE_SIZE;
	}
	if (length == 0){
		return 0;
	}

	if(wait_event_interruptible(channel->write_queue, channel->count < channel->capacity)) {
		return -ERESTARTSYS;
	}

	node = kmalloc(sizeof(mb_msg_node), GFP_KERNEL);
	if (!node) {
		return -ENOMEM;
	}

	if (copy_from_user(node->msg.text, buffer, length)) {
		kfree(node);
		return -EFAULT;
	}

	node->msg.length = length;
	node->next = NULL;

	spin_lock_irqsave(&channel->lock,flags);
	if(channel->count >= channel->capacity) {
		spin_unlock_irqrestore(&channel->lock, flags);
		kfree(node);
		return -EAGAIN;
	}


	node->prev = channel->tail;

	if (channel->tail)
		channel->tail->next = node;
	else
		channel->head = node;

	channel->tail = node;

	channel->count++;
	channel->sent++;
	spin_unlock_irqrestore(&channel->lock, flags);

	pr_info( "mailbox: recieved message, count = %d\n", channel->count);

	wake_up_interruptible(&channel->read_queue);
	return length; 

}
