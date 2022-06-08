#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

/* Buffer 数据结构定义 */
// 数据结构
#define CAPACITY 4
typedef struct Buffer
{
    int buf[CAPACITY];
    int in;
    int out;
} Buffer ;
// 1. 初始化
void buffer_init(Buffer* buffer)
{
    if (buffer != NULL)
    {
        buffer->in = 0;
        buffer->out = 0;
    }
}
// 2. 是否为空
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
// 3. 是否为满
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
// 4. 读数据
int get_item(Buffer* buffer)
{
    int item;
    item = buffer->buf[buffer->out];
    buffer->out = (buffer->out + 1) % CAPACITY;
    return item;
}
// 5. 写数据
void put_item(Buffer* buffer, int item)
{
    buffer->buf[buffer->in] = item;
    buffer->in = (buffer->in + 1) % CAPACITY;
}

// 两个Buffer
Buffer buffer1, buffer2;

/* 临界区 条件变量定义 */
pthread_mutex_t mutex1, mutex2;                                             /* 两个Buffer分配两个临界区 */
pthread_cond_t wait_empty_buffer1, wait_empty_buffer2;                      /* Buffer对应的 为空 条件 */
pthread_cond_t wait_full_buffer1, wait_full_buffer2;                        /* Buffer对应的 为满 条件 */

/* 生产者 计算者 消费者 线程函数 */
#define ITEM_COUNT (CAPACITY * 2)
// 生产者线程函数
// 生产数据到 buffer1
void* produce(void* arg)
{
    int i， item;
    for (i = 0; i < ITEM_COUNT; i++) {
        pthread_mutex_lock(&mutex1);                                        /* LOCK MUTEX1 */
        while (buffer_is_full(&buffer1))                                    /* 等待 buffer1 不满 */
            pthread_cond_wait(&wait_empty_buffer1, &mutex1);

        item = 'a' + i;                                                     /* 生产 */
        put_item(&buffer1, item);
        printf("produce item: %c\n", item);

        pthread_cond_signal(&wait_full_buffer1);                            /* SIGNAL buffer1 满 */
        pthread_mutex_unlock(&mutex1);                                      /* UNLOCK MUTEX1 */
    }
    return NULL;
}
// 计算者线程函数
// 从 buffer1 取数据 计算后 放到 buffer2
void* calculate(void* arg)
{
    int i, item;
    
    for (i = 0; i < ITEM_COUNT; i++) {
        // 取数据部分
        pthread_mutex_lock(&mutex1);                                        /* LOCK MUTEX1 */
        while (buffer_is_empty(&buffer1))                                   /* 等待 buffer1 不空 */
            pthread_cond_wait(&wait_full_buffer1, &mutex1);
        item = get_item(&buffer1);                                          /* 获取数据 */
        printf("    calculate get item: %c\n", item);
        pthread_cond_signal(&wait_empty_buffer1);                           /* SIGNAL buffer1 空 */
        pthread_mutex_unlock(&mutex1);                                      /* UNLOCK MUTEX1 */

        // 计算部分
        item = item + 'A' - 'a';

        // 放数据部分
        pthread_mutex_lock(&mutex2);                                        /* LOCK MUTEX2 */
        while (buffer_is_full(&buffer2))                                    /* 等待buffer2不满 */
            pthread_cond_wait(&wait_empty_buffer2, &mutex2);        
        put_item(&buffer2, item);                                           /* 放数据 */
        printf("    calculate put item: %c\n", item);
        pthread_cond_signal(&wait_full_buffer2);                            /* SIGNAL buffer2 有数据 */
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

/* 主函数部分 */

int main()
{
    // 初始化buffer
    buffer_init(&buffer1);
    buffer_init(&buffer2);
    // 线程ID
    pthread_t calculater_tid, consumer_tid;
    // 初始化 互斥量 条件变量
    pthread_mutex_init(&mutex1, NULL);
    pthread_mutex_init(&mutex2, NULL);
    pthread_cond_init(&wait_empty_buffer1, NULL);
    pthread_cond_init(&wait_empty_buffer2, NULL);
    pthread_cond_init(&wait_full_buffer1, NULL);
    pthread_cond_init(&wait_full_buffer2, NULL);

    // 创建子线程执行 计算者、消费者， 主线程执行生产者
    pthread_create(&calculater_tid, NULL, calculate, NULL);
    pthread_create(&consumer_tid, NULL, consume, NULL);
    produce(NULL);

    // 等待子线程执行完毕
    pthread_join(calculater_tid, NULL);
    pthread_join(consumer_tid, NULL);
    return 0;
}