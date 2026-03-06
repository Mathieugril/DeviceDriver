#include "mailbox.h"
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>

mb_build mb_build;

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

};

static int __init mb_init(void) {
	major = register_chrdev(0, "mailbox.c", &mb_fops);
	if(major < 0) {
	  printk("main.c - [ERROR] registering chardev");
	  return major;
	}
	printk("Welcome - Major Device Number: %d\n", major);
	return 0;
}

static int mb_open(struct inode *inode, struct file *file) {
	printk(KERN_INFO "mailbox: opened\n");
	return 0;
}

static int mb_release(struct inode *inode, struct file *file) {
	printk(KERN_INFO "mailbox: closed\n");
	return 0;
}

static void __exit mb_exit(void) {
	unregister_chrdev(major, "main.c");
	printk("Goodbye - Major Device Number: %d\n", major);

}



MODULE_LICENSE("GPL");
MODULE_AUTHOR("Group 3");
MODULE_DESCRIPTION("Message handling system");


module_init(mb_init);
module_exit(mb_exit);
