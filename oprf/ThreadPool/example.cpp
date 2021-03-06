#include <iostream>
#include <vector>
#include <chrono>
#include <stdio.h>
#include "ThreadPool.h"

int process(const int i)
{
    //  std::cout << "===hello " << i << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    //  std::cout << "world " << i << std::endl;
    return i * i;
}
int main()
{

    ThreadPool pool(8);
    std::vector<std::future<int>> results;

    for (int i = 0; i < 81; ++i)
    {
        results.emplace_back(
            // pool.enqueue([i]
            //              {
            //                  //  std::cout << "===hello " << i << std::endl;
            //                  std::this_thread::sleep_for(std::chrono::seconds(1));
            //                  //  std::cout << "world " << i << std::endl;
            //                  return i * i;
            //              }));
            pool.enqueue(process, i));
    }
    printf("===>>const results.size:%ld\n", results.size());
    // for (auto &&result : results)
    //     std::cout << result.get() << ' ';
    for (int i = 0; i < results.size(); i++)
        std::cout << results[i].get() << ' ';
    std::cout << std::endl;

    return 0;
}
