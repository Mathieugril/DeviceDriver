#include "mailbox.h"



//does as says (string.h + strcat unavailable in kernel)
void copy_string_to_buffer(char* string,unsigned long *postion){  
    for(int i=0;;i++){
        //printk("%s  %lu   %d \n",string,*postion,i);
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

// //void set_proc_buffer();

void set_proc_buffer_contents(void){    //generates contents to put in proc file 
    unsigned long current_postition=0;  //keeps track of postion in string. also erases anything already 
    for(int i=0;i<PROCFS_MAX_SIZE;i++){ //initialise everything in strinng to prevent linux from thinking its a proc file
		procfs_buffer[i]=' ';
	}
       copy_string_to_buffer("Mailbox driver stats\n=======================\n\nCH  Queued  Cap  sent   received\n\0",&current_postition);

    for(int i=0;i<CHANNELS_NUM;i++){    //actual contents start
        mb_channel_s channel = channels[i];
        char string[15];
        snprintf(string,10,"%d     ",i);
        copy_string_to_buffer(string,&current_postition);
        snprintf(string,10,"%d      ",channel.count);
        copy_string_to_buffer(string,&current_postition);
        snprintf(string,10,"%d    ",channel.capacity);
        copy_string_to_buffer(string,&current_postition);
        snprintf(string,10,"%d        ",1);
        copy_string_to_buffer(string,&current_postition);
        snprintf(string,10,"%d\n",1);                
        copy_string_to_buffer(string,&current_postition);
    
    } //actual contents end

    procfs_buffer_size=current_postition;

    return;
}

//called every time the proc file is opened
 ssize_t procfs_read(struct file *file, char __user *buffer, size_t length, loff_t *offset)  
{   
    
    
	
    set_proc_buffer_contents(); //does as says


    if (*offset || procfs_buffer_size == 0) { //prints when buffer is empty  
        pr_debug("procfs_read: END\n ");
 
        *offset = 0;  
        return 0;  
    }  
    procfs_buffer_size = min(procfs_buffer_size, length);   //makes sure the read size is not exceeded?
    //printk("%s %lu",procfs_buffer,procfs_buffer_size);
    if (copy_to_user(buffer, procfs_buffer, procfs_buffer_size))//puts proc buffer contents in proc file
 
        return -EFAULT;  
    *offset += procfs_buffer_size;  //i dont actually kn what the buffer size is.

 
    pr_debug("procfs_read: read %lu bytes\n", procfs_buffer_size);  
    return procfs_buffer_size; 
 
}  

//not really used, can change proc buffers contents, but thats redone every read call. probably called when proc file
ssize_t procfs_write(struct file *file, const char __user *buffer, size_t len, loff_t *off)   
{  
    procfs_buffer_size = min(PROCFS_MAX_SIZE, len);
 
    if (copy_from_user(procfs_buffer, buffer, procfs_buffer_size))//sets buffer contents to contents of proc file  
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
