#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>




void operation_select(int channel){
    while (1)
    { 
    int input=0;
        printf("enter desired operation\n1: read\n2: write");
        scanf("%d",&input);
        if((input>0)&&(input<3)){
            
        }
        printf("invalid input");
    }
}

int mailbox_select(){
    while(1){
        int input=0;
        printf("enter desired channel\n");
        for(int i;i<5;i++){
            printf("%d: mailbox%d\n",i,i);
        }
        scanf("%d",&input);
        if((input>=0)&&(input<5)){
            return input;
        }
        printf("invalid input\n");
    }
}

int main(int argc, char *argv[]){
    int *mailbox0, *mailbox1, *mailbox2, mailbox3, *mailbox4;//not initialised yet

    mailbox3=open("/dev/mailbox0",O_RDWR);
    char word[]="word\n\0a";
    char word2[sizeof word];
    write(mailbox3,word,sizeof word);
    read(mailbox3,word2,sizeof word2);
    printf("%s",word2);
    close(mailbox3);
    return 0;

    // int driver;
    // driver=open("/dev/mailbox0",O_RDWR);
    // char word[]="word\n";
    // char word2[14];
    // write(driver,word,sizeof word);
    // read(driver,word2,sizeof word2);
    // printf("%s",word2);
    // close(driver);
}