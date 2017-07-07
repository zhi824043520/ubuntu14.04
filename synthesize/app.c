#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>

#include "cmd.h"

#define PATHDIR "/dev/myleds"

int main(int argc, const char *argv[])
{
	int fd = -1;
	int ret = -1;
	char buf[20] = {0};

	fd = open(PATHDIR, O_RDWR|O_NONBLOCK);
	if (fd < 0) {
		perror("open");
		return -1;
	}

while (1) {
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
	close(fd);

	return 0;
}
