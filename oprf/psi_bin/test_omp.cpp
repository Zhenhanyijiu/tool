#include <iostream>
#include <omp.h>
#include <stdio.h>
#include <unistd.h>

using namespace std;
//g++ test_omp.cpp -fopenmp
void test()
{
    for (int i = 0; i < 80000; i++)
    {
    }
}

int main()
{
    float startTime = omp_get_wtime();
    int count = 160000000;
    count = 80000;
    int sum = 0;
    int num = 2;
    //指定2个线程
#pragma omp parallel for num_threads(num)
    for (int i = 0; i < count; i++)
    {
        test();
        // sum++;
        // sleep(1);
    }
    float endTime = omp_get_wtime();
    printf("指定 2 个线程，执行时间: %f,sum:%d\n", endTime - startTime, sum);
    startTime = endTime;

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
    sum = 0;
    for (int i = 0; i < count; i++)
    {
        test();
        // sum++;
        // sleep(1);
    }
    endTime = omp_get_wtime();
    printf("不使用OpenMP多线程，执行时间: %f,sum:%d\n", endTime - startTime, sum);
    startTime = endTime;

    printf("===>>main end\n");
    return 0;
}