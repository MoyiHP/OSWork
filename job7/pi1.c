#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#define N 50000					/* 总计算项数 */
// 计算函数
float master_output = 0.0;
void master_calculate()
{
	int i = 1;
	for (i = 1; i <= N / 2; i++)
	{
		if(i % 2 == 1)
		{
			master_output += (1.0 / (2 * i - 1));
		}
		else
		{
			master_output -= (1.0 / (2 * i - 1));
		}
	}
}

float worker_output = 0.0;
void* worker_calculate(void* arg)
{
	int i = 1;
	for (i = N / 2 + 1; i <= N ; i++)
	{
		if (i % 2 == 1)
		{
			worker_output += (1.0 / (2 * i - 1));
		}
		else
		{
			worker_output -= (1.0 / (2 * i - 1));
		}
	}
	return NULL;
}


int main()
{
	// 子进程计算后半部分
	pthread_t worker_tid;
	pthread_create(&worker_tid, NULL, worker_calculate, NULL);

	// 父进程计算前半部分
	master_calculate();

	// 等待子进程计算结果
	pthread_join(worker_tid, NULL);

	float PI = (master_output + worker_output) * 4;;
	printf("master_output = %.6f; worker_outpur = %.6f ; PI = %.6f\n", master_output, worker_output, PI);

	return 0;
}