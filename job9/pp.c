#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

/* 信号量数据结构定义 */
// 数据结构
typedef struct Sema
{
	int value;                  /* 资源数量 */
	pthread_mutex_t mutex;      /* 互斥量 */
	pthread_cond_t cond;        /* 条件变量 */
} Sema;
// 1. 初始化
void sema_init(Sema* sema, int value)
{
	sema->value = value;
	pthread_mutex_init(&sema->mutex, NULL);
	pthread_cond_init(&sema->cond, NULL);
}
// 2. 等待
void sema_wait(Sema* sema)
{
	pthread_mutex_lock(&sema->mutex);
	while (sema->value <= 0)
	{
		pthread_cond_wait(&sema->cond, &sema->mutex);
	}
	sema->value--;
	pthread_mutex_unlock(&sema->mutex);
}
// 3. 唤醒
void sema_signal(Sema* sema)
{
	pthread_mutex_lock(&sema->mutex);
	sema->value++;
	pthread_cond_signal(&sema->cond);
	pthread_mutex_unlock(&sema->mutex);
}

// 信号量
Sema ping_sema, pong_sema;

void* ping(void* arg)
{
	while (1)
	{
		// 等待 
		sema_wait(&pong_sema);						/* 等待一个pong */
		// 输出
		printf("ping\n");							/* 消耗一个pong以输出一个ping */
		// 放入
		sema_signal(&ping_sema);					/* 放入一个ping */
		sleep(1);
	}
}

void* pong(void* arg)
{
	while (1)
	{
		// 等待 
		sema_wait(&ping_sema);						/* 等待一个ping */
		// 输出
		printf("pong\n");							/* 消耗一个ping以输出一个pong */
		// 放入
		sema_signal(&pong_sema);					/* 放入一个pong */
		sleep(1);
	}
}

int main()
{
	// 线程ID
	pthread_t ping_tid, pong_tid;
	// 初始化 互斥量 条件变量
	sema_init(&ping_sema, 0);
	sema_init(&pong_sema, 1);						/* 因为一开始输出ping,所以一开始资源是只有一个pong */

	// 创建子线程执行 
	pthread_create(&ping_tid, NULL, ping, NULL);
	pthread_create(&pong_tid, NULL, pong, NULL);

	// 等待子线程执行完毕
	pthread_join(ping_tid, NULL);
	pthread_join(pong_tid, NULL);
	return 0;
}