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

/* 信号量数据结构定义 */
// 数据结构
typedef struct Sema
{
    int value;                  /* 资源数量 */
    pthread_mutex_t mutex;      /* 互斥量 */
    pthread_cond_t cond;        /* 条件变量 */
} Sema ;
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
    while ( sema->value <= 0)
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

// 两个Buffer
Buffer buffer1, buffer2;
// 信号量
Sema mutex_buffer1_sema, mutex_buffer2_sema;                                /* buffer1 2 互斥量 */
Sema full_buffer1_sema, empty_buffer1_sema;                                 /* 满/空 buffer1 信号量 */
Sema full_buffer2_sema, empty_buffer2_sema;                                 /* 满/空 buffer2 信号量 */

/* 生产者 计算者 消费者 线程函数 */
#define ITEM_COUNT (CAPACITY * 2)
// 生产者线程函数
// 生产数据到 buffer1
void* produce(void* arg)
{
    int i, item;
    for (i = 0; i < ITEM_COUNT; i++) {
        // 等待
        sema_wait(&empty_buffer1_sema);                                     /* 等待buffer1中的一个空项 */
        sema_wait(&mutex_buffer1_sema);                                     /* LOCK buffer1 */
        // 写入数据
        item = i + 'a';
        put_item(&buffer1, item);
        printf("produce item: %c\n", item);
        // 释放
        sema_signal(&mutex_buffer1_sema);                                   /* UNLOCK buffer1 */
        sema_signal(&full_buffer1_sema);                                     /* 唤醒一个等待buffer1满项的线程 */
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
        // 等待
        sema_wait(&full_buffer1_sema);                                     /* 等待buffer1中的一个满项 */
        sema_wait(&mutex_buffer1_sema);                                     /* LOCK buffer1 */
        // 取出数据
        item = get_item(&buffer1);
        printf("    calculate get item: %c\n", item);
        // 释放
        sema_signal(&mutex_buffer1_sema);                                   /* UNLOCK buffer1 */
        sema_signal(&empty_buffer1_sema);                                    /* 唤醒一个等待buffer1空项的线程 */

        // 计算部分
        item = item + 'A' - 'a';

        // 放数据部分
        // 等待
        sema_wait(&empty_buffer2_sema);                                         /* 等待buffer2中的一个空项 */
        sema_wait(&mutex_buffer2_sema);                                         /* LOCK buffer2 */
        // 放入数据
        put_item(&buffer2, item);
        printf("    calculate put item: %c\n", item);
        // 释放
        sema_signal(&mutex_buffer2_sema);                                       /* UNLOCK buffer2 */
        sema_signal(&full_buffer2_sema);                                        /* 唤醒一个等待buffer2满项的线程 */
    }
    return NULL;
}

void* consume(void* arg)
{
    int i;
    int item;

    for (i = 0; i < ITEM_COUNT; i++) {
         // 等待
        sema_wait(&full_buffer2_sema);                                      /* 等待buffer2中的一个满项 */
        sema_wait(&mutex_buffer2_sema);                                     /* LOCK buffer2 */
        // 取出数据
        item = get_item(&buffer2);
        printf("        consume item: %c\n", item);
        // 释放
        sema_signal(&mutex_buffer2_sema);                                   /* UNLOCK buffer2 */
        sema_signal(&empty_buffer2_sema);                                    /* 唤醒一个等待buffer2空项的线程 */
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
    // 初始化 信号量
    sema_init(&mutex_buffer1_sema, 1);
    sema_init(&empty_buffer1_sema, CAPACITY - 1);
    sema_init(&full_buffer1_sema, 0);

    sema_init(&mutex_buffer2_sema, 1);
    sema_init(&empty_buffer2_sema, CAPACITY - 1);
    sema_init(&full_buffer2_sema, 0);

    // 创建子线程执行 计算者、消费者， 主线程执行生产者
    pthread_create(&calculater_tid, NULL, calculate, NULL);
    pthread_create(&consumer_tid, NULL, consume, NULL);
    produce(NULL);

    // 等待子线程执行完毕
    pthread_join(calculater_tid, NULL);
    pthread_join(consumer_tid, NULL);
    return 0;
}