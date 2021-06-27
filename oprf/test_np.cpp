#include <stdio.h>

#include <iostream>

#include "cryptoTools/Common/BitVector.h"
#include "cryptoTools/Common/Defines.h"
#include "cryptoTools/Crypto/PRNG.h"
#include "cryptoTools/Crypto/RandomOracle.h"
namespace oc = osuCrypto;

using namespace std;
// using RandomOracle = oc::Blake2;
#if 0
int main()
{
    oc::PRNG rng(oc::toBlock(123));
    auto seed = rng.get<oc::block>();
    //   printf("===>>\n");
    cout << "s1:" << seed << endl;
    seed = rng.get<oc::block>();
    cout << "s2:" << seed << endl;
    ////
    auto R = rng.get<oc::block>();
    R = rng.get<oc::block>();
    oc::RandomOracle ro;
    cout << "===>>RandomOracle,HashSize:" << oc::RandomOracle::HashSize << endl;
    vector<oc::u8> comm(oc::RandomOracle::HashSize);
    // 0b81ce934b685f0e6e3a1e9d8cc1705f
    oc::u8 dd[16] = {0x5f, 0x70, 0xc1, 0x8c, 0x9d, 0x1e, 0x3a, 0x6e,
                     0x0e, 0x5f, 0x68, 0x4b, 0x93, 0xce, 0x81, 0x0b};
    ro.Update(dd);
    ro.Final(comm.data()); //
    for (int i = 0; i < comm.size(); i++)
    {
        printf("%x ", comm[i]);
    }
    printf("\n");
    oc::BitVector choices(60); //-w width==60
    rng.get(choices.data(), choices.sizeBytes());
    cout << "===>>choices.sizeBytes:" << choices.sizeBytes() << endl;
    cout << "===>>choice:" << choices << endl;
    oc::RandomOracle sha(sizeof(oc::block));
    printf("===>>sizeof(oc::block):%ld\n", sizeof(oc::block));
    sha.Update(dd, 16);
    oc::block res;
    sha.Final(res);
    cout << "===>>res:" << res << endl;
    ////
    //   vector<oc::u8> u8_vec(10);
    //   comm.data

    oc::BitVector bv(128);
    //就是iknp-ote协议中发送方sender，随机选择的s作为输入
    bv.randomize(rng);
    oc::u64 bvsize = bv.size();
    oc::u64 bvsizeByte = bv.sizeBytes();
    cout << "===>>bvsize:" << bvsize << endl;
    cout << "===>>bvsizebyres:" << bvsizeByte << endl;
    cout << "===>>bv:" << bv << endl;
    // for (int i = 0; i < 10; i++) {
    //   oc::u8 ch = bv[i];
    //   printf("%d ", ch);
    // }
    cout << "===>>          bv:";
    for (int i = 0; i < bv.size(); i++)
    {
        oc::u8 ch = bv[i];
        printf("%d", ch);
    }
    cout << endl;

    oc::BitVector choices2(8 * 128); // 8*128==1024
    choices2 = bv;
    cout << "===>>1...choices2:" << choices2 << endl;
    choices2.resize(8 * 128);
    cout << "===>>2...choices2:" << choices2
         << ",numBytes:" << choices2.sizeBytes() << endl;
    auto choiceBlocks = choices2.getSpan<oc::block>();
    cout << "===>>choiceBlocksSize:" << choiceBlocks.size() << endl; // 8
    for (int i = 0; i < choiceBlocks.size(); i++)
    {
        cout << "i:" << i << "," << choiceBlocks[i] << endl;
    }
    //   cout << "===>>choiceBlocksSize:" << choiceBlocks. << endl;         // 8
    cout << "===>>rng.mBlockIdx:" << rng.mBlockIdx << endl;
    cout << "===>>rng.mBlockIdx:" << rng.mBlockIdx << endl;
}
#endif
#if 0
#include "naorpinkas.h"
#include <string.h>
#include <algorithm>
void genOTMsg(vector<array<oc::block, 2>> &otMsg)
{
    oc::PRNG rng(oc::toBlock(0x1122334455));
    oc::u64 pairsSize = otMsg.size();
    // otMsg.resize(pairsSize);
    for (int i = 0; i < pairsSize; i++)
    {
        otMsg[i][0] = rng.get<oc::block>();
        otMsg[i][1] = rng.get<oc::block>();
    }
}
void printOTMsg(const vector<array<oc::block, 2>> &otMsg)
{
    int num = otMsg.size();
    for (int i = 0; i < num; i++)
    {
        cout << "第" << i + 1 << "对消息：\n";
        cout << "" << (0) << ":" << otMsg[i][0] << endl;
        cout << "" << (1) << ":" << otMsg[i][1] << endl;
    }
}
void check_recover(const vector<array<oc::block, 2>> &otMsg,
                   const oc::BitVector rChoices, const vector<oc::block> otMsgRecover)
{
    int size = otMsg.size();
    if (size != rChoices.size() || size != otMsgRecover.size())
    {
        printf("=======check error!!! error\n");
        return;
    }
    for (int i = 0; i < size; i++)
    {
        oc::u8 r = rChoices[i];
        cout << "第" << i + 1 << "对：\n";
        cout << "src:" << otMsg[i][r] << ",recover:" << otMsgRecover[i] << endl;
        if (neq(otMsg[i][r], otMsgRecover[i]))
        {
            printf("=======src is not equal recover");
            exit(-1);
        }
    }
}
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("error!!! argc!=2\n");
        return -1;
    }
    int flag = 0;
    //生成需要处理的消息对
    int pairs = atoi(argv[1]);
    printf("===>>input pairs:%d\n", pairs);
    int otMsgPairsSize = pairs;
    vector<array<oc::block, 2>> otMsg(otMsgPairsSize);
    genOTMsg(otMsg);
    printOTMsg(otMsg);
    oc::u8 r[8] = {0x77, 0xaa, 0xcb, 0x2b, 0xfd, 0xaf, 0x96, 0xea};
    oc::PRNG rng(oc::toBlock(0x666666));
    vector<oc::block> dest_block;
    dest_block.resize(pairs / 128 + 1);
    rng.get<oc::block>(dest_block.data(), pairs / 128 + 1);
    oc::BitVector rChoices((oc::u8 *)dest_block.data(), otMsgPairsSize);
    cout << "===rChoices:" << rChoices << ",size:" << rChoices.size() << endl;
    //初始化一个np-ot-sender
    oc::NaorPinkasSender npSender;
    npSender.init(rng, otMsgPairsSize, 128);
    oc::u8 *buf_AC = NULL;
    uint64_t bufSize = 0;
    cout << "=====before genpubparam...\n";
    //生成公共参数AC，并将AC发送给receiver
    flag = npSender.genPublicParam(&buf_AC, bufSize);
    cout << "1===>>flag:" << flag << endl;
    cout << "=====after  genpubparam...\n";
    //初始化一个np-ot-receiver
    oc::NaorPinkasReceiver npReceiver; //(oc::toBlock(0x332211), otMsgPairsSize);
    npReceiver.init(rng, otMsgPairsSize);
    oc::u8 *pk0buf = nullptr;
    oc::u64 pk0bufSize = 0;
    //传入公共参数AC，并生成pk0发送给sender方
    flag = npReceiver.genPK0(buf_AC, bufSize, rChoices, &pk0buf, pk0bufSize);
    cout << "2===>>flag:" << flag << endl;
    vector<array<oc::block, 2>> enckey;
    flag = npSender.getEncKey(pk0buf, pk0bufSize, enckey);
    cout << "3===>>flag:" << flag << endl;
    vector<array<oc::block, 2>> otMsgCipher;
    //对每一对消息进行加密，并发送给receiver
    flag = npSender.genOTCipher(enckey, otMsg, otMsgCipher);
    cout << "4===>>flag:" << flag << endl;
    //获取解密密钥
    vector<oc::block> deckey;
    flag = npReceiver.getDecKey(deckey);
    cout << "5===>>flag:" << flag << endl;
    vector<oc::block> otMsgRecover;
    flag = npReceiver.genOTRecover(deckey, rChoices, otMsgCipher, otMsgRecover);
    cout << "6===>>flag:" << flag << endl;
    cout << "===>>recover:" << otMsgRecover[0] << endl;
    check_recover(otMsg, rChoices, otMsgRecover);
    return 0;
}
#endif
#if 1
#include "iknpote.h"
int main()
{
    oc::PRNG rng(oc::toBlock(0x112233));
    //iknp 接收者
    oc::IknpOtExtReceiver iknpReceiver;
    iknpReceiver.init(rng);
    oc::u8 *pubParamBuf = nullptr;
    oc::u64 pubParamBufSize = 0;
    iknpReceiver.genPublicParamFromNpot(&pubParamBuf, pubParamBufSize);
    //iknp 发送者
    oc::IknpOtExtSender iknpSender;
    iknpSender.init(rng); //默认128对
    oc::u8 *pk0buf = nullptr;
    oc::u64 pk0bufSize = 0;
    iknpSender.genPK0FromNpot(pubParamBuf, pubParamBufSize, &pk0buf, pk0bufSize);
    //iknp receiver
    iknpReceiver.getEncKeyFromNpot(pk0buf, pk0bufSize);
    //iknp sender
    iknpSender.getDecKeyFromNpot();
    return 0;
}
#endif