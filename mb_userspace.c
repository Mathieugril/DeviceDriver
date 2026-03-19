#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <errno.h>
#include <ctype.h>


#define MB_FLUSH      _IO('m', 1)
#define MB_FLUSH_ALL  _IO('m', 2)
#define MB_GET_COUNT  _IOR('m', 3, int)
#define MB_SET_MAX    _IOW('m', 4, int)

#define NUM_CHANNELS  4

void print_menu(void) {
    printf("\n╔══════════════════════════════════════╗\n");
    printf("║       MAILBOX DRIVER DEMO MENU       ║\n");
    printf("╠══════════════════════════════════════╣\n");
    printf("║  1. Write a message to a channel     ║\n");
    printf("║  2. Read a message from a channel    ║\n");
    printf("║  3. Read all messages from a channel ║\n");
    printf("║  4. Check message count (ioctl)      ║\n");
    printf("║  5. Flush a channel (ioctl)          ║\n");
    printf("║  6. Flush ALL channels (ioctl)       ║\n");
    printf("║  7. Show /proc/mailbox stats         ║\n");
    printf("║  8. Run multi-process demo           ║\n");
    printf("║  9. Exit                             ║\n");
    printf("╚══════════════════════════════════════╝\n");
    printf("Enter choice: ");
}

//prevents scanf from causing infinite loop (i hope)
void scan_loop_fix(){
    char c='0';
    do{
        c=getchar();
    }
    while(!isdigit(c));
    ungetc(c,stdin);
    printf("invalid input");
    return;
}


int pick_channel(void) {
    int ch;
    while (1) {
        printf("Enter channel (0-%d): ", NUM_CHANNELS - 1);
        if(scanf("%d", &ch)==0)scan_loop_fix();
        while (getchar()!='\n');        
        if (ch >= 0 && ch < NUM_CHANNELS) return ch;
        printf("Invalid channel. Try again.\n");
    }
}

int open_channel(int channel, int flags) {
    char path[32];
    snprintf(path, sizeof(path), "/dev/mailbox%d", channel);
    int fd = open(path, flags);
    if (fd < 0) {
        perror("Failed to open channel");
    }
    return fd;
}

void do_write(void) {
    int ch = pick_channel();
    int fd = open_channel(ch, O_WRONLY);
    if (fd < 0) return;

    char msg[256];
    printf("Enter message to send: ");
    getchar(); // flush newline
    fgets(msg, sizeof(msg), stdin);
    msg[strcspn(msg, "\n")] = '\0'; // remove trailing newline

    ssize_t ret = write(fd, msg, strlen(msg));
    if (ret < 0) {
        perror("Write failed");
    } else {
        printf("[OK] Sent \"%s\" to channel %d\n", msg, ch);
    }
    close(fd);
}

void do_read(void) {
    int ch = pick_channel();
    int fd = open_channel(ch, O_RDONLY);
    if (fd < 0) return;

    int count = 0;
    if (ioctl(fd, MB_GET_COUNT, &count) < 0) {
        perror("MB_GET_COUNT failed");
    }
    else if(count==0){
        printf("channel is empty\n");
	close(fd);
        return;
    }

    char buf[256];
    memset(buf, 0, sizeof(buf));

    printf("Waiting for message on channel %d (will block if empty)...\n", ch);
    ssize_t ret = read(fd, buf, sizeof(buf) - 1);
    if (ret < 0) {
        perror("Read failed");
    } else {
        buf[ret] = '\0';
        printf("Received from channel %d: \"%s\"\n", ch, buf);
    }
    close(fd);
    return;
}


//read all messages in given channels
void do_read_all(void) {

    int ch = pick_channel();
    int fd = open_channel(ch, O_RDONLY);
    if (fd < 0) return;
    int count = 0;

    while(1){

        if (ioctl(fd, MB_GET_COUNT, &count) < 0) {
            perror("MB_GET_COUNT failed");
        }
        else if(count==0){
            printf("channel is empty\n");
            return;
        }

        char buf[256];
        memset(buf, 0, sizeof(buf));

	ssize_t ret = read(fd, buf, sizeof(buf) - 1);
        if (ret < 0) {
            perror("Read failed");
        } else {
            buf[ret] = '\0';
            printf("Received from channel %d: \"%s\"\n", ch, buf);
        }
    }
    close(fd);
}


void do_get_count(void) {
    int ch = pick_channel();
    int fd = open_channel(ch, O_RDWR);
    if (fd < 0) return;

    int count = 0;
    if (ioctl(fd, MB_GET_COUNT, &count) < 0) {
        perror("MB_GET_COUNT failed");
    } else {
        printf("Channel %d has %d message(s) queued\n", ch, count);
    }
    close(fd);
}


void do_flush(void) {
    int ch = pick_channel();
    int fd = open_channel(ch, O_RDWR);
    if (fd < 0) return;

    if (ioctl(fd, MB_FLUSH, 0) < 0) {
        perror("MB_FLUSH failed");
    } else {
        printf("Channel %d flushed\n", ch);
    }
    close(fd);
}


void do_flush_all(void) {
    int fd = open_channel(0, O_RDWR);
    if (fd < 0) return;

    if (ioctl(fd, MB_FLUSH_ALL, 0) < 0) {
        perror("MB_FLUSH_ALL failed");
    } else {
        printf("All channels flushed\n");
    }
    close(fd);
}


void do_show_proc(void) {
    FILE *f = fopen("/proc/mailbox", "r");
    if (!f) {
        perror("Cannot open /proc/mailbox");
        return;
    }
    printf("\n========== /proc/mailbox ==========\n");
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        printf("%s", line);
    }
    printf("====================================\n");
    fclose(f);
}


void producer_process(int channel) {
    int fd = open_channel(channel, O_WRONLY);
    if (fd < 0) exit(1);

    for (int i = 0; i < 5; i++) {
        char msg[128];
        snprintf(msg, sizeof(msg), "[CH%d] msg_%d from PID %d", channel, i, getpid());
        write(fd, msg, strlen(msg));
        printf("[PRODUCER ch%d] sent: \"%s\"\n", channel, msg);
        fflush(stdout);
        usleep(300000); // 300ms between messages
    }
    close(fd);
    exit(0);
}

void consumer_process(int channel) {
    int fd = open_channel(channel, O_RDONLY);
    if (fd < 0) exit(1);

    for (int i = 0; i < 5; i++) {
        char buf[256];
        memset(buf, 0, sizeof(buf));
        ssize_t ret = read(fd, buf, sizeof(buf) - 1);
        if (ret > 0) {
            buf[ret] = '\0';
            printf("[CONSUMER ch%d] got: \"%s\"\n", channel, buf);
            fflush(stdout);
        }
    }
    close(fd);
    exit(0);
}

void do_multiprocess_demo(void) {
    printf("\n--- Starting multi-process demo across all %d channels ---\n", NUM_CHANNELS);
    printf("Forking 1 producer + 1 consumer per channel...\n\n");

    pid_t pids[NUM_CHANNELS * 2];
    int count = 0;

    for (int ch = 0; ch < NUM_CHANNELS; ch++) {
        pid_t pid = fork();
        if (pid == 0) producer_process(ch);
        pids[count++] = pid;
    }

    sleep(1); // let producers write some messages first

    // fork consumers
    for (int ch = 0; ch < NUM_CHANNELS; ch++) {
        pid_t pid = fork();
        if (pid == 0) consumer_process(ch);
        pids[count++] = pid;
    }

    // show stats while demo runs
    for (int i = 0; i < 3; i++) {
        sleep(1);
        do_show_proc();
    }

    // wait for all children
    for (int i = 0; i < count; i++) {
        int status;
        waitpid(pids[i], &status, 0);
    }

    printf("\n--- Multi-process demo complete ---\n");
}


int main(void) {
    printf("\nMailbox Driver - Interactive Demo\n");
    printf("==================================\n");

    int choice;
    while (1) {
        print_menu();
        if(scanf("%d", &choice)==0)scan_loop_fix();
        while (getchar()!='\n');
        switch (choice) {
            case 1: do_write();              break;
            case 2: do_read();               break;
            case 3: do_read_all();           break;
            case 4: do_get_count();          break;
            case 5: do_flush();              break;
            case 6: do_flush_all();          break;
            case 7: do_show_proc();          break;
            case 8: do_multiprocess_demo();  break;
            case 9:
                printf("Goodbye!\n");
                return 0;
            default:
                printf("Invalid choice. Try again.\n");
        }
    }
}
