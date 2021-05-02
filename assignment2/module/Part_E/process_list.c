#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_LENGTH 1000
char buffer[BUFFER_LENGTH];


int main() {
	int fd;
	fd = open("/dev/process_list", O_RDONLY);

	if(fd < 0) {
		perror("FAILURE: Process List Device Failed to Open");
		return errno;
	}

	int bytes_read;
	while(bytes_read = read(fd, buffer, strlen(buffer)) > 0) {
		printf("%s\n", buffer);
		memset(buffer, 0, sizeof(char)* BUFFER_LENGTH);
	}

	if(bytes_read < 0) {
			perror("FAILURE: Failed to Read from Process List Device");
			return errno;
	}

	close(fd);
	return 0; 
}