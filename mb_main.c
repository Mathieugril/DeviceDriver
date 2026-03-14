#include "mailbox.h"
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/proc_fs.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
 
#define HAVE_PROC_OPS  
#endif  


#define PROCFS_ENTRY_FILENAME "mailbox"  

static ssize_t procfs_read(struct file *filp, char __user *buffer, size_t length, loff_t *offset);
static ssize_t procfs_write(struct file *file, const char __user *buffer, size_t len, loff_t *off);
static int procfs_open(struct inode *inode, struct file *file);
static int procfs_close(struct inode *inode, struct file *file);


#ifdef HAVE_PROC_OPS  
static struct proc_ops file_ops_4_our_proc_file = {
 
    .proc_read = procfs_read,  
    .proc_write = procfs_write,
 
    .proc_open = procfs_open,  
    .proc_release = procfs_close,  
};
 
#else  
static const struct file_operations file_ops_4_our_proc_file = {  
    .read = procfs_read,  
    .write = procfs_write,  
    .open = procfs_open,
 
    .release = procfs_close,  
};  
#endif  

mb_build_s mb_build;

static int major;
static int mb_open(struct inode *inode, struct file *file); 
static int mb_release(struct inode *inode, struct file *file); 
extern ssize_t mb_read (struct file *file, char __user *buffer, size_t length, loff_t *offset);
extern ssize_t mb_write (struct file *file, const char __user *buffer, size_t length, loff_t *offset);
extern long mb_ioctl(struct file *file, unsigned int cmd, unsigned long arg);


static struct file_operations mb_fops = {
	.read = mb_read,
	.write = mb_write,
	.open = mb_open,
	.release = mb_release,
	.unlocked_ioctl = mb_ioctl,	

};

static struct class *cls; //used far below, in init, makes /dev entries

mb_channel_s channels[CHANNELS_NUM];

static int __init mb_init(void) {
	
	
	major = register_chrdev(0, "mailbox", &mb_fops);
	if(major < 0) {
	  printk("main.c - [ERROR] registering chardev");
	  return major;
	}
	printk("Welcome - Major Device Number: %d\n", major);
	cls = class_create("mib");	
	for (int i = 0; i < CHANNELS_NUM; i++) {
		char dev_entry_name[10];

		snprintf(dev_entry_name,10,"mailbox%d",i);
		pr_debug("%s",dev_entry_name);

		device_create(cls, NULL, MKDEV(major, i), NULL, dev_entry_name);  
		spin_lock_init(&channels[i].lock);
		init_waitqueue_head(&channels[i].read_queue);
		init_waitqueue_head(&channels[i]. write_queue);
		channels[i].head = 1;
		channels[i].tail = 0;	
		channels[i].count = 0;
	}

	//add proc + set size + user
	our_proc_file = proc_create(PROCFS_ENTRY_FILENAME, 0644, NULL, &file_ops_4_our_proc_file);
    
	if (our_proc_file == NULL) {
        pr_debug("Error: Could not initialize /proc/%s\n", PROCFS_ENTRY_FILENAME);
 
        return -ENOMEM;  
    }  
    proc_set_size(our_proc_file, 80);
    proc_set_user(our_proc_file, GLOBAL_ROOT_UID, GLOBAL_ROOT_GID);  
    pr_debug("/proc/%s created\n", PROCFS_ENTRY_FILENAME);  //like printk but only prints when /*#define DEBUG*/ is present
	
	
	

	return 0;
}

static int mb_open(struct inode *inode, struct file *file) {
	int minor = iminor(inode);
	if (minor >= CHANNELS_NUM) {
		pr_err("mailbox: channel with invalid minor was was called: minor %d", minor);
		return -ENODEV;
	}

	file->private_data = &channels[minor];
	pr_info("mailbox: channel %d opened\n", minor);
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
	remove_proc_entry(PROCFS_ENTRY_FILENAME, NULL);
    pr_debug("/proc/%s removed\n",PROCFS_ENTRY_FILENAME); 

	unregister_chrdev(major, "mailbox");
	printk("Goodbye - Major Device Number: %d\n", major);

	for(int i=0;i<CHANNELS_NUM;i++){
	device_destroy(cls, MKDEV(major, i));
	}  
    class_destroy(cls);
}



MODULE_LICENSE("GPL");
MODULE_AUTHOR("Group 3");
MODULE_DESCRIPTION("Message handling system: mailbox driver using LED display with 5 channels");


module_init(mb_init);
module_exit(mb_exit);
