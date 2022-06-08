#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

/* �ź������ݽṹ���� */
// ���ݽṹ
typedef struct Sema
{
	int value;                  /* ��Դ���� */
	pthread_mutex_t mutex;      /* ������ */
	pthread_cond_t cond;        /* �������� */
} Sema;
// 1. ��ʼ��
void sema_init(Sema* sema, int value)
{
	sema->value = value;
	pthread_mutex_init(&sema->mutex, NULL);
	pthread_cond_init(&sema->cond, NULL);
}
// 2. �ȴ�
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
// 3. ����
void sema_signal(Sema* sema)
{
	pthread_mutex_lock(&sema->mutex);
	sema->value++;
	pthread_cond_signal(&sema->cond);
	pthread_mutex_unlock(&sema->mutex);
}

// �ź���
Sema ping_sema, pong_sema;

void* ping(void* arg)
{
	while (1)
	{
		// �ȴ� 
		sema_wait(&pong_sema);						/* �ȴ�һ��pong */
		// ���
		printf("ping\n");							/* ����һ��pong�����һ��ping */
		// ����
		sema_signal(&ping_sema);					/* ����һ��ping */
		sleep(1);
	}
}

void* pong(void* arg)
{
	while (1)
	{
		// �ȴ� 
		sema_wait(&ping_sema);						/* �ȴ�һ��ping */
		// ���
		printf("pong\n");							/* ����һ��ping�����һ��pong */
		// ����
		sema_signal(&pong_sema);					/* ����һ��pong */
		sleep(1);
	}
}

int main()
{
	// �߳�ID
	pthread_t ping_tid, pong_tid;
	// ��ʼ�� ������ ��������
	sema_init(&ping_sema, 0);
	sema_init(&pong_sema, 1);						/* ��Ϊһ��ʼ���ping,����һ��ʼ��Դ��ֻ��һ��pong */

	// �������߳�ִ�� 
	pthread_create(&ping_tid, NULL, ping, NULL);
	pthread_create(&pong_tid, NULL, pong, NULL);

	// �ȴ����߳�ִ�����
	pthread_join(ping_tid, NULL);
	pthread_join(pong_tid, NULL);
	return 0;
}