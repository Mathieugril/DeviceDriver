#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>

/* ioctl commands — must match mailbox.h exactly */
#define MB_FLUSH      _IO('m', 1)
#define MB_FLUSH_ALL  _IO('m', 2)
#define MB_GET_COUNT  _IOR('m', 3, int)
#define MB_SET_MAX    _IOW('m', 4, int)

#define NUM_CHANNELS  5
#define NUM_MESSAGES  10   /* how many messages each producer sends */

/* ------------------------------------------------------------------ */
/* PRODUCER: opens a channel, writes NUM_MESSAGES messages into it     */
/* ------------------------------------------------------------------ */
void producer_process(int channel) {
    char path[32];
    snprintf(path, sizeof(path), "/dev/mailbox%d", channel);

    int fd = open(path, O_WRONLY);
    if (fd < 0) {
        perror("producer: open failed");
        exit(1);
    }

    for (int i = 0; i < NUM_MESSAGES; i++) {
        char msg[128];
        snprintf(msg, sizeof(msg), "[CH%d] message %d from PID %d", channel, i, getpid());

        ssize_t ret = write(fd, msg, strlen(msg));
        if (ret < 0) {
            if (errno == EINTR) { i--; continue; }   /* retry on signal */
            perror("producer: write failed");
            break;
        }
        printf("[PRODUCER ch%d] sent: \"%s\"\n", channel, msg);
        fflush(stdout);
        usleep(200000);  /* 200ms between messages */
    }

    close(fd);
    printf("[PRODUCER ch%d] done, PID %d exiting\n", channel, getpid());
    exit(0);
}

/* ------------------------------------------------------------------ */
/* CONSUMER: opens a channel, reads messages until producer is done    */
/* ------------------------------------------------------------------ */
void consumer_process(int channel, int expected_msgs) {
    char path[32];
    snprintf(path, sizeof(path), "/dev/mailbox%d", channel);

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("consumer: open failed");
        exit(1);
    }

    for (int i = 0; i < expected_msgs; i++) {
        char buf[256];
        memset(buf, 0, sizeof(buf));

        ssize_t ret = read(fd, buf, sizeof(buf) - 1);
        if (ret < 0) {
            if (errno == EINTR) { i--; continue; }   /* retry on signal */
            perror("consumer: read failed");
            break;
        }
        buf[ret] = '\0';
        printf("[CONSUMER ch%d] received: \"%s\"\n", channel, buf);
        fflush(stdout);
    }

    close(fd);
    printf("[CONSUMER ch%d] done, PID %d exiting\n", channel, getpid());
    exit(0);
}

/* ------------------------------------------------------------------ */
/* STATS: prints /proc/mailbox to the terminal                         */
/* ------------------------------------------------------------------ */
void show_proc_stats(void) {
    FILE *f = fopen("/proc/mailbox", "r");
    if (!f) {
        perror("show_proc_stats: cannot open /proc/mailbox");
        return;
    }
    printf("\n========== /proc/mailbox ==========\n");
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        printf("%s", line);
    }
    printf("====================================\n\n");
    fflush(stdout);
    fclose(f);
}

/* ------------------------------------------------------------------ */
/* IOCTL DEMO: shows flush and get_count working from userspace        */
/* ------------------------------------------------------------------ */
void demo_ioctl(int channel) {
    char path[32];
    snprintf(path, sizeof(path), "/dev/mailbox%d", channel);

    int fd = open(path, O_RDWR);
    if (fd < 0) {
        perror("demo_ioctl: open failed");
        return;
    }

    /* get current message count */
    int count = 0;
    if (ioctl(fd, MB_GET_COUNT, &count) < 0) {
        perror("demo_ioctl: MB_GET_COUNT failed");
    } else {
        printf("[IOCTL] channel %d has %d message(s) queued\n", channel, count);
    }

    /* flush the channel */
    if (ioctl(fd, MB_FLUSH, 0) < 0) {
        perror("demo_ioctl: MB_FLUSH failed");
    } else {
        printf("[IOCTL] channel %d flushed\n", channel);
    }

    close(fd);
}

/* ------------------------------------------------------------------ */
/* BLOCKING DEMO: fills a channel to capacity to prove write blocks   */
/* ------------------------------------------------------------------ */
void blocking_demo(void) {
    printf("\n--- BLOCKING DEMO: filling channel 4 to capacity ---\n");

    int fd = open("/dev/mailbox4", O_WRONLY);
    if (fd < 0) {
        perror("blocking_demo: open failed");
        return;
    }

    /* Write 16 messages (FIFO_LIMIT) as fast as possible */
    for (int i = 0; i < 16; i++) {
        char msg[64];
        snprintf(msg, sizeof(msg), "block_test_msg_%d", i);
        write(fd, msg, strlen(msg));
        printf("[BLOCKING DEMO] wrote message %d to ch4\n", i);
        fflush(stdout);
    }

    printf("[BLOCKING DEMO] channel 4 is now full. Next write will BLOCK...\n");
    fflush(stdout);

    /* This write will block until someone reads from channel 4 */
    char *extra = "this_should_block";
    printf("[BLOCKING DEMO] attempting one more write (should block)...\n");
    fflush(stdout);
    write(fd, extra, strlen(extra));
    printf("[BLOCKING DEMO] unblocked!\n");

    close(fd);
}

/* ------------------------------------------------------------------ */
/* MAIN                                                                */
/* ------------------------------------------------------------------ */
int main(int argc, char *argv[]) {
    printf("============================================\n");
    printf("  Mailbox Driver User-Space Demo\n");
    printf("  PID: %d\n", getpid());
    printf("============================================\n\n");

    /* Show initial state of /proc/mailbox */
    show_proc_stats();

    pid_t pids[NUM_CHANNELS * 2];
    int pid_count = 0;

    /* Fork one producer per channel */
    for (int ch = 0; ch < NUM_CHANNELS; ch++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork failed");
            exit(1);
        }
        if (pid == 0) {
            producer_process(ch);   /* child — never returns */
        }
        pids[pid_count++] = pid;
        printf("[MAIN] spawned producer for channel %d (PID %d)\n", ch, pid);
    }

    /* Small delay so producers have time to write some messages */
    sleep(1);

    /* Fork one consumer per channel */
    for (int ch = 0; ch < NUM_CHANNELS; ch++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork failed");
            exit(1);
        }
        if (pid == 0) {
            consumer_process(ch, NUM_MESSAGES);  /* child — never returns */
        }
        pids[pid_count++] = pid;
        printf("[MAIN] spawned consumer for channel %d (PID %d)\n", ch, pid);
    }

    /* Parent polls /proc/mailbox every second while children run */
    for (int i = 0; i < 5; i++) {
        sleep(1);
        show_proc_stats();
    }

    /* Wait for all children to finish */
    printf("[MAIN] waiting for all children...\n");
    for (int i = 0; i < pid_count; i++) {
        int status;
        waitpid(pids[i], &status, 0);
    }

    /* Final ioctl demo */
    printf("\n--- IOCTL DEMO ---\n");
    for (int ch = 0; ch < NUM_CHANNELS; ch++) {
        demo_ioctl(ch);
    }

    /* Final stats */
    show_proc_stats();

    printf("[MAIN] demo complete.\n");
    return 0;
}
