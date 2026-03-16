#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include "mailbox.h"


#define LCD_ADD 0x27
#define LCD_COL 20
#define LCD_ROW 4
#define I2C_BUS "/dev/i2c-1"
#define NUM_CH 4
#define MAC_MSG 16
#define DELAY 3


#define LCD_BACKLIGHT 0x08
#define LCD_ENABLE 0x04	
//show_bargraph(stats,count);


#define LCD_RW 0x02
#define LCD_RS 0x01


int i2c_fd;

/*
typedef struct {
	int channel;
	int queued;
	int capacity;
	unsigned long sent;
	unsigned long received;
} channel_stats_s;
*/

static void i2c_write_byte(unsigned char data) {
	write(i2c_fd, &data, 1);
}


static void lcd_send_nibble(unsigned char nibble, unsigned char mode) {
	unsigned char data = (nibble & 0xF0) | LCD_BACKLIGHT | mode;
	i2c_write_byte(data | LCD_ENABLE);
	usleep(500);
	i2c_write_byte(data & ~LCD_ENABLE);
	usleep(500);
}


static void lcd_send_byte(unsigned char byte, unsigned char mode) {
	lcd_send_nibble(byte & 0xF0, mode);
	lcd_send_nibble((byte << 4) & 0xF0, mode);
}

static void lcd_cmd(unsigned char cmd) {
	lcd_send_byte(cmd, 0x00);
}

static void lcd_char(unsigned char ch) {
	lcd_send_byte(ch, LCD_RS);
}

static void lcd_string(const char *str) {
	while(*str)
		lcd_char(*str++);
}


static void lcd_set_cursor(int row, int col) {
	int row_offsets[] = {0x00, 0x40, 0x14, 0x54};
	lcd_cmd(0x80 | (row_offsets[row] + col));

}

static void lcd_clear(void) {
	lcd_cmd(0x01);
	usleep(2000);
}

static int lcd_init(void) {

	i2c_fd = open(I2C_BUS, O_RDWR);

	if(i2c_fd < 0) {
	perror("Failed to open I2C");
	return -1;
	}

	if(ioctl(i2c_fd, I2C_SLAVE, LCD_ADD) < 0) {
	perror("Failed to set address");
	return -1;
	}


	usleep(50000);
	lcd_send_nibble(0x30, 0x00); usleep(5000);
	lcd_send_nibble(0x30, 0x00); usleep(1000);
 	lcd_send_nibble(0x30, 0x00); usleep(1000); 
	lcd_send_nibble(0x20, 0x00); usleep(1000);


	lcd_cmd(0x28); //
	lcd_cmd(0x0C); // display on cursor off
	lcd_cmd(0x06); // entry mode
	lcd_clear;


	return 0;


}

static int parse_proc(channel_stats_s *stats, int max_channels) {

	FILE *f;
	char line[256];
	int count = 0;

	f = fopen("/proc/mailbox", "r");
	if(!f) {
	perror("Failed to open /proc/mailbox");
	return -1;
	}

	// these are to take up the space needed for the proc header
	fgets(line, sizeof(line), f);
	fgets(line, sizeof(line), f);
	fgets(line, sizeof(line), f);
	fgets(line, sizeof(line), f);
	fgets(line, sizeof(line), f);

	while(fgets(line, sizeof(line), f) && count < max_channels) {
	
		int ch, queued, cap;
		unsigned long sent, received;

		if(sscanf(line, "%d %d %d %lu %lu", &ch, &queued, &cap, &sent, &received) == 5) {
		
			stats[count].channel = ch;
			stats[count].queued = queued;
			stats[count].capacity = cap;
			stats[count].sent = sent;
			stats[count].received = received;
			count++;
		}
	}
	fclose(f);
	return count;

}


static void show_bargraph(channel_stats_s *stats, int count) {
	
	char lines[21];

	lcd_clear();

	for(int i = 0; i < CHANNELS_NUM && i < count; i++) {
		
		int filled = stats[i].queued;
		if(filled > 16) { filled = 16;}

		lines[0] = '0' + stats[i].channel;
		lines[1] = '=';
		lines[2] = '[';

		for(int j = 0; j < 16; j++) {
			lines[3+j] = (j < filled) ? '#' : ' ';
		}

		lines[19] = ']';
		lines[20] = '\0';

		lcd_set_cursor(i, 0);
		lcd_string(lines);

	}



}

//static void show_stats(channel_stats_s *stats, int count, int start) {


//}

int main(void) {

	channel_stats_s stats[CHANNELS_NUM];
	int count;
	int screen = 0;
	int stats_page = 0;

	printf("Starting LCD on %s", I2C_BUS);

	if(lcd_init() < 0){
	return 1;
	}


	printf(" - LCD ready - \n");

	while(1){

	count = parse_proc(stats, CHANNELS_NUM);

	if(count > 0) {
		show_bargraph(stats,count);

		sleep(1);
	}
	}

return 0;
}
