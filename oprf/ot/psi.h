#include "iknpote.h"
#include <unordered_map>
namespace osuCrypto
{
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
        vector<u8> hashOutputBuff;
        vector<vector<u8>> transHashInputs;
        vector<vector<u8>> hashInputs;
        IknpOtExtReceiver iknpOteReceiver;

    public:
        PsiSender();
        ~PsiSender();
        //初始化psiSender
        int init(block commonSeed, block localSeed,
                 u64 matrixWidth, u64 logHeight, u64 senderSize,
                 u64 hash2LengthInBytes = 10, u64 bucket2ForComputeH2Output = 256);
        //生成基本ot协议的公共参数，并发送给对方
        int genPublicParamFromNpot(u8 **pubParamBuf, u64 &pubParamBufByteSize);
        //接收对方pk0Buf；并生成T^R的结果uBuffOutput，发送给对方
        int genMatrixTxorRBuff(u8 *pk0Buf, const u64 pk0BufSize,
                               u8 **uBuffOutput, u64 &uBuffOutputSize);
        //接收recvMatrixADBuff，恢复出矩阵C
        int recoverMatrixC(const u8 *recvMatrixADBuff, const u64 recvMatixADBuffSize,
                           const vector<block> &senderSet);
        //循环计算hash输出并发送给对方
        /*
        for (auto low = 0; low < senderSize; low += bucket2) {
            auto up = low + bucket2 < senderSize ? low + bucket2 : senderSize;
            调用：
            computeHashOutputToReceiverOnce(const u64 low, const u64 up,
                                            u8 **sendBuff, u64 &sendBuffSize)
        }
        */
        int computeHashOutputToReceiverOnce(const u64 low, const u64 up,
                                            u8 **sendBuff, u64 &sendBuffSize);
    };
    //psi 接收方结构定义如下
    class PsiReceiver
    {
    private:
        //目前w<=512*1024
        u64 matrixWidth;
        u64 matrixWidthInBytes;
        u64 receiverSize;
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
        std::unordered_map<u64, std::vector<std::pair<block, u32>>> allHashes;
        IknpOtExtSender iknpOteSender;

    public:
        PsiReceiver();
        ~PsiReceiver();
        //初始化psiReceiver
        int init(block commonSeed, block localSeed,
                 u64 matrixWidth, u64 logHeight, u64 receiverSize,
                 u64 hash2LengthInBytes = 10, u64 bucket2ForComputeH2Output = 256);
        //接收对方发送过来的公共参数，并根据baseot choices输入生成pk0s,并发送过对方
        int genPK0FromNpot(u8 *pubParamBuf, const u64 pubParamBufByteSize,
                           u8 **pk0Buf, u64 &pk0BufSize);
        //uBuffInput对方发送过来的，作为输入，encMsgOutput为输出，大小在外部初始化
        // int getEncMsg(const vector<block> &uBuffInput, vector<array<block, 2>> &encMsgOutput);
        int getSendMatrixADBuff(const u8 *uBuffInput, const int uBuffInputSize,
                                const vector<block> &receiverSet,
                                u8 **sendMatrixADBuff, u64 &sendMatixADBuffSize);
        //将sendMatrixADBuff发送给对方之后，接下来生成AllHashMap
        int genenateAllHashesMap();
        // Receive hash outputs from sender and compute PSI
        /*
        for (auto low = 0; low < senderSize; low += bucket2) {
            auto up = low + bucket2 < senderSize ? low + bucket2 : senderSize;
            调用：
            recvFromSenderAndComputePSIOnce(const u8 *recvBuff, const u64 recvBufSize,
                                            u64 low, u64 up, vector<int> &psiMsgIndex)
        }
        */
        int recvFromSenderAndComputePSIOnce(const u8 *recvBuff, const u64 recvBufSize,
                                            const u64 low, const u64 up, vector<int> &psiMsgIndex);
    };
}