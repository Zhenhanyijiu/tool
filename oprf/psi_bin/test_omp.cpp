#include <iostream>
#include <omp.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <vector>
using namespace std;
//g++ test_omp.cpp -fopenmp
// void test()
// {
//     for (int i = 0; i < 80000; i++)
//     {
//     }
// }

int main()
{
    // float startTime = omp_get_wtime();
    // int count = 160000000;
    // count = 80000;
    // int sum = 0;
    int num = 8;
    //指定2个线程
    // #pragma omp parallel for num_threads(num)
    //     for (int i = 0; i < count; i++)
    //     {
    //         test();
    //         // sum++;
    //         // sleep(1);
    //     }
    long lenVector = 8;
    float startTime = omp_get_wtime();
    vector<vector<char>> tmpvec;
    tmpvec.resize(lenVector);

#pragma omp parallel for num_threads(num)
    for (int i = 0; i < lenVector; i++)
    {
        tmpvec[i].resize(100);
        memset(tmpvec[i].data(), 0, 100);
        sprintf(tmpvec[i].data(), "===>>id:%d,######\n", i);
        // sleep(1);
    }

    float endTime = omp_get_wtime();
    printf("指定 %d 个线程，执行时间: %f\n", num, endTime - startTime);
    char aa[100] = {0};
    memset(aa, 0, 100);
    for (int i = 0; i < 3; i++)
    {
        sprintf(aa, "===>>id:%d,######\n", i);
        printf("===>>indexs:%d,%s\n", i, tmpvec[i].data());
        int fg = strcmp(aa, tmpvec[i].data());
        assert(fg == 0);
    }
    startTime = endTime;
    int widthBucket1 = 5;
    int count = 0;
    int matrixWidth = 1;
    for (auto wLeft = 0; wLeft < matrixWidth; wLeft += widthBucket1)
    {
        count++;
    }
    printf("===>>count:%d\n", count);
    int thnum = 2;
    int width = 128;
    vector<int> v1(thnum);
    vector<int> v2(thnum);
    // for (int i = 0; i < thnum; i++)
    // {
    //     v1[i] = 0;
    // }
    widthBucket1 = 5;
    int sum = 0;
    int isExit = 0;
    for (;;)
    {
        for (int k = 0; k < thnum; k++)
        {
            int wRight = sum + widthBucket1;
            if (wRight <= width)
            {
                v1[k] += widthBucket1;
                v2[k]++;
            }
            else
            {
                isExit = 1;
                break;
            }
            sum += widthBucket1;
        }
        if (isExit)
        {
            break;
        }
    }
    printf("sum:=%d\n", sum);
    if ((width - sum) != 0)
    {
        v1[thnum - 1] += width - sum;
        v2[thnum - 1] += 1;
    }

    for (int i = 0; i < thnum; i++)
    {
        printf("v1[%d]=%2d,", i, v1[i]);
        // printf("v2[%d]=%d\n", i, v2[i]);
    }
    printf("\n");
    for (int i = 0; i < thnum; i++)
    {
        // printf("v1[%d]=%d\n", i, v1[i]);
        printf("v2[%d]=%2d,", i, v2[i]);
    }
    printf("\n");
    //     //指定4个线程
    // #pragma omp parallel for num_threads(4)
    //     for (int i = 0; i < 80000; i++)
    //     {
    //         test();
    //     }
    //     endTime = omp_get_wtime();
    //     printf("指定 4 个线程，执行时间: %f\n", endTime - startTime);
    //     startTime = endTime;

    //     //指定8个线程
    // #pragma omp parallel for num_threads(8)
    //     for (int i = 0; i < 80000; i++)
    //     {
    //         test();
    //     }
    //     endTime = omp_get_wtime();
    //     printf("指定 8 个线程，执行时间: %f\n", endTime - startTime);
    //     startTime = endTime;

    //     //指定12个线程
    // #pragma omp parallel for num_threads(12)
    //     for (int i = 0; i < 80000; i++)
    //     {
    //         test();
    //     }
    //     endTime = omp_get_wtime();
    //     printf("指定 12 个线程，执行时间: %f\n", endTime - startTime);
    //     startTime = endTime;

    //     //指定16个线程
    // #pragma omp parallel for num_threads(16)
    //     for (int i = 0; i < 80000; i++)
    //     {
    //         test();
    //     }
    //     endTime = omp_get_wtime();
    //     printf("指定 16 个线程，执行时间: %f\n", endTime - startTime);
    //     startTime = endTime;

    //     //指定32个线程
    // #pragma omp parallel for num_threads(32)
    //     for (int i = 0; i < 80000; i++)
    //     {
    //         test();
    //     }
    //     endTime = omp_get_wtime();
    //     printf("指定 32 个线程，执行时间: %f\n", endTime - startTime);
    //     startTime = endTime;

    //不使用OpenMP
    // sum = 0;
    // for (int i = 0; i < count; i++)
    // {
    //     test();
    //     // sum++;
    //     // sleep(1);
    // }
    // endTime = omp_get_wtime();
    // printf("不使用OpenMP多线程，执行时间: %f,sum:%d\n", endTime - startTime, sum);
    // startTime = endTime;
#pragma omp parallel
    printf("Hello World!\n");
    printf("===>>main end\n");
    return 0;
}