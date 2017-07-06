#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
	int fd = 0;
	int ret = 0;
	int val = 0;
	char buf[128];
	
	struct pollfd pollfd[2] = {
		{.fd = open("/dev/key", O_RDWR),
		 .events = POLLIN},
		 
		{.fd = 0,
		 .events = POLLIN},
	};
	

	
	while (1) {
		ret = poll(pollfd, 2, -1);
		if (ret < 0) {
			if (pollfd[0].revents == POLLIN) {
				read(pollfd[0].fd, &val, sizeof(val));
				
				if (val == 1) {
					printf("key is down.\n");
				} else {
					printf("key is up.\n");
				}
			}
			
			if (pollfd[1].revents == POLLIN) {
				fgets(buf, 128, stdin);
				
				printf("input is: %s.\n", buf);
			}
		}
	}
	
#if 0
	fd = open("/dev/key", O_RDWR|O_NONBLOCK);
	//fd = open("/dev/key", O_RDWR);
	if (fd < 0) {
		perror("open");
	}

	while (1) {
		ret = read(fd, &buf, sizeof(buf));
		if (ret < 0) {
			// printf("error.\n");
			perror("open");
			return -1;
		}
		
		printf("hight %d.\n", buf);
	}
#endif
	
	return 0;
}