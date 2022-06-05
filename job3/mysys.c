#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void mysys(char *command)
{
	if(command == NULL) return;
	pid_t pid = fork();
	if(pid == 0)
	{
		execl("/bin/sh", "sh", "-c", command, NULL);
	}	
	wait(NULL);
}

int main()
{
	printf("-----------------------\n");
	mysys("echo HELLO WORLD");
	printf("-----------------------\n");
	mysys("ls /");
	printf("-----------------------\n");
	return 0;
}
