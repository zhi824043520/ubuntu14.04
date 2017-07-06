#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

int fd = 0;

void handler(int signr)
{
	int temp = 0;
	
	read(fd, &temp, sizeof(temp));
	
	if (temp == 1) {
		printf("key is down.\n");
	} else {
		printf("key is up.\n");
	}
}

int main(int argc, char *argv[])
{
	int ret = 0;
	int buf = 0;
	
	fd = open("/dev/key", O_RDWR);
	if (fd < 0) {
		perror("open");
	}
	
	ret = fcntl(fd, F_GETFL);
	ret |= FASYNC;
	ret = fcntl(fd, F_SETFL, ret);
	
	fcntl(fd, F_SETOWN, getpid());

    signal(SIGIO, handler);
	
	while (1) {
		sleep(1);
		
		printf("This is heartbeat.\n");
	}
	
	return 0;
}