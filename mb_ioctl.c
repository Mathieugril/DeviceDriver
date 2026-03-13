

#include "mailbox.h"
#include <linux/fs.h>
#include <linux/uaccess.h>



static int ioctl_flush(mb_channel_s *channel) {

    channel->count = 0;
    channel->tail = 0;
    channel->head = 0;

    wake_up_interruptible(&channel->write_queue);
    pr_info("mailbox: flushed channel\n");
    return 0;

}

static int ioctl_flush_all(void) {

    for(int i = 0; i < CHANNELS_NUM; i++) {
    channels[i].count = 0;
    channels[i].tail = 0;
    channels[i].head = 0;
    wake_up_interruptible(&channels[i].write_queue);
    }
    pr_info("mailbox: flushed all channels\n");
    return 0;

}

static int ioctl_get_count(mb_channel_s *channel, unsigned long arg) {

    int count = channel->count;
    if(put_user(count, (int __user *)arg)) {
        return -EFAULT;
    }
    return 0;

}

static int ioctl_set_max(mb_channel_s *channel, unsigned long arg) {

    // needs to be done
    //
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


