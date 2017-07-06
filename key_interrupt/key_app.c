#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
	int fd = 0;
	int ret = 0;
	int buf = 0;
	
	//fd = open("/dev/key", O_RDWR|O_NONBLOCK);
	fd = open("/dev/key", O_RDWR);
	if (fd < 0) {
		perror("open");
	}
	
	while (1) {
		ret = read(fd, &buf, sizeof(buf));
		if (ret < 0) {
			perror("open");
			return -1;
		}
		
		if (buf == 0) {
			printf("This is key down.\n");
		} else {
			printf("This is key up.\n");
		}
	}
	
	return 0;
}