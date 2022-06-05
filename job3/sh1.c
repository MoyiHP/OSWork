/*
 shell frame
 while(1)
 {
 	print > ;
	read line;
	split line into args;
	// buildin command
	if (args[0] == "exit")	// exit can not implement by child proc
		exit(0);
	if (args[0] == "cd")
	// extern command
	// implement through child proc
	pid = fork();
	if pid == 0 then 
		execvp(args[0],args);
	wait(null);
 }
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_PATH 20

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
	char command[50] = {'\0'};
	char tmpCommand[50] = {'\0'};
	char *args[10] = {NULL};
	char delim[] = " \n";
	while(1)
	{
		printf("[%s] <sh1> ", getcwd(NULL, MAX_PATH));
		// read line
		fgets(command,50,stdin);
		// split
		if(args == NULL)
		{
			printf("error commnad!\n");
		}

		int i = 0;
		strcpy(tmpCommand, command);
		char *token = strtok(tmpCommand, delim);
		for(i = 0; i <= 9 && token != NULL; i++ , token = strtok(NULL, delim))
		{
			args[i] = token;
		}
		// build in command
		/// 1. exit
		if(strcmp(args[0],"exit") == 0)
		{
			exit(0);
		}
		/// 2. cd
		else if(strcmp(args[0], "cd") == 0)
		{
			chdir(strcat( strcat(getcwd(NULL, MAX_PATH), "/") , args[1]));
		}
		/// 3. pwd 
		else if(strcmp(args[0], "pwd") == 0)
		{
			printf("%s \n", getcwd(NULL, MAX_PATH));
		}
		/// 4. ls
		else if(strcmp(args[0], "ls") == 0)
		{
			DIR *curdir = opendir(getcwd(NULL, MAX_PATH));
			int count = 0;
			while(1)
			{
				struct dirent *de = readdir(curdir);
				if(de == NULL)	break;
				switch(de->d_type)
				{
					case DT_DIR:
					{
						printf("DIR:");
						printf("%s ", de->d_name);
						break;
					}
					case DT_REG:
					{
						printf("REG:");
						printf("%s ", de->d_name);
						break;
					}
					default:
						break;
				}

				count++;
				if(count == 5)
				{
					count = 0;
					printf("\n");
				}
			}
			printf("\n");

			closedir(curdir);
		}

		// extern cmd
		else
		{
			mysys(command);
		}
	}

	return 0;
}
