#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

/*
FUNC : find_file
	 : 在指定文件path下 查找 target
PARAM :
	@path : 指定文件路径
	@target : 目标句子
*/
void find_file(char* path, char* target)
{
	FILE* file = fopen(path, "r");								/* 打开文件 */
	
	char line[256];												/* 按行读取文件，如果包含(strstr)target输出 */
	while (fgets(line, sizeof(line), file)) {
		if (strstr(line, target))
			printf("%s: %s", path, line);
	}

	fclose(file);
}

/*
FUNC : find_dir
	 : 在指定文件夹路径下,查找target
PARAM :
	@path : 指定文件夹路径
	@target : 目标句子
*/
void find_dir(char* path, char* target)
{
	DIR* dir = opendir(path);									/* 打开文件夹 */
	
	struct dirent* entry;										/* 依次访问文件夹下所有文件 忽略. ..  */
	while (entry = readdir(dir)) {
		if (strcmp(entry->d_name, ".") == 0)
			continue;
		if (strcmp(entry->d_name, "..") == 0)
			continue;
		if (entry->d_type == DT_DIR)							/* 如果仍旧是文件夹 递归调用 find_dir */
		{
			char nextpath[128];
			strcpy(nextpath, path);
			strcat(strcat(nextpath, "/"), entry->d_name);

			find_dir(nextpath, target);
		}
		if (entry->d_type == DT_REG)							/* 如果是文件 调用 find_file */
		{
			char tmp_path[128];
			strcpy(tmp_path, path);

			find_file(strcat(strcat(tmp_path, "/"), entry->d_name), target);

			strcpy(path, tmp_path);
		}
	}
	closedir(dir);
}

int main(int argc, char* argv[])
{
	if (argc != 3) {
		puts("Usage: sfind file string");
		return 0;
	}

	char* path = argv[1];									/* path argv1 查找的起始点 文件夹名称或者文件名称 */
	char* string = argv[2];									/* string argv2 查找的目标句子 */

	struct stat info;										/* 获取起始点文件信息stat */
	stat(path, &info);
	
	if (S_ISDIR(info.st_mode))								/* 如果起始点文件为DIR,调用find_dir */
	{
		find_dir(path, string);
	}
	else if(S_ISREG(info.st_mode))
	{
		find_file(path, string);							/* 否则,调用find_file */
	}
		
	return 0;
}