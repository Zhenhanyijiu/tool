#include "iknpote.h"
#include <unordered_map>
namespace osuCrypto
{
    // typedef unsigned long long u64_t;
    typedef uint64_t u64_t;
    // typedef block block_t;
    typedef unsigned char u8_t;
    // typedef unsigned int u32_t;
    typedef uint32_t u32_t;
    //psi 发送方结构定义如下
    class PsiSender
    {
    private:
        u64 matrixWidth;
        u64 matrixWidthInBytes;
        block commonSeed;
        u64 logHeight;
        u64 height;
        u64 heightInBytes;
        u64 senderSize;
        u64 senderSizeInBytes;
        u64 bucket1;
        u64 bucket2ForComputeH2Output;
        u64 h1LengthInBytes;
        u64 hash2LengthInBytes;
        BitVector choicesWidthInput;
        vector<block> recoverMsgWidthOutput;
        vector<block> uBuffOutput;
        // vector<u8> hashOutputBuff;
        vector<vector<u8>> transHashInputs;
        vector<vector<u8>> hashInputs;
        IknpOtExtReceiver iknpOteReceiver;
        vector<vector<u8>> senderSet;
        u64 lowL;
        // u64 upR;

    public:
        PsiSender();
        // ~PsiSender();
        //初始化psiSender
        int init(u8_t *commonSeed, u64_t senderSize, u64_t matrixWidth, u64_t logHeight,
                 u64_t hash2LengthInBytes = 10, u64_t bucket2ForComputeH2Output = 256);

        //生成基本ot协议的公共参数，并发送给对方
        int genPublicParamFromNpot(u8_t **pubParamBuf, u64_t *pubParamBufByteSize);
        //接收对方pk0Buf；并生成T^R的结果uBuffOutput，发送给对方
        int genMatrixTxorRBuff(u8_t *pk0Buf, const u64_t pk0BufSize,
                               u8_t **uBuffOutputTxorR, u64_t *uBuffOutputSize);
        //接收recvMatrixADBuff，恢复出矩阵C
        int recoverMatrixC(const u8_t *recvMatrixADBuff, const u64_t recvMatixADBuffSize,
                           const vector<vector<u8_t>> senderSet);
        //判断发送方发送数据是否结束
        int isSendEnd();
        //循环计算hash输出并发送给对方
        /*
        for (auto low = 0; low < senderSize; low += bucket2) {
            auto up = low + bucket2 < senderSize ? low + bucket2 : senderSize;
            调用：
            computeHashOutputToReceiverOnce(const u64 low, const u64 up,
                                            u8 **sendBuff, u64 &sendBuffSize)
        }
        */
        // int computeHashOutputToReceiverOnce(const u64_t low, const u64_t up,
        // u8_t **sendBuff, u64_t *sendBuffSize);
        //hashOutputBuff外部开辟空间，大小为10*bucket2ForComputeH2Output
        int computeHashOutputToReceiverOnce(u8_t *hashOutputBuff, u64_t *sendBuffSize);
    };
    //psi 接收方结构定义如下
    class PsiReceiver
    {
    private:
        //目前w<=512*1024
        u64 matrixWidth;
        u64 matrixWidthInBytes;
        u64 receiverSize;
        u64 senderSize;
        u64 receiverSizeInBytes;
        u64 logHeight;
        u64 height;
        u64 heightInBytes;
        u64 bucket1;
        u64 bucket2ForComputeH2Output;
        u64 h1LengthInBytes;
        u64 hash2LengthInBytes;
        vector<array<block, 2>> encMsgOutput; //可能用不着
        block commonSeed;
        vector<u8> sendMatrixADBuff;
        vector<vector<u8>> transHashInputs;
        std::unordered_map<u64, std::vector<std::pair<block, u32_t>>> allHashes;
        IknpOtExtSender iknpOteSender;
        u64 lowL;

    public:
        PsiReceiver();
        // ~PsiReceiver();
        //初始化psiReceiver
        //commonSeed,localSeed 长度为16字节
        int init(u8_t *commonSeed, u64_t receiverSize, u64_t senderSize,
                 u64_t matrixWidth, u64_t logHeight, u64_t hash2LengthInBytes = 10,
                 u64_t bucket2ForComputeH2Output = 256);
        //接收对方发送过来的公共参数，并根据baseot choices输入生成pk0s,并发送过对方
        int genPK0FromNpot(u8_t *pubParamBuf, const u64_t pubParamBufByteSize,
                           u8_t **pk0Buf, u64_t *pk0BufSize);
        //uBuffInput对方发送过来的，作为输入，encMsgOutput为输出，大小在外部初始化
        // int getEncMsg(const vector<block> &uBuffInput, vector<array<block, 2>> &encMsgOutput);
        int getSendMatrixADBuff(const u8_t *uBuffInputTxorR, const u64_t uBuffInputSize,
                                const vector<vector<u8_t>> receiverSet,
                                u8_t **sendMatrixADBuff, u64_t *sendMatixADBuffSize);
        //将sendMatrixADBuff发送给对方之后，接下来生成AllHashMap
        int genenateAllHashesMap();
        //判断接收方接收数据是否结束
        int isRecvEnd();
        // Receive hash outputs from sender and compute PSI
        /*
        for (auto low = 0; low < senderSize; low += bucket2) {
            auto up = low + bucket2 < senderSize ? low + bucket2 : senderSize;
            调用：
            recvFromSenderAndComputePSIOnce(const u8 *recvBuff, const u64 recvBufSize,
                                            u64 low, u64 up, vector<int> &psiMsgIndex)
        }
        */
        int recvFromSenderAndComputePSIOnce(const u8_t *recvBuff, const u64_t recvBufSize,
                                            vector<u32_t> *psiMsgIndex);
    };
    // void generateDataSetDebug(const int ptype, const u64_t dataSize, const u64_t psiSize,
    //                           u64_t seed, u64_t ids, vector<vector<u8_t>> *dataSet);
}