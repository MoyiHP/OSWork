#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>

#define MAX_PATH 20

int main(int argc, char *argv[])
{
	// ls cwd
	DIR *dir = NULL;
	if(argc == 1)
	{
		dir = opendir(getcwd(NULL, MAX_PATH));
	}
	else
	{
		dir = opendir(argv[1]);
	}

	while(dir != NULL)
	{
		struct dirent *de = readdir(dir);
		if(de == NULL) break;
		switch(de->d_type)
		{
			case DT_DIR:
			{
				printf("DIR:%s\n", de->d_name);
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

	return 0;
}
