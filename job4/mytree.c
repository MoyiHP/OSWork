#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>

#define MAX_PATH 20

void myls(char *path,int lev)
{
	DIR *dir = opendir(path);
	if (dir == NULL)	return;
	else
	{
		while(1)
		{
			struct dirent *de = readdir(dir);
			if(de == NULL) break;
			if(strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0 )	continue;
			
			int i = 0;
			for(i = 1; i <= lev; i++)
			{
				printf("-");
			}
			switch(de->d_type)
			{
				case DT_DIR:
				{
					printf("DIR:%s\n", de->d_name);
					myls(strcat(strcat(path,"/"),de->d_name), lev+1);
					break;
				}
				case DT_REG:
				{
					printf("REG:%s\n", de->d_name);
					break;
				}
				default:
					break;
			}
		}
	}
}

int main(int argc, char *argv[])
{
	if(argc == 1)
	{
		myls(getcwd(NULL,MAX_PATH),0);
	}
	else
	{
		myls(argv[1],0);
	}
	return 0;
}
