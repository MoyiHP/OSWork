#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

#define NR_WORKER 2
#define DEBUG 0

/* 任务与任务队列 */
/* 任务 */
typedef struct Task 
{
	int is_end;												/* 结束任务, ==1 子线程结束 */
	char path[128];											/* 查找路径 */
	char string[128];										/* 目标字符串 */
} Task ;
/* 任务队列 */
#define QUEUE_SIZE 8										/* 最大任务量 8 - 1 = 7 */
typedef struct TaskQueue
{
	Task tasks[QUEUE_SIZE];
	int rear;
	int front;
} TaskQueue;
// 1. 初始化
void taskQueue_init(TaskQueue* tq)
{
	if (tq != NULL)
	{
		tq->front = 0;
		tq->rear = 0;
	}
}
// 2. 是否为空
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
// 3. 是否为满
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
// 4. 取出队列头
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
// 5. 向队列尾写入数据
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
// 6. 显示队列内数据
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

// 任务队列
TaskQueue tq;
// 互斥量 与 状态量
pthread_mutex_t mutex_tq;										/* TQ 是 临界资源 */
pthread_cond_t cond_empty_tq, cond_valid_tq;					/* TQ 有空位 TQ 有有效数据 */

// 功能函数
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
		if (entry->d_type == DT_REG)							/* 如果是文件 创建任务 */
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
	 : 子程序入口
		- 在一个循环中
		- 从任务队列中，获取一个任务，去执行
		- 当读取到一个特殊的任务(is_end 为真)，循环结束
*/
void* worker_entry(void* arg)
{
	Task task;
	while (1) 
	{
		// 从任务队列中获取一个任务 task;
		pthread_mutex_lock(&mutex_tq);							/* LOCK mutex_tq */
		while (taskQueue_isEmpty(&tq))
		{
			pthread_cond_wait(&cond_valid_tq, &mutex_tq);
		}
		taskQueue_pop(&tq, &task);
		pthread_cond_signal(&cond_empty_tq);
		pthread_mutex_unlock(&mutex_tq);
		// 执行该任务 task
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

	char* path = argv[1];										/* 目标路径 */
	char* string = argv[2];										/* 目标字符串 */

	struct stat info;											/* 获取起始点文件信息stat */
	stat(path, &info);

	if (S_ISDIR(info.st_mode))									/* 如果起始点文件为DIR,启用多线程处理 */
	{
		// 1. 创建一个任务队列;
		taskQueue_init(&tq);
		pthread_mutex_init(&mutex_tq, NULL);
		pthread_cond_init(&cond_empty_tq, NULL);
		pthread_cond_init(&cond_valid_tq, NULL);
		// 2. 创建 NR_WORKER 个子线程;						
		pthread_t worker_tid[NR_WORKER];
		int i = 0;
		for (i = 0; i <= NR_WORKER - 1; i++)
		{
			pthread_create(&worker_tid[i], NULL, worker_entry, NULL);
		}
		// 3. 主线程工作 对目录 path 进行递归遍历 把叶子节点的路径加入到任务队列中
		find_dir(path, string);
		// 4. 创建 WORER_NUMBER 个特殊任务
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
		// 5. 等待所有的子线程结束;
		for (i = 0; i <= NR_WORKER - 1; i++)
		{
			pthread_join(worker_tid[i], NULL);
		}
	}
	else if (S_ISREG(info.st_mode))
	{
		find_file(path, string);								/* 否则,调用find_file */
	}

	return 0;
}