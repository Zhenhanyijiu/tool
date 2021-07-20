#include <stdio.h>

#include <iostream>

#include "cryptoTools/Common/BitVector.h"
#include "cryptoTools/Common/Defines.h"
#include "cryptoTools/Common/CLP.h"
#include "cryptoTools/Crypto/PRNG.h"
#include "cryptoTools/Crypto/RandomOracle.h"
#include "iknpote.h"
#include "naorpinkas.h"
#include "psi.h"
#include <string.h>
#include <algorithm>
#include "channel.h"
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>
#include <fstream>
#define Psi_Size 2000000
namespace oc = osuCrypto;

using namespace std;
typedef struct TimeComputeType
{
    struct timeval start;
    struct timeval end;
} TimeCompute;
void *newTimeCompute()
{
    return malloc(sizeof(TimeCompute));
}
void startTime(void *tc)
{
    assert(tc != nullptr);
    TimeCompute *t = (TimeCompute *)tc;
    gettimeofday(&(t->start), NULL);
}
//毫秒
int getEndTime(void *tc)
{
    assert(tc != nullptr);
    TimeCompute *t = (TimeCompute *)tc;
    gettimeofday(&(t->end), NULL);
    int time_use = (t->end.tv_sec - t->start.tv_sec) * 1000000 +
                   (t->end.tv_usec - t->start.tv_usec);
    return time_use / 1000;
}
void releaseTimeCompute(void *tc)
{
    if (tc)
    {
        free(tc);
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
void printOTMsgSingle(const vector<oc::block> &otMsg)
{
    int num = otMsg.size();
    for (int i = 0; i < num; i++)
    {
        cout << "第" << i + 1 << "个消息：" << otMsg[i] << endl;
        // cout << "" << (1) << ":" << otMsg[i][1] << endl;
    }
}
void check_recover(const vector<array<oc::block, 2>> &otMsg,
                   const oc::BitVector &rChoices, const vector<oc::block> &otMsgRecover)
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
        printf("===check_recover ok...\n");
    }
}

//psi test use socket
#if 1
oc::u64 seed;
void generateDataSet(const int ptype, const oc::u64 dataSize,
                     oc::u64 seed, vector<vector<oc::u8>> &dataSet)
{
    oc::PRNG prng(oc::toBlock(0x77887788 + seed));
    //sender
    oc::u64 psiSize = Psi_Size;
    dataSet.resize(dataSize);
    assert(dataSize >= psiSize);
    if (ptype == 0)
    {
        oc::u64 i = 0;
        for (; i < psiSize; i++)
        {
            dataSet[i].resize(16);
            prng.get(dataSet[i].data(), 16);
        }
        prng.SetSeed(oc::toBlock(0x998877 + seed));
        for (; i < dataSize; i++)
        {
            dataSet[i].resize(16);
            // dataSet[i] = prng.get<oc::block>();
            prng.get(dataSet[i].data(), 16);
        }
    }
    //receiver
    if (ptype == 1)
    {
        for (oc::u64 i = 0; i < dataSet.size(); i++)
        {
            dataSet[i].resize(16);
            // dataSet[i] = prng.get<oc::block>();
            prng.get(dataSet[i].data(), 16);
        }
    }
}
//write file
void writeFileAllByCPP(const char *fileName, const char *buf, long bufSize)
{
    // 文件输出流;
    ofstream out;
    // 要写入整个文件，必须采用二进制打开
    out.open(fileName, ios::binary);
    assert(out);
    out.write(buf, bufSize);
    out.close();
}
//read file
char *readFileAllByCPP(const char *fileName, long *sizeOut)
{
    // filebuf *pbuf;
    ifstream filestr;
    // char *buffer;
    // 要读入整个文件，必须采用二进制打开
    filestr.open(fileName, ios::binary);
    // 获取filestr对应buffer对象的指针
    filebuf *pbuf = filestr.rdbuf();
    // 调用buffer对象方法获取文件大小
    long size = pbuf->pubseekoff(0, ios::end, ios::in);
    pbuf->pubseekpos(0, ios::in);
    // 分配内存空间
    /* 分配内存存储整个文件 */
    char *buffer = (char *)malloc(sizeof(char) * size);
    if (buffer == NULL)
    {
        return NULL;
    }
    // 获取文件内容
    long n = pbuf->sgetn(buffer, size);
    assert(n == size);
    *sizeOut = size;
    filestr.close();
    return buffer;
}
//get psi data and save to file
void savePsiToFile(const char *fileName, const vector<oc::u32> &psiMsgIndexs,
                   const vector<vector<oc::u8>> &recvSet)
{
    int psiSize = psiMsgIndexs.size();
    assert(recvSet.size() > 0);
    int ids = recvSet[0].size();
    long bufSize = psiSize * ids + psiSize; //空格也要加上
    printf("===>>ids:%d\n", ids);
    char *buf = (char *)malloc(bufSize);
    assert(buf);
    char *left = buf;
    char space = ' ';
    int count = 0;
    for (int i = 0; i < psiSize; i++)
    {
        memcpy(left, (char *)(recvSet[psiMsgIndexs[i]].data()), ids);
        count += ids;
        left += ids;
        memcpy(left, &space, 1);
        count += 1;
        left += 1;
    }
    printf("===>>new file count:%d\n", count);
    writeFileAllByCPP(fileName, buf, bufSize - 1);
    free(buf);
}
void generateDataFromFile(const char *inFile, vector<vector<oc::u8>> &dataSet, int ids)
{
    long filesize = 0;
    char *bufread = readFileAllByCPP(inFile, &filesize);
    assert(bufread);
    int dataSetSize = dataSet.size();
    char *left = bufread;
    char *end = bufread + filesize;
    int i = 0;
    for (; left < end; left += ids + 1, i++)
    {
        if (i < dataSetSize)
        {
            dataSet[i].resize(ids);
            memcpy((char *)(dataSet[i].data()), left, ids);
        }
    }
    free(bufread);
    printf("===>>i:%d\n", i);
    assert(i == dataSetSize);
}
int main(int argc, char **argv)
{
    // int ptype = atoi(argv[1]); //0 send;1 recv
    // int matrixWidth = atoi(argv[2]);
    // int logHeight = atoi(argv[3]);
    // int senderSize = atoi(argv[4]);
    // int receiverSize = atoi(argv[5]);
    oc::CLP cmd;
    cmd.parse(argc, argv);
    cmd.setDefault("w", 60);
    int matrixWidth = cmd.get<oc::u64>("w");
    printf("w:%d\n", matrixWidth);
    cmd.setDefault("h", 10);
    int logHeight = cmd.get<oc::u64>("h");
    printf("logH:%d\n", logHeight);
    cmd.setDefault("ss", 5000000);
    int senderSize = cmd.get<oc::u64>("ss");
    printf("ss:%d\n", senderSize);
    cmd.setDefault("rs", 5000000);
    int receiverSize = cmd.get<oc::u64>("rs");
    printf("rs:%d\n", receiverSize);
    //数据id的长度
    cmd.setDefault("ids", 18);
    int ids = cmd.get<oc::u64>("ids");
    printf("ids:%d,length of id\n", ids);
    //命令行传种子
    cmd.setDefault("sd", 0);
    seed = cmd.get<oc::u64>("sd");
    printf("sd:%d\n", seed);
    //数据文件路径
    cmd.setDefault("in", "");
    string inFile = cmd.get<string>("in");
    printf("in:%s,input file\n", inFile.c_str());
    //保存数据文件路径
    cmd.setDefault("out", "");
    string outFile = cmd.get<string>("out");
    printf("out:%s,output file\n", outFile.c_str());
    // int hash2LengthInBytes = atoi(argv[6]);
    int hash2LengthInBytes = 10;
    // int bucket2ForComputeH2Output = atoi(argv[7]);
    int bucket2ForComputeH2Output = 256;
    bucket2ForComputeH2Output = 512;
    bucket2ForComputeH2Output = 10240;
    //ip地址
    cmd.setDefault("ip", "127.0.0.1");
    string address = cmd.get<string>("ip");
    printf("ip:%s\n", address.c_str());
    //port端口
    cmd.setDefault("port", 7878);
    int port = cmd.get<oc::u64>("port");
    printf("port:%d\n", port);
    if (!cmd.isSet("r"))
    {
        std::cout << "=================================\n"
                  << "||  Private Set Intersection   ||\n"
                  << "=================================\n"
                  << "\n"
                  << "This program reports the performance of the private set "
                     "intersection protocol.\n"
                  << "\n"
                  << "Experimenet flag:\n"
                  << " -r 0    to run a sender.\n"
                  << " -r 1    to run a receiver.\n"
                  << "\n"
                  << "Parameters:\n"
                  << " -ss     the size of dataset on sender side.\n"
                  << " -rs     the size of dataset on receiver side.\n"
                  << " -ids    the size of an ID for bytes.\n"
                  << " -w      width of the matrix.\n"
                  << " -h      log(height) of the matrix.\n"
                  << " -sd     common seed (optional) for sender and receiver.\n"
                  << " -in     the input file path .\n"
                  << " -out    the output file path saving psi-set.\n"
                  << " -ip     ip address .\n"
                  << " -port   port.\n";
        return 0;
    }
    int ptype = cmd.get<oc::u64>("r");
    printf("r:%d\n", ptype);
    oc::block commonSeed = oc::toBlock(0x333333 + seed);
    if (ptype == CLIENT)
    {
        //生成sendSet
        void *timeCompute = newTimeCompute();
        assert(timeCompute);
        startTime(timeCompute);
        vector<vector<oc::u8>> sendSet;
        sendSet.resize(senderSize);
        if (inFile == "")
        {
            generateDataSet(0, senderSize, seed, sendSet);
        }
        else
        {
            generateDataFromFile(inFile.c_str(), sendSet, ids);
        }
        int useTime = getEndTime(timeCompute);
        printf("生成发送者数据集:%dms\n", useTime);
        //生成sendSet end
        //printOTMsgSingle(sendSet);
        oc::PsiSender psiSender;
        oc::block localSeed = oc::toBlock(0x111111 + seed);
        //初始化一个socket连接
        void *client = initChannel(CLIENT, address.c_str(), port);
        assert(client);
        //psi 发送方
        startTime(timeCompute);
        int fg = psiSender.init(commonSeed, localSeed, matrixWidth, logHeight, senderSize,
                                hash2LengthInBytes, bucket2ForComputeH2Output);
        assert(fg == 0);
        //[1]生成基本ot协议的公共参数，并发送给对方
        oc::u8 *pubParamBuf = nullptr;
        oc::u64 pubParamBufSize = 0;
        fg = psiSender.genPublicParamFromNpot(&pubParamBuf, pubParamBufSize);
        assert(fg == 0);
        int n = send_data(client, (char *)pubParamBuf, pubParamBufSize);
        assert(n == pubParamBufSize);
        //[2]接收对方发过来的pk0sBuff
        char *pk0sBuf = nullptr;
        n = recv_data(client, &pk0sBuf);
        assert(n > 0);
        oc::u8 *uBuffOutput = nullptr;
        oc::u64 uBuffOutputSize = 0;
        //[3]输入pk0s，生成uBuffOutput,并发送给对方
        fg = psiSender.genMatrixTxorRBuff((oc::u8 *)pk0sBuf, n, &uBuffOutput, uBuffOutputSize);
        assert(fg == 0);
        n = send_data(client, (char *)uBuffOutput, uBuffOutputSize);
        assert(n == uBuffOutputSize);
        //[4]接收MatrixAxorD,生成矩阵C
        char *matrixAxorD = nullptr;
        n = recv_data(client, &matrixAxorD);
        assert(n > 0);
        // assert(matrixAxorD);
        fg = psiSender.recoverMatrixC((oc::u8 *)matrixAxorD, n, sendSet);
        assert(fg == 0);
        //循环发送hash输出给对方
        char *hashOutputOnceBuff = nullptr;
        oc::u64 hashOutputOnceBuffSize = 0;
        int totalCyc = senderSize / bucket2ForComputeH2Output;
        for (auto low = 0; low < senderSize; low += bucket2ForComputeH2Output)
        {
            auto up = low + bucket2ForComputeH2Output < senderSize ? low + bucket2ForComputeH2Output : senderSize;
            fg = psiSender.computeHashOutputToReceiverOnce(low, up, (oc::u8 **)&hashOutputOnceBuff, hashOutputOnceBuffSize);
            assert(fg == 0);
            n = send_data(client, hashOutputOnceBuff, hashOutputOnceBuffSize);
            assert(n == hashOutputOnceBuffSize);
        }
        useTime = getEndTime(timeCompute);
        printf("发送方用时：%dms,totalCyc:%d\n", useTime, totalCyc);
        // printf("----------->>>>>>main over \n");
        // //释放mem, sender
        releaseTimeCompute(timeCompute);
        releaseChannel(client);
        printf("----------->>>>>>main over \n");
        return 0;
    }
    if (ptype == SERVER) //recv
    {
        //生成recvSet
        void *timeCompute = newTimeCompute();
        assert(timeCompute);
        startTime(timeCompute);
        vector<vector<oc::u8>> recvSet;
        recvSet.resize(receiverSize);
        // generateDataSet(1, receiverSize, seed, recvSet);
        if (inFile == "")
        {
            generateDataSet(1, receiverSize, seed, recvSet);
        }
        else
        {
            generateDataFromFile(inFile.c_str(), recvSet, ids);
        }
        int useTime = getEndTime(timeCompute);
        printf("生成接收者数据集:%dms\n", useTime);
        //生成recvSet end
        //printOTMsgSingle(recvSet);
        oc::PsiReceiver psiRecv;
        oc::block localSeed = oc::toBlock(0x222222 + seed);
        //初始化一个socket连接
        void *server = initChannel(SERVER, address.c_str(), port);
        assert(server);
        startTime(timeCompute);
        //初始化psiRecv
        int fg = psiRecv.init(commonSeed, localSeed, matrixWidth, logHeight,
                              receiverSize, hash2LengthInBytes, bucket2ForComputeH2Output);
        assert(fg == 0);
        //接收对方的公共参数,并生成 pk0sBuf
        char *pubParamBuf = nullptr;
        char *pk0sBuf = nullptr;
        oc::u64 pk0sBufSize = 0;
        int n = recv_data(server, &pubParamBuf);
        assert(n > 0);
        fg = psiRecv.genPK0FromNpot((oc::u8 *)pubParamBuf, n, (oc::u8 **)&pk0sBuf, pk0sBufSize);
        assert(fg == 0);
        n = send_data(server, pk0sBuf, pk0sBufSize);
        //接收 uBuffInput,并生成matrixAD,并发送给对方
        char *uBuffInput = nullptr;
        char *matrixADBuff = nullptr;
        oc::u64 matrixADBuffSize = 0;
        n = recv_data(server, &uBuffInput);
        assert(n > 0);
        fg = psiRecv.getSendMatrixADBuff((oc::u8 *)uBuffInput, n, recvSet,
                                         (oc::u8 **)&matrixADBuff, matrixADBuffSize);
        assert(fg == 0);
        n = send_data(server, matrixADBuff, matrixADBuffSize);
        //本方生成hashMap
        void *timeHashMapNeed = newTimeCompute();
        assert(timeHashMapNeed);
        startTime(timeHashMapNeed);
        fg = psiRecv.genenateAllHashesMap();
        assert(fg == 0);
        int time1 = getEndTime(timeHashMapNeed);
        releaseTimeCompute(timeHashMapNeed);
        printf("生成hashMap表所需时间:%dms\n", time1);
        //循环接收对方发来的hashOutput
        char *hashOutput = nullptr;
        vector<oc::u32> psiMsgIndexs;
        int totalCyc = senderSize / bucket2ForComputeH2Output;
        for (auto low = 0; low < senderSize; low += bucket2ForComputeH2Output)
        {
            auto up = low + bucket2ForComputeH2Output < senderSize ? low + bucket2ForComputeH2Output : senderSize;
            n = recv_data(server, &hashOutput);
            assert(n > 0);
            fg = psiRecv.recvFromSenderAndComputePSIOnce((oc::u8 *)hashOutput,
                                                         n, low, up, psiMsgIndexs);
            // printf("fg=>%d\n", fg);
            assert(fg == 0);
        }
        useTime = getEndTime(timeCompute);
        printf("接收方用时：%dms,totalCyc:%d\n", useTime, totalCyc);
        // for (int i = 0; i < psiMsgIndexs.size(); i++)
        // {
        //     cout << "i:" << i << "," << recvSet[i] << endl;
        // }
        printf("psi count:%d\n", psiMsgIndexs.size());
        // assert(psiMsgIndexs.size() == Psi_Size);
        if (outFile != "")
        {
            savePsiToFile(outFile.c_str(), psiMsgIndexs, recvSet);
            int useTime1 = getEndTime(timeCompute);
            printf("psi数据保存到文件：%dms\n", useTime1 - useTime);
        }

        // //释放mem
        releaseTimeCompute(timeCompute);
        releaseChannel(server);
        printf("===>>main end\n");
        return 0;
    }

    return 0;
}
#endif
