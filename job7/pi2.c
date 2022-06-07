#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#define NR_TOTAL 50000							/* 总计算项数 */
#define NR_CPU	2								/* 总CPU数目 */
#define NR_CHILD (NR_TOTAL / NR_CPU)			/* 每个子进程的任务量 */

typedef struct Param			/* 每个线程的计算范围[start,end] */
{
	int start;
	int end;
} Param ;

typedef struct Result			/* 每个线程的返回值类型 */
{
	float val;
} Result ;

/*
FUNC : calcute
	 : 计算 start - end 的项和
PARAM :
	@arg : void* 协定为 Param* 类型
OTHRE : 这是一个线程函数
*/
void* calculate(void* arg)
{
	Param* param = (Param*) arg;
	Result* result = malloc(sizeof(Result));
	result->val = 0.0;

	int i = 0;
	for (i = param->start; i <= param->end; i++)
	{
		if (i % 2 == 1)
		{
			result->val += (1.0 / (2 * i - 1));
		}
		else
		{
			result->val -= (1.0 / (2 * i - 1));
		}
	}

	return result;
}

int main()
{
	// STEP1 创建子线程 分配任务
	pthread_t workers[NR_CPU + 1];					/* 根据NR_CPU创建对应数量的线程 i in 1..NR_CPU */
	Param params[NR_CPU + 1];						/* 每个线程对应一个参数,规定计算范围 [(i-1)*NR_CHILD +1 , i * NR_CHILD] */
	int i = 1;
	for (i = 1; i <= NR_CPU; i++)
	{
		Param* param = &params[i];
		param->start = (i - 1) * NR_CHILD + 1;
		param->end = i * NR_CHILD;
		pthread_create(&workers[i], NULL, calculate, param);
	}

	// STEP2 等待子线程执行 获取数据
	float total = 0.0;
	for (i = 1; i <= NR_CPU; i++)
	{
		Result* result = NULL;
		pthread_join(workers[i], (void**)&result);
		total += result->val;
		free(result);
	}

	// STEP3 汇总计算结果
	float PI = 4 * total;
	printf("PI = %.6f\n", PI);

	return 0;
}