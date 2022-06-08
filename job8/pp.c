#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

int state;								/* state 0 -- init 1 -- ping 2 -- pong */
pthread_mutex_t mutex;
pthread_cond_t wait_ping, wait_pong;

void* ping(void* arg)
{
	while (1)
	{
		pthread_mutex_lock(&mutex);
		while (state == 1)				/* state == 0 / 2 �������  == 1 Ӧ�õȴ�*/
		{
			pthread_cond_wait(&wait_pong, &mutex);
		}
		printf("ping\n");
		state = 1;

		pthread_cond_signal(&wait_ping);
		pthread_mutex_unlock(&mutex);   
		sleep(1);
	}
}

void* pong(void* arg)
{
	while (1)
	{
		pthread_mutex_lock(&mutex);
		while (state == 0 || state == 2)				/* state == 1 �������  == 0/2 Ӧ�õȴ�*/
		{
			pthread_cond_wait(&wait_ping, &mutex);
		}
		printf("pong\n");
		state = 2;

		pthread_cond_signal(&wait_pong);
		pthread_mutex_unlock(&mutex);
		sleep(1);
	}
}

int main()
{
	// �߳�ID
	pthread_t ping_tid, pong_tid;
	// ��ʼ�� ������ ��������
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&wait_ping, NULL);
	pthread_cond_init(&wait_pong, NULL);

	// �������߳�ִ�� 
	state = 0;
	pthread_create(&ping_tid, NULL, ping, NULL);
	pthread_create(&pong_tid, NULL, pong, NULL);

	// �ȴ����߳�ִ�����
	pthread_join(ping_tid, NULL);
	pthread_join(pong_tid, NULL);
	return 0;
}