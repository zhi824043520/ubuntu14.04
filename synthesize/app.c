#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "cmd.h"

#define LED_PATHDIR "/dev/myleds"
#define KEY0_PATHDIR "/dev/key0"

static int fd_k = 0;
static int conut = 0;

void handler(int signr)
{
	int temp = 0;
	
	read(fd_k, &temp, sizeof(temp));
	
	if (temp == 1) {
		conut++;
		if (conut == 2)
			conut = 0;
		
		printf("key is down.\n");
	}
}

int main(int argc, char *argv[])
{
	int ret = 0;
	int buf = 0;
	int fd = -1;
	
	fd_k = open(KEY0_PATHDIR, O_RDWR);
	if (fd_k < 0) {
		perror("open");
	}
	
	fd = open(LED_PATHDIR, O_RDWR);
	if (fd < 0) {
		perror("open");
		return -1;
	}
	
	ret = fcntl(fd_k, F_GETFL);
	ret |= FASYNC;
	ret = fcntl(fd_k, F_SETFL, ret);
	
	fcntl(fd_k, F_SETOWN, getpid());

    signal(SIGIO, handler);
	
	while (1) {
		while (conut == 0) {
			ioctl(fd, LED1, 1);
			usleep(200000);
			ioctl(fd, LED2, 1);
			usleep(200000);
			ioctl(fd, LED3, 1);
			usleep(200000);
			ioctl(fd, LED4, 1);
			usleep(200000);
			
			ioctl(fd, LED1, 0);
			usleep(200000);
			ioctl(fd, LED2, 0);
			usleep(200000);
			ioctl(fd, LED3, 0);
			usleep(200000);
			ioctl(fd, LED4, 0);
			usleep(200000);
		}
		
		while (conut == 1) {
			ioctl(fd, LED1, 1);
			usleep(200000);
			ioctl(fd, LED1, 0);
			usleep(200000);
			
			ioctl(fd, LED2, 1);
			usleep(200000);
			ioctl(fd, LED2, 0);
			usleep(200000);
			
			ioctl(fd, LED3, 1);
			usleep(200000);
			ioctl(fd, LED3, 0);
			usleep(200000);
			
			ioctl(fd, LED4, 1);
			usleep(200000);
			ioctl(fd, LED4, 0);
			usleep(200000);
		}
	}
	
	return 0;
}

