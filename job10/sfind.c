#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

/*
FUNC : find_file
	 : ��ָ���ļ�path�� ���� target
PARAM :
	@path : ָ���ļ�·��
	@target : Ŀ�����
*/
void find_file(char* path, char* target)
{
	FILE* file = fopen(path, "r");								/* ���ļ� */
	
	char line[256];												/* ���ж�ȡ�ļ����������(strstr)target��� */
	while (fgets(line, sizeof(line), file)) {
		if (strstr(line, target))
			printf("%s: %s", path, line);
	}

	fclose(file);
}

/*
FUNC : find_dir
	 : ��ָ���ļ���·����,����target
PARAM :
	@path : ָ���ļ���·��
	@target : Ŀ�����
*/
void find_dir(char* path, char* target)
{
	DIR* dir = opendir(path);									/* ���ļ��� */
	
	struct dirent* entry;										/* ���η����ļ����������ļ� ����. ..  */
	while (entry = readdir(dir)) {
		if (strcmp(entry->d_name, ".") == 0)
			continue;
		if (strcmp(entry->d_name, "..") == 0)
			continue;
		if (entry->d_type == DT_DIR)							/* ����Ծ����ļ��� �ݹ���� find_dir */
		{
			char nextpath[128];
			strcpy(nextpath, path);
			strcat(strcat(nextpath, "/"), entry->d_name);

			find_dir(nextpath, target);
		}
		if (entry->d_type == DT_REG)							/* ������ļ� ���� find_file */
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

	char* path = argv[1];									/* path argv1 ���ҵ���ʼ�� �ļ������ƻ����ļ����� */
	char* string = argv[2];									/* string argv2 ���ҵ�Ŀ����� */

	struct stat info;										/* ��ȡ��ʼ���ļ���Ϣstat */
	stat(path, &info);
	
	if (S_ISDIR(info.st_mode))								/* �����ʼ���ļ�ΪDIR,����find_dir */
	{
		find_dir(path, string);
	}
	else if(S_ISREG(info.st_mode))
	{
		find_file(path, string);							/* ����,����find_file */
	}
		
	return 0;
}