#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

int array[] = { 1,12,3,15,4,11,2,5,14,6,16,20,9,13,17,8,7,10,18,19 };			/* ���������� */ 
int array_size = 20;

typedef struct Range								/* ѡ������Χ[start, end] */
{
	int start;
	int end;
} Range;


/*
FUNC : select
	 : �� Range[start,end]���ҵ���С��һ���������� index
*/
int selectMin(Range range)
{
	int minIndex = range.start;
	int i = range.start;
	for (i = range.start; i <= range.end; i++)
	{
		if (array[i] < array[minIndex])
		{
			minIndex = i;
		}
	}
	return minIndex;
}

void swap(int i, int j)
{
	int tmp = array[i];
	array[i] = array[j];
	array[j] = tmp;
}

/*
FUNC : selectSort
	 : ѡ������(��С����)һ������
*/
void* selectSort(void* arg)
{
	Range* range = (Range*)arg;
	int i = range->start;
	for (i = range->start; i <= range->end; i++)
	{
		int minIndex = selectMin(*range);
		swap(i, minIndex);
		range->start++;
	}
}

void dumpArray()
{
	int i = 0;
	for (i = 0; i <= array_size - 1; i++)
	{
		printf("%d ", array[i]);
	}
	printf("\n");
}

void merge()
{
	int left = 0;
	int right = 19;
	int mid = (left + right) / 2;

	int tmparray[20];
	int i = left;
	int j = mid + 1;
	int k = 0;
	while (i <= mid && j <= right)
	{
		if (array[i] <= array[j])
		{
			tmparray[k] = array[i];
			i++;
			k++;
		}
		else
		{
			tmparray[k] = array[j];
			j++;
			k++;
		}
	}
	while (i <= mid)
	{
		tmparray[k] = array[i];
		i++;
		k++;
	}
	while (j <= right)
	{

		tmparray[k] = array[j];
		j++;
		k++;
	}

	for (i = 0; i <= 19; i++)
	{
		array[i] = tmparray[i];
	}
}

int main()
{
	pthread_t worker1, worker2;
	Range range1 = { 0,9 };
	Range range2 = { 10,19 };
	pthread_create(&worker1, NULL, selectSort, &range1);
	pthread_create(&worker1, NULL, selectSort, &range2);

	pthread_join(worker1, NULL);
	pthread_join(worker2, NULL);

	merge();

	dumpArray();
	
	return 0;
}