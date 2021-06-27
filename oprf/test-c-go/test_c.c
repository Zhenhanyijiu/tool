#include "cryptoTools/Crypto/PRNG.h"
#include <cryptoTools/Common/Defines.h>
#include <string.h>
#include "test_c.h"
namespace oc = osuCrypto;
typedef char Block[16];
void getrng(const char *in, const int in_len, char *out, int *out_len)
{
    oc::PRNG prng;
    prng.SetSeed(oc::toBlock((oc::u8 *)in));
    oc::block res = prng.get<oc::block>();
    memcpy(out, (oc::u8 *)&res, 16);
    *out_len = 16;
}
#if 0
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