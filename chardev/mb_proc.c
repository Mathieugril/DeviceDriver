#include "mailbox.h"

 ssize_t procfs_read(struct file *filp, char __user *buffer, size_t length, loff_t *offset);
 ssize_t procfs_write(struct file *file, const char __user *buffer, size_t len, loff_t *off);
 int procfs_open(struct inode *inode, struct file *file);
 int procfs_close(struct inode *inode, struct file *file);

 ssize_t procfs_read(struct file *filp, char __user *buffer, size_t length, loff_t *offset)  
{
 
    if (*offset || procfs_buffer_size == 0) {  
        pr_debug("procfs_read: END\n");
 
        *offset = 0;  
        return 0;  
    }  
    procfs_buffer_size = min(procfs_buffer_size, length);
 
    if (copy_to_user(buffer, procfs_buffer, procfs_buffer_size))
 
        return -EFAULT;  
    *offset += procfs_buffer_size;  

 
    pr_debug("procfs_read: read %lu bytes\n", procfs_buffer_size);  
    return procfs_buffer_size;
 
}  

ssize_t procfs_write(struct file *file, const char __user *buffer, size_t len, loff_t *off)  
{  
    procfs_buffer_size = min(PROCFS_MAX_SIZE, len);
 
    if (copy_from_user(procfs_buffer, buffer, procfs_buffer_size))  
        return -EFAULT;
 
    *off += procfs_buffer_size;  
  
    pr_debug("procfs_write: write %lu bytes\n", procfs_buffer_size);
 
    return procfs_buffer_size;  
}  

int procfs_open(struct inode *inode, struct file *file)
 
{  
    return 0;  
}  
 int procfs_close(struct inode *inode, struct file *file)  
{
 
    return 0;  
}  