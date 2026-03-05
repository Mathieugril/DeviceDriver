#include "mailbox.h"
#include <stdio.h>
#include <linux/fs.h>
#include <linux/uaccess.h>


ssize_t mb_read (struct file *file, char __user *buffer, size_t length, lofft_t *offset) {


}

ssize_t mb_write (struct file *file, char __user *buffer, size_t length, loff_t *offset) {


}
