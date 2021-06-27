
#include <stdio.h>
#include "test_c.h"
//g++ -I. -L. test_c_main.c -ltest_c
#if 1
int main()
{
    char out[77] = {0};
    char in[] = "123456";
    int out_len = 0;
    getrng(in, 0, out, &out_len);
    printf("====>>outlen:%d\n", out_len);
    for (int i = 0; i < out_len; i++)
    {
        printf("%x ", (unsigned char)out[i]);
    }
    printf("\n");
    return 0;
}

#endif