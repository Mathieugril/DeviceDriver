#include "mailbox.h"
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/kernel.h>

mb_build_s mb_build;

static int major;
static int mb_open(struct inode *inode, struct file *file); 
static int mb_release(struct inode *inode, struct file *file); 
extern ssize_t mb_read (struct file *file, char __user *buffer, size_t length, loff_t *offset);
extern ssize_t mb_write (struct file *file, const char __user *buffer, size_t length, loff_t *offset);


static struct file_operations mb_fops = {
	.read = mb_read,
	.write = mb_write,
	.open = mb_open,
	.release = mb_release,
//	.unlocked_ioctl - mb_ioctl,	

};
mb_channel_s channel;
static int __init mb_init(void) {
	
	
	major = register_chrdev(0, "mailbox.c", &mb_fops);
	if(major < 0) {
	  printk("main.c - [ERROR] registering chardev");
	  return major;
	}
	printk("Welcome - Major Device Number: %d\n", major);

	spin_lock_init(&channel.lock);
	init_waitqueue_head(&channel.read_queue);
	init_waitqueue_head(&channel. write_queue);
	channel.head = 0;
	channel.tail = 0;	
	channel.count = 0;

	return 0;
}

static int mb_open(struct inode *inode, struct file *file) {
	int minor = iminor(inode);
	if (minor >= CHANNELS_NUM) {
		pr_err("mailbox: channel with invalid minor was was called: minor %d", minor);
		return -ENODEV;
	}

	pr_info("mailbox: channel %d opened\n", minor);
	file->private_data = &channel;
	mb_build.channels[minor].is_open = true;


	return 0;
}

static int mb_release(struct inode *inode, struct file *file) {
	
	mb_channel_s *chan = file->private_data;
	int minor = iminor(inode);
	pr_info("mailbox: closed %d closed\n", minor);

	chan->is_open = false;

	wake_up_interruptible(&chan->read_queue);
	wake_up_interruptible(&chan->write_queue);
	
	return 0;
}

static void __exit mb_exit(void) {
	unregister_chrdev(major, "main.c");
	printk("Goodbye - Major Device Number: %d\n", major);

}



MODULE_LICENSE("GPL");
MODULE_AUTHOR("Group 3");
MODULE_DESCRIPTION("Message handling system: mailbox driver using LED display with 5 channels");


module_init(mb_init);
module_exit(mb_exit);
