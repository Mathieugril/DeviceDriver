#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mathieu");
MODULE_DESCRIPTION("Message handling system");

static int major;



static ssize_t my_read(struct file *f, char __user *u, size_t l, loff_t *o)
{
	printk("main.c - read is called\n");
	return 0;

}

static struct file_operations fops = {
	.read = my_read
};


static int __init my_init(void) {

	major = register_chrdev(0, "main.c", &fops);
	if(major < 0) {
	  printk("main.c - [ERROR] registering chardev");
	  return major;
	}
	printk("Welcome - Major Device Number: %d\n", major);
	return 0;
}

static void __exit my_exit(void)
{
	unregister_chrdev(major, "main.c");

}
module_init(my_init);
module_exit(my_exit);
