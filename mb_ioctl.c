

#include "mailbox.h"
#include <linux/fs.h>
#include <linux/uaccess.h> 

extern mb_channel_s channels[CHANNELS_NUM];
void free_channel_messages(mb_channel_s *channel);
long mb_ioctl(struct file *file, unsigned int cmd, unsigned long arg);


 void free_channel_messages(mb_channel_s *channel) {
	mb_msg_node *node = channel->head;
	while (node) {
		mb_msg_node *next = node->next;
		kfree(node);
		node = next;
	}
	channel->head = NULL;
	channel->tail = NULL;
	channel->count = 0;
}

static int ioctl_flush(mb_channel_s *channel) {

    unsigned long flags;

    spin_lock_irqsave(&channel->lock, flags);
    free_channel_messages(channel);
    spin_unlock_irqrestore(&channel->lock, flags);

    wake_up_interruptible(&channel->write_queue);
    pr_info("mailbox: flushed channel\n");
    return 0;

}

static int ioctl_flush_all(void) {

    unsigned long flags;

    for(int i = 0; i < CHANNELS_NUM; i++) {
	spin_lock_irqsave(&channels[i].lock, flags);
        free_channel_messages(&channels[i]);
	spin_unlock_irqrestore(&channels[i].lock, flags);

        wake_up_interruptible(&channels[i].write_queue);
    }
    pr_info("mailbox: flushed all channels\n");
    return 0;

}

static int ioctl_get_count(mb_channel_s *channel, unsigned long arg) {

    unsigned long flags;


    spin_lock_irqsave(&channel->lock, flags);
    int count = channel->count;
    spin_unlock_irqrestore(&channel->lock, flags);

    if(put_user(count, (int __user *)arg)) {
        return -EFAULT;
    }
    return 0;

}

static int ioctl_set_max(mb_channel_s *channel, unsigned long arg) {
    int new_max;
    unsigned long flags;

    if (get_user(new_max, (int __user *)arg)) {
        return -EFAULT;
    }

    if (new_max <= 0) {
        return -EINVAL;
    }

    spin_lock_irqsave(&channel->lock, flags);
    if (new_max < channel->count) {
        spin_unlock_irqrestore(&channel->lock, flags);
        return -EINVAL;
    }

    channel->capacity = new_max;
    spin_unlock_irqrestore(&channel->lock, flags);

    wake_up_interruptible(&channel->write_queue);
    pr_info("mailbox: set channel max capacity to %d\n", new_max);
    return 0;
}

long mb_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {

	mb_channel_s *channel = file->private_data;

	switch(cmd) {
	case MB_FLUSH:
		return ioctl_flush(channel);
		break;
   	case MB_FLUSH_ALL:
        	return ioctl_flush_all();
        	break;
	case MB_GET_COUNT:
		return ioctl_get_count(channel, arg);
		break;
	case MB_SET_MAX:
		return ioctl_set_max(channel, arg);
		break;
	default:
		return -ENOTTY;

	}

}


