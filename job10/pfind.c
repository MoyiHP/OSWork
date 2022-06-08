#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

#define NR_WORKER 2
#define DEBUG 0

/* ������������� */
/* ���� */
typedef struct Task 
{
	int is_end;												/* ��������, ==1 ���߳̽��� */
	char path[128];											/* ����·�� */
	char string[128];										/* Ŀ���ַ��� */
} Task ;
/* ������� */
#define QUEUE_SIZE 8										/* ��������� 8 - 1 = 7 */
typedef struct TaskQueue
{
	Task tasks[QUEUE_SIZE];
	int rear;
	int front;
} TaskQueue;
// 1. ��ʼ��
void taskQueue_init(TaskQueue* tq)
{
	if (tq != NULL)
	{
		tq->front = 0;
		tq->rear = 0;
	}
}
// 2. �Ƿ�Ϊ��
int taskQueue_isEmpty(TaskQueue* tq)
{
	if (tq != NULL)
	{
		return tq->front == tq->rear;
	}
	else
	{
		return 0;
	}
}
// 3. �Ƿ�Ϊ��
int taskQueue_isFull(TaskQueue* tq)
{
	if (tq != NULL)
	{
		return (tq->rear + 1) % QUEUE_SIZE == tq->front ;
	}
	else
	{
		return 0;
	}
}
// 4. ȡ������ͷ
void taskQueue_pop(TaskQueue* tq, Task* task)
{

	if (tq != NULL && task != NULL)
	{
		task->is_end = tq->tasks[tq->front].is_end;
		strcpy(task->path, tq->tasks[tq->front].path);
		strcpy(task->string, tq->tasks[tq->front].string);

		if (DEBUG)
		{
			printf("[DEBUG] pop task:\n");
			printf("id : %d\n", tq->front);
			printf("is end = %d\n", task->is_end);
			printf("path = %s\n", task->path);
			printf("string = %s\n\n", task->string);
		}
	}
	if (tq != NULL)
	{
		tq->front = (tq->front + 1) % QUEUE_SIZE;
	}
}
// 5. �����βд������
void taskQueue_push(TaskQueue* tq, Task* task)
{
	if (tq != NULL && task != NULL)
	{
		tq->tasks[tq->rear].is_end = task->is_end;
		strcpy(tq->tasks[tq->rear].path, task->path);
		strcpy(tq->tasks[tq->rear].string, task->string);
		if (DEBUG)
		{
			printf("[DEBUG] push task:\n");
			printf("id : %d\n", tq->rear);
			printf("is end = %d\n", task->is_end);
			printf("path = %s\n", task->path);
			printf("string = %s\n\n", task->string);
		}
		tq->rear = (tq->rear + 1) % QUEUE_SIZE;
	}
}
// 6. ��ʾ����������
void taskQueue_dump(TaskQueue* tq)
{
	if (tq == NULL)
	{
		return;
	}

	printf("==== task queue ====\n");
	int i = tq->front;
	for (i; i != tq->rear; i = (i + 1) % QUEUE_SIZE)
	{
		printf("id : %d\n", i);
		printf("is end = %d\n", tq->tasks[i].is_end);
		printf("path = %s\n", tq->tasks[i].path);
		printf("string = %s\n", tq->tasks[i].string);
	}
	printf("====================\n");
}

// �������
TaskQueue tq;
// ������ �� ״̬��
pthread_mutex_t mutex_tq;										/* TQ �� �ٽ���Դ */
pthread_cond_t cond_empty_tq, cond_valid_tq;					/* TQ �п�λ TQ ����Ч���� */

// ���ܺ���
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
		if (entry->d_type == DT_REG)							/* ������ļ� �������� */
		{
			char tmp_path[128];
			strcpy(tmp_path, path);

			Task task;
			task.is_end = 0;
			strcpy(task.path, strcat(strcat(path, "/"), entry->d_name));
			strcpy(task.string, target);

			pthread_mutex_lock(&mutex_tq);
			while (taskQueue_isFull(&tq))
				pthread_cond_wait(&cond_empty_tq, &mutex_tq);

			taskQueue_push(&tq, &task);

			pthread_cond_signal(&cond_valid_tq);
			pthread_mutex_unlock(&mutex_tq);

			strcpy(path, tmp_path);
		}
	}
	closedir(dir);
}

/* 
FUNC : worker_entry
	 : �ӳ������
		- ��һ��ѭ����
		- ����������У���ȡһ������ȥִ��
		- ����ȡ��һ�����������(is_end Ϊ��)��ѭ������
*/
void* worker_entry(void* arg)
{
	Task task;
	while (1) 
	{
		// ����������л�ȡһ������ task;
		pthread_mutex_lock(&mutex_tq);							/* LOCK mutex_tq */
		while (taskQueue_isEmpty(&tq))
		{
			pthread_cond_wait(&cond_valid_tq, &mutex_tq);
		}
		taskQueue_pop(&tq, &task);
		pthread_cond_signal(&cond_empty_tq);
		pthread_mutex_unlock(&mutex_tq);
		// ִ�и����� task
		if (task.is_end)
			break;
		else
		{
			find_file(task.path, task.string);
		}
	}
}

int main(int argc, char* argv[])
{
	if (argc != 3) 
	{
		puts("Usage: sfind file string");
		return 0;
	}

	char* path = argv[1];										/* Ŀ��·�� */
	char* string = argv[2];										/* Ŀ���ַ��� */

	struct stat info;											/* ��ȡ��ʼ���ļ���Ϣstat */
	stat(path, &info);

	if (S_ISDIR(info.st_mode))									/* �����ʼ���ļ�ΪDIR,���ö��̴߳��� */
	{
		// 1. ����һ���������;
		taskQueue_init(&tq);
		pthread_mutex_init(&mutex_tq, NULL);
		pthread_cond_init(&cond_empty_tq, NULL);
		pthread_cond_init(&cond_valid_tq, NULL);
		// 2. ���� NR_WORKER �����߳�;						
		pthread_t worker_tid[NR_WORKER];
		int i = 0;
		for (i = 0; i <= NR_WORKER - 1; i++)
		{
			pthread_create(&worker_tid[i], NULL, worker_entry, NULL);
		}
		// 3. ���̹߳��� ��Ŀ¼ path ���еݹ���� ��Ҷ�ӽڵ��·�����뵽���������
		find_dir(path, string);
		// 4. ���� WORER_NUMBER ����������
		Task endtask;
		endtask.is_end = 1;
		endtask.path[0] = '\0';
		endtask.string[0] = '\0';
		for (i = 0; i <= NR_WORKER - 1; i++)
		{
			pthread_mutex_lock(&mutex_tq);
			while (taskQueue_isFull(&tq))
				pthread_cond_wait(&cond_empty_tq, &mutex_tq);

			taskQueue_push(&tq, &endtask);

			pthread_cond_signal(&cond_valid_tq);
			pthread_mutex_unlock(&mutex_tq);
		}
		// 5. �ȴ����е����߳̽���;
		for (i = 0; i <= NR_WORKER - 1; i++)
		{
			pthread_join(worker_tid[i], NULL);
		}
	}
	else if (S_ISREG(info.st_mode))
	{
		find_file(path, string);								/* ����,����find_file */
	}

	return 0;
}