#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	char *src_pathname = argv[1];	// argv0 is the command, argv1 is the first param - the src_pathname, argv2 - the dst_pathname
	char *dst_pathname = argv[2];
	// open file
	int src_fd = open(src_pathname, O_RDONLY);
	int dst_fd = open(dst_pathname, O_WRONLY | O_TRUNC | O_CREAT);
	if(src_fd == -1)
	{
		printf("can not open the file %s \n", src_pathname);
		return 0;
	}	
	if(dst_fd == -1)
	{
		printf("can not create the file %s \n", dst_pathname);
		return 0;
	}

	// read and write file
	char buff[80] = {'\0'};
	int count = 0 ;
	while(1)
	{
		count = read(src_fd, buff, sizeof(buff));
		if(count == -1 || count == 0)	
		{
			break;
		}
		else
		{
			write(dst_fd, buff, count);
		}
	}

	// close file
	close(src_fd);
	close(dst_fd);

	return 0;
}
