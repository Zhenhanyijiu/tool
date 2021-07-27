#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#define NUM_THREADS 5
typedef struct DataType
{
    int id;
    char display[100];
} DataType;

void *process(void *args)
{
    sleep(5);
    DataType *data = (DataType *)args;
    sprintf(data->display, "this is id:%d", data->id);
    pthread_exit(NULL);
    return NULL;
}
#if 0
int main()
{
    pthread_t tids[NUM_THREADS];
    DataType datas[NUM_THREADS];
    for (size_t i = 0; i < NUM_THREADS; i++)
    {
        /* code */
        memset(datas[i].display, 0, 100);
        datas[i].id = i;
    }
    for (size_t i = 0; i < NUM_THREADS; i++)
    {
        /* code */
        int ret = pthread_create(&tids[i], NULL, process, (void *)&datas[i]);
        if (ret)
        {
            printf("===>>pthread create errorh");
        }
    }
    //等各个线程退出后，进程才结束，否则进程强制结束了，线程可能还没反应过来；
    for (size_t i = 0; i < NUM_THREADS; i++)
    {
        /* code */
        printf("id:%d,,display:%s\n", datas[i].id, datas[i].display);
    }

    pthread_exit(NULL);

    return 0;
}
#endif
#if 1
#include <stdlib.h>
//等待线程完成demo
int main()
{

    //定义5个线程id
    pthread_t tids[NUM_THREADS];
    //线程属性
    pthread_attr_t attr;
    //初始化并设置线程为可连接的
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    //创建线程
    DataType datas[NUM_THREADS];
    for (size_t i = 0; i < NUM_THREADS; i++)
    {
        memset(datas[i].display, 0, 100);
        datas[i].id = i;
        int ret = pthread_create(&tids[i], NULL, process, (void *)&datas[i]);
        if (ret)
        {
            printf("===>>pthread create errorh");
            exit(-1);
        }
    }

    return 0;
}
#endif
