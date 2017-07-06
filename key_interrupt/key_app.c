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
		if (ret > 0) {
			printf("revents: %d.\n", pollfd[0].revents);
			printf("POLLIN: %d.\n", POLLIN);
			if (pollfd[0].revents == POLLIN) {
				printf("start\n");
				read(pollfd[0].fd, &val, sizeof(val));
				printf("end\n");
				
				if (val == 1) {
					printf("key is down.\n");
				} else if (val == 0) {
					printf("key is up.\n");
				}
			}
			
			if (pollfd[1].revents == POLLIN) {
				fgets(buf, 128, stdin);
				
				printf("input is: %s.", buf);
			}
		}
	}
	
	return 0;
}