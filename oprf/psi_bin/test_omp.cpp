#include <assert.h>
#include <omp.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <future>
#include <iostream>
#include <unordered_map>
#include <vector>
using namespace std;
// g++ test_omp.cpp -fopenmp
// void test()
// {
//     for (int i = 0; i < 80000; i++)
//     {
//     }
// }
#if 0
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
#endif
#if 1
double f(double a, double b) {
  double c = a + b;
  printf(">>>>>>>\n");
  sleep(2);
  return c;
}
int main() {
  unordered_map<unsigned long long int, vector<unsigned int>> table_map;
  printf("table_map:%ld,%ld\n", sizeof(table_map),
         sizeof(unordered_map<unsigned long long int, vector<unsigned int>>));
  unordered_map<int, int> table_map1;
  table_map1[8] = 77;
  table_map1[7] = 77;
  printf("table_map1:%ld,%ld\n", sizeof(table_map1),
         sizeof(unordered_map<int, int>));

  ////////////
  //   double a = 1.0, b = 2.1;
  //   future<double> fu = async(f, a, b);
  //   shared_future<double> fu = async(f, a, b);
  //   shared_future<double> c1 = async(f, a, fu.get());
  //   shared_future<double> c2 = async(f, a, fu.get());
  // cout << "..." << endl;
  //   printf("=======......1\n");
  //   cout << "result:" << c1.get() << endl;
  //   printf("=======......2\n");
  //   cout << "result:" << c2.get() << endl;
  printf("=======......3\n");
#pragma omp parallel  // for num_threads(2)
  printf("hhhh\n");
#pragma omp parallel for num_threads(2)
  for (int i = 0; i < 2; i++) {
    double a = i, b = 2.7;
    shared_future<double> fu = async(f, a, b);
    cout << "result:" << fu.get() << endl;
  }

  //   cout << "result:" << fu.get() << endl;
}
#endif