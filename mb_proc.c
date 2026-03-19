#include "mailbox.h"


extern mb_channel_s channels[CHANNELS_NUM];
struct proc_dir_entry *our_proc_file;
static char procfs_buffer[PROCFS_MAX_SIZE];
static unsigned long procfs_buffer_size = 0;
void copy_string_to_buffer(char *string, unsigned long *position);
void copy_char_to_buffer(char character, unsigned long *position);
void set_proc_buffer_contents(void);
ssize_t procfs_read(struct file *file, char __user *buffer, size_t length, loff_t *offset);
ssize_t procfs_write(struct file *file,const char __user *buffer, size_t len, loff_t *off);



void copy_string_to_buffer(char* string,unsigned long *postion){  
    for(int i=0;;i++){
        if(string[i]=='\0'||*postion>PROCFS_MAX_SIZE){
            break;
        }
        procfs_buffer[*postion]=string[i];
        *postion+=1;
        
    }
    return;
}
void copy_char_to_buffer(char character,unsigned long *postion){ 
    procfs_buffer[*postion]=character;
    *postion+=1;
    return;
}


void set_proc_buffer_contents(void){    //generates contents to put in proc file 
    unsigned long current_postition = 0;  
    unsigned long flags;
    mb_channel_s ch_stats;

    for(int i=0;i<PROCFS_MAX_SIZE;i++){ 
	procfs_buffer[i]=' ';
	}
       copy_string_to_buffer("Mailbox driver stats\n=======================================\n\nCH  Queued  Cap  sent   received\n\0",&current_postition);

    for(int i=0;i<CHANNELS_NUM;i++){    //actual contents start
        char string[15];

	spin_lock_irqsave(&channels[i].lock, flags);
        ch_stats = channels[i];
	spin_unlock_irqrestore(&channels[i].lock, flags);
        snprintf(string,10,"%d     ",i);
        copy_string_to_buffer(string,&current_postition);
        snprintf(string,10,"%d      ",ch_stats.count);
        copy_string_to_buffer(string,&current_postition);
        snprintf(string,10,"%d    ",ch_stats.capacity);
        copy_string_to_buffer(string,&current_postition);
        snprintf(string,10,"%lu        ",ch_stats.sent);
        copy_string_to_buffer(string,&current_postition);
        snprintf(string,10,"%lu\n",ch_stats.received);                
        copy_string_to_buffer(string,&current_postition);
    
    } //actual contents end

    procfs_buffer_size=current_postition;

    return;
}

//called every time the proc file is opened
 ssize_t procfs_read(struct file *file, char __user *buffer, size_t length, loff_t *offset)  
{   
    	
    set_proc_buffer_contents(); 
    if (*offset || procfs_buffer_size == 0) {   
        pr_debug("procfs_read: END\n ");
 
        *offset = 0;  
        return 0;  
    }  
    procfs_buffer_size = min(procfs_buffer_size, length);  
    if (copy_to_user(buffer, procfs_buffer, procfs_buffer_size))//puts proc buffer contents in proc file
 
        return -EFAULT;  
    *offset += procfs_buffer_size;  

 
    pr_debug("procfs_read: read %lu bytes\n", procfs_buffer_size);  
    return procfs_buffer_size; 
 
}  

ssize_t procfs_write(struct file *file, const char __user *buffer, size_t len, loff_t *off)   
{  
    procfs_buffer_size = min(PROCFS_MAX_SIZE, len);
 
    if (copy_from_user(procfs_buffer, buffer, procfs_buffer_size))//sets buffer contents to contents of proc file  
        return -EFAULT;
 
    *off += procfs_buffer_size;  
  
    pr_debug("procfs_write: write %lu bytes\n", procfs_buffer_size);
 
    return procfs_buffer_size;  
}  
