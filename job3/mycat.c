#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	char *pathname = argv[1];	// argv0 is the command, argv1 is the first param - the pathname
	// open file
	int fd = open(pathname, O_RDONLY);
	if(fd == -1)
	{
		printf("can not open the file %s \n", pathname);
		return 0;
	}	
	
	// read and cout file
	char readBuff[80] = {'\0'};
	int count = 0 ;
	while(1)
	{
		count = read(fd, readBuff, sizeof(readBuff));
		if(count == -1 || count == 0)	
		{
			break;
		}
		else
		{
			readBuff[count] = '\0';
			// use fd = 1. means write on screen
			write(1, readBuff, count);
		}
	}

	// close file
	close(fd);

	return 0;
}
