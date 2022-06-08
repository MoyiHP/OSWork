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

/* �ź������ݽṹ���� */
// ���ݽṹ
typedef struct Sema
{
    int value;                  /* ��Դ���� */
    pthread_mutex_t mutex;      /* ������ */
    pthread_cond_t cond;        /* �������� */
} Sema ;
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
    while ( sema->value <= 0)
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

// ����Buffer
Buffer buffer1, buffer2;
// �ź���
Sema mutex_buffer1_sema, mutex_buffer2_sema;                                /* buffer1 2 ������ */
Sema full_buffer1_sema, empty_buffer1_sema;                                 /* ��/�� buffer1 �ź��� */
Sema full_buffer2_sema, empty_buffer2_sema;                                 /* ��/�� buffer2 �ź��� */

/* ������ ������ ������ �̺߳��� */
#define ITEM_COUNT (CAPACITY * 2)
// �������̺߳���
// �������ݵ� buffer1
void* produce(void* arg)
{
    int i, item;
    for (i = 0; i < ITEM_COUNT; i++) {
        // �ȴ�
        sema_wait(&empty_buffer1_sema);                                     /* �ȴ�buffer1�е�һ������ */
        sema_wait(&mutex_buffer1_sema);                                     /* LOCK buffer1 */
        // д������
        item = i + 'a';
        put_item(&buffer1, item);
        printf("produce item: %c\n", item);
        // �ͷ�
        sema_signal(&mutex_buffer1_sema);                                   /* UNLOCK buffer1 */
        sema_signal(&full_buffer1_sema);                                     /* ����һ���ȴ�buffer1������߳� */
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
        // �ȴ�
        sema_wait(&full_buffer1_sema);                                     /* �ȴ�buffer1�е�һ������ */
        sema_wait(&mutex_buffer1_sema);                                     /* LOCK buffer1 */
        // ȡ������
        item = get_item(&buffer1);
        printf("    calculate get item: %c\n", item);
        // �ͷ�
        sema_signal(&mutex_buffer1_sema);                                   /* UNLOCK buffer1 */
        sema_signal(&empty_buffer1_sema);                                    /* ����һ���ȴ�buffer1������߳� */

        // ���㲿��
        item = item + 'A' - 'a';

        // �����ݲ���
        // �ȴ�
        sema_wait(&empty_buffer2_sema);                                         /* �ȴ�buffer2�е�һ������ */
        sema_wait(&mutex_buffer2_sema);                                         /* LOCK buffer2 */
        // ��������
        put_item(&buffer2, item);
        printf("    calculate put item: %c\n", item);
        // �ͷ�
        sema_signal(&mutex_buffer2_sema);                                       /* UNLOCK buffer2 */
        sema_signal(&full_buffer2_sema);                                        /* ����һ���ȴ�buffer2������߳� */
    }
    return NULL;
}

void* consume(void* arg)
{
    int i;
    int item;

    for (i = 0; i < ITEM_COUNT; i++) {
         // �ȴ�
        sema_wait(&full_buffer2_sema);                                      /* �ȴ�buffer2�е�һ������ */
        sema_wait(&mutex_buffer2_sema);                                     /* LOCK buffer2 */
        // ȡ������
        item = get_item(&buffer2);
        printf("        consume item: %c\n", item);
        // �ͷ�
        sema_signal(&mutex_buffer2_sema);                                   /* UNLOCK buffer2 */
        sema_signal(&empty_buffer2_sema);                                    /* ����һ���ȴ�buffer2������߳� */
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
    // ��ʼ�� �ź���
    sema_init(&mutex_buffer1_sema, 1);
    sema_init(&empty_buffer1_sema, CAPACITY - 1);
    sema_init(&full_buffer1_sema, 0);

    sema_init(&mutex_buffer2_sema, 1);
    sema_init(&empty_buffer2_sema, CAPACITY - 1);
    sema_init(&full_buffer2_sema, 0);

    // �������߳�ִ�� �����ߡ������ߣ� ���߳�ִ��������
    pthread_create(&calculater_tid, NULL, calculate, NULL);
    pthread_create(&consumer_tid, NULL, consume, NULL);
    produce(NULL);

    // �ȴ����߳�ִ�����
    pthread_join(calculater_tid, NULL);
    pthread_join(consumer_tid, NULL);
    return 0;
}