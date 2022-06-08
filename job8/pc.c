#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

/* Buffer ���ݽṹ���� */
// ���ݽṹ
#define CAPACITY 4
typedef struct Buffer
{
    int buf[CAPACITY];
    int in;
    int out;
} Buffer ;
// 1. ��ʼ��
void buffer_init(Buffer* buffer)
{
    if (buffer != NULL)
    {
        buffer->in = 0;
        buffer->out = 0;
    }
}
// 2. �Ƿ�Ϊ��
int buffer_is_empty(Buffer* buffer)
{
    if (buffer != NULL)
    {
        return buffer->in == buffer->out;
    }
    else
    {
        return 0;
    }
}
// 3. �Ƿ�Ϊ��
int buffer_is_full(Buffer* buffer)
{
    if (buffer != NULL)
    {
        return (buffer->in + 1) % CAPACITY == buffer->out;
    }
    else
    {
        return 0;
    } 
}
// 4. ������
int get_item(Buffer* buffer)
{
    int item;
    item = buffer->buf[buffer->out];
    buffer->out = (buffer->out + 1) % CAPACITY;
    return item;
}
// 5. д����
void put_item(Buffer* buffer, int item)
{
    buffer->buf[buffer->in] = item;
    buffer->in = (buffer->in + 1) % CAPACITY;
}

// ����Buffer
Buffer buffer1, buffer2;

/* �ٽ��� ������������ */
pthread_mutex_t mutex1, mutex2;                                             /* ����Buffer���������ٽ��� */
pthread_cond_t wait_empty_buffer1, wait_empty_buffer2;                      /* Buffer��Ӧ�� Ϊ�� ���� */
pthread_cond_t wait_full_buffer1, wait_full_buffer2;                        /* Buffer��Ӧ�� Ϊ�� ���� */

/* ������ ������ ������ �̺߳��� */
#define ITEM_COUNT (CAPACITY * 2)
// �������̺߳���
// �������ݵ� buffer1
void* produce(void* arg)
{
    int i�� item;
    for (i = 0; i < ITEM_COUNT; i++) {
        pthread_mutex_lock(&mutex1);                                        /* LOCK MUTEX1 */
        while (buffer_is_full(&buffer1))                                    /* �ȴ� buffer1 ���� */
            pthread_cond_wait(&wait_empty_buffer1, &mutex1);

        item = 'a' + i;                                                     /* ���� */
        put_item(&buffer1, item);
        printf("produce item: %c\n", item);

        pthread_cond_signal(&wait_full_buffer1);                            /* SIGNAL buffer1 �� */
        pthread_mutex_unlock(&mutex1);                                      /* UNLOCK MUTEX1 */
    }
    return NULL;
}
// �������̺߳���
// �� buffer1 ȡ���� ����� �ŵ� buffer2
void* calculate(void* arg)
{
    int i, item;
    
    for (i = 0; i < ITEM_COUNT; i++) {
        // ȡ���ݲ���
        pthread_mutex_lock(&mutex1);                                        /* LOCK MUTEX1 */
        while (buffer_is_empty(&buffer1))                                   /* �ȴ� buffer1 ���� */
            pthread_cond_wait(&wait_full_buffer1, &mutex1);
        item = get_item(&buffer1);                                          /* ��ȡ���� */
        printf("    calculate get item: %c\n", item);
        pthread_cond_signal(&wait_empty_buffer1);                           /* SIGNAL buffer1 �� */
        pthread_mutex_unlock(&mutex1);                                      /* UNLOCK MUTEX1 */

        // ���㲿��
        item = item + 'A' - 'a';

        // �����ݲ���
        pthread_mutex_lock(&mutex2);                                        /* LOCK MUTEX2 */
        while (buffer_is_full(&buffer2))                                    /* �ȴ�buffer2���� */
            pthread_cond_wait(&wait_empty_buffer2, &mutex2);        
        put_item(&buffer2, item);                                           /* ������ */
        printf("    calculate put item: %c\n", item);
        pthread_cond_signal(&wait_full_buffer2);                            /* SIGNAL buffer2 ������ */
        pthread_mutex_unlock(&mutex2);                                      /* UNLOCK MUTEX2 */
    }
    return NULL;
}

void* consume(void* arg)
{
    int i;
    int item;

    for (i = 0; i < ITEM_COUNT; i++) {
        pthread_mutex_lock(&mutex2);
        while (buffer_is_empty(&buffer2))
            pthread_cond_wait(&wait_full_buffer2, &mutex2);

        item = get_item(&buffer2);
        printf("        consume item: %c\n", item);

        pthread_cond_signal(&wait_empty_buffer2);
        pthread_mutex_unlock(&mutex2);
    }
    return NULL;
}

/* ���������� */

int main()
{
    // ��ʼ��buffer
    buffer_init(&buffer1);
    buffer_init(&buffer2);
    // �߳�ID
    pthread_t calculater_tid, consumer_tid;
    // ��ʼ�� ������ ��������
    pthread_mutex_init(&mutex1, NULL);
    pthread_mutex_init(&mutex2, NULL);
    pthread_cond_init(&wait_empty_buffer1, NULL);
    pthread_cond_init(&wait_empty_buffer2, NULL);
    pthread_cond_init(&wait_full_buffer1, NULL);
    pthread_cond_init(&wait_full_buffer2, NULL);

    // �������߳�ִ�� �����ߡ������ߣ� ���߳�ִ��������
    pthread_create(&calculater_tid, NULL, calculate, NULL);
    pthread_create(&consumer_tid, NULL, consume, NULL);
    produce(NULL);

    // �ȴ����߳�ִ�����
    pthread_join(calculater_tid, NULL);
    pthread_join(consumer_tid, NULL);
    return 0;
}