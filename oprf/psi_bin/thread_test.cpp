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
#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <vector>
#if 0
using namespace std;
typedef struct Info
{
    int id;
    char name[100];
    vector<int> *vecP;
} Info;
void *wait(void *t)
{
    int i;
    // long tid;
    // tid = (long)t;
    Info *info = (Info *)t;
    // printf("===id:%d\n", info->id);
    printf("===id:%d\n", info->id);
    if (info->id == 0)
    {
        sleep(6);
        memset(info->name, 0, 100);
        sprintf(info->name, "%d,sleep 6s...\n", info->id);
    }
    else
    {
        sleep(2);
        memset(info->name, 0, 100);
        sprintf(info->name, "%d,sleep 2s...\n", info->id);
    }
    // cout << "===>>Thread with id : " << *(int *)(t) << "  ...exiting " << endl;
    pthread_exit(NULL);
}

int main()
{
    int rc;
    int i;
    pthread_t threads[NUM_THREADS];
    pthread_attr_t attr;
    void *status;

    // 初始化并设置线程为可连接的（joinable）
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    int nn = 0;
    Info infos[NUM_THREADS];

    for (i = 0; i < NUM_THREADS; i++)
    {
        // int nn = 0;
        // cout << "###main() : creating thread, " << i << endl;
        // Info info;
        // info.id = nn;
        // printf("===id:%d\n", info.id);
        infos[i].id = i;
        rc = pthread_create(&threads[i], NULL, wait, (void *)(infos + i));
        if (rc)
        {
            cout << "Error:unable to create thread," << rc << endl;
            exit(-1);
        }
        // nn = nn + 1;
    }

    // 删除属性，并等待其他线程
    pthread_attr_destroy(&attr);
    for (i = 0; i < NUM_THREADS; i++)
    {
        rc = pthread_join(threads[i], &status);
        if (rc)
        {
            cout << "Error:unable to join," << rc << endl;
            exit(-1);
        }
        cout << "Main: completed thread id :" << i;
        cout << "  exiting with status :" << status << endl;
    }

    cout << "************Main: program exiting.*********" << endl;
    for (i = 0; i < NUM_THREADS; i++)
    {
        printf("===>>>%d:%s\n", i, infos[i].name);
    }
    pthread_exit(NULL);
}
#endif
#if 0
using namespace std;
typedef struct Info
{
    int id;
    int num;
    int *vecIn;
    int *vecOut;
} Info;
void *wait(void *t)
{
    Info *info = (Info *)t;
    printf("===id:%d\n", info->id);
    for (int i = 0; i < info->num; i++)
    {
        printf("..id(%d)...in.....(%d)\n", info->id, info->vecIn[i]);
        info->vecOut[i] = 10 + info->vecIn[i];
    }
    sleep(1);
    pthread_exit(NULL);
}

int main()
{
    // int rc;
    int i;
    pthread_t threads[2];
    pthread_attr_t attr;
    void *status;
    // 初始化并设置线程为可连接的（joinable）
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    vector<int> vecInput(10);
    vector<int> vecOutput(10);
    for (int i = 0; i < 10; i++)
    {

        vecInput[i] = i + 1;
        vecOutput[i] = 0;
    }

    Info infos[2];

    for (i = 0; i < 2; i++)
    {
        infos[i].id = i;
        infos[i].num = 5;
        infos[i].vecIn = vecInput.data() + i * 5;
        infos[i].vecOut = vecOutput.data() + i * 5;
        int rc = pthread_create(&threads[i], NULL, wait, (void *)(infos + i));
        if (rc)
        {
            cout << "Error:unable to create thread," << rc << endl;
            exit(-1);
        }
    }
    // 删除属性，并等待其他线程
    pthread_attr_destroy(&attr);
    for (i = 0; i < 2; i++)
    {
        int rc = pthread_join(threads[i], &status);
        if (rc)
        {
            cout << "Error:unable to join," << rc << endl;
            exit(-1);
        }
        cout << "Main: completed thread id :" << i;
        cout << "  exiting with status :" << status << endl;
    }

    cout << "************Main: program exiting.*********" << endl;

    for (i = 0; i < 10; i++)
    {
        printf("===>>>vecOutput[%d]=%d\n", i, vecOutput[i]);
    }
    pthread_exit(NULL);
}
#endif
#if 1
using namespace std;
typedef struct Info
{
    int id;
    int num;
    vector<char> *vecIn;
    vector<char> *vecOut;
} Info;
void *wait(void *t)
{
    Info *info = (Info *)t;
    printf("===id:%d\n", info->id);
    for (int i = 0; i < info->num; i++)
    {
        printf("..id(%d)...in.....(%s)\n", info->id, info->vecIn[i].data());
        // info->vecOut[i] = 10 + info->vecIn[i];
        sprintf(info->vecOut[i].data(), "output + %s", info->vecIn[i].data());
    }
    sleep(2);
    pthread_exit(NULL);
}

int main()
{
    // int rc;
    int i;
    pthread_t threads[2];
    pthread_attr_t attr;
    void *status;
    // 初始化并设置线程为可连接的（joinable）
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    vector<vector<char>> vecInput(10);
    vector<vector<char>> vecOutput(10);
    // vector<vector<int>> vecOutputpp;
    vecInput.resize(10);
    vecOutput.resize(10);

    for (int i = 0; i < 10; i++)
    {
        vecInput[i].resize(100);
        memset(vecInput[i].data(), 0, 100);
        sprintf(vecInput[i].data(), "this is %d input", i);
        vecOutput[i].resize(100);
        memset(vecOutput[i].data(), 0, 100);
    }

    Info infos[2];

    for (i = 0; i < 2; i++)
    {
        infos[i].id = i;
        infos[i].num = 5;
        infos[i].vecIn = (vector<char> *)(vecInput.data()) + i * 5;
        infos[i].vecOut = (vector<char> *)(vecOutput.data()) + i * 5;
        int rc = pthread_create(&threads[i], NULL, wait, (void *)(infos + i));
        if (rc)
        {
            cout << "Error:unable to create thread," << rc << endl;
            exit(-1);
        }
    }
    // 删除属性，并等待其他线程
    pthread_attr_destroy(&attr);
    for (i = 0; i < 2; i++)
    {
        int rc = pthread_join(threads[i], &status);
        if (rc)
        {
            cout << "Error:unable to join," << rc << endl;
            exit(-1);
        }
        cout << "Main: completed thread id :" << i;
        cout << "  exiting with status :" << status << endl;
    }

    cout << "************Main: program exiting.*********" << endl;

    for (i = 0; i < 10; i++)
    {
        printf("===>>>vecOutput[%d]=%s\n", i, vecOutput[i].data());
    }
    pthread_exit(NULL);
}
#endif
