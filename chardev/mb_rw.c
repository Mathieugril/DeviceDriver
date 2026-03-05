
#include <linux/fs.h>
#include "mailbox.h"

static char text[MESSAGE_SIZE];

ssize_t mb_read (struct file *file, char __user *buffer, size_t length, loff_t *offset);
ssize_t mb_write (struct file *file,const char __user *buffer, size_t length, loff_t *offset); 


ssize_t mb_read (struct file *file, char __user *buffer, size_t length, loff_t *offset){
  

	copy_to_user(buffer, text, strlen(text));
	return strlen(text);
}

ssize_t mb_write (struct file *file, const char __user *buffer, size_t length, loff_t *offset) {

	int ret;
	memset(text, 0, MESSAGE_SIZE);
	ret = copy_from_user(text, buffer, length);
	printk(KERN_INFO "mailbox recieved message: %s, ret = %d, length = %zu\n", text, ret, length);
	return length; 

}
