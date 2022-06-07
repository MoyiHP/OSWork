#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#define NR_TOTAL 50000							/* �ܼ������� */
#define NR_CPU	2								/* ��CPU��Ŀ */
#define NR_CHILD (NR_TOTAL / NR_CPU)			/* ÿ���ӽ��̵������� */

typedef struct Param			/* ÿ���̵߳ļ��㷶Χ[start,end] */
{
	int start;
	int end;
} Param ;

typedef struct Result			/* ÿ���̵߳ķ���ֵ���� */
{
	float val;
} Result ;

/*
FUNC : calcute
	 : ���� start - end �����
PARAM :
	@arg : void* Э��Ϊ Param* ����
OTHRE : ����һ���̺߳���
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
	// STEP1 �������߳� ��������
	pthread_t workers[NR_CPU + 1];					/* ����NR_CPU������Ӧ�������߳� i in 1..NR_CPU */
	Param params[NR_CPU + 1];						/* ÿ���̶߳�Ӧһ������,�涨���㷶Χ [(i-1)*NR_CHILD +1 , i * NR_CHILD] */
	int i = 1;
	for (i = 1; i <= NR_CPU; i++)
	{
		Param* param = &params[i];
		param->start = (i - 1) * NR_CHILD + 1;
		param->end = i * NR_CHILD;
		pthread_create(&workers[i], NULL, calculate, param);
	}

	// STEP2 �ȴ����߳�ִ�� ��ȡ����
	float total = 0.0;
	for (i = 1; i <= NR_CPU; i++)
	{
		Result* result = NULL;
		pthread_join(workers[i], (void**)&result);
		total += result->val;
		free(result);
	}

	// STEP3 ���ܼ�����
	float PI = 4 * total;
	printf("PI = %.6f\n", PI);

	return 0;
}