#include "iknpote.h"
#include "ThreadPool.h"
#include <unordered_map>
#include <vector>
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
        int threadNumOmp;
        u64 matrixWidth;
        u64 matrixWidthInBytes;
        // block commonSeed;
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
        // vector<vector<u8>> senderSet;
        PRNG *commonPrng;
        //对所有输入计算hash1,存到此处
        block *sendSet;
        u64 lowL;
        // u64 upR;

    public:
        PsiSender();
        ~PsiSender();
        //初始化psiSender
        int init(u8_t *commonSeed, u64_t senderSize, u64_t matrixWidth, u64_t logHeight,
                 int threadNum = 1, u64_t hash2LengthInBytes = 10,
                 u64_t bucket2ForComputeH2Output = 256);

        //生成基本ot协议的公共参数，并发送给对方
        int genPublicParamFromNpot(u8_t **pubParamBuf, u64_t *pubParamBufByteSize);
        //接收对方pk0Buf；并生成T^R的结果uBuffOutput，发送给对方
        int genMatrixTxorRBuff(u8_t *pk0Buf, const u64_t pk0BufSize,
                               u8_t **uBuffOutputTxorR, u64_t *uBuffOutputSize);
        //计算所有id的hash1值，备用
        int computeAllHashOutputByH1(const vector<vector<u8_t>> senderSet);
        //接收recvMatrixADBuff，恢复出矩阵C
        int recoverMatrixC(const u8_t *recvMatrixADBuff, const u64_t recvMatixADBuffSize);
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
        int computeHashOutputToReceiverOnce(u8_t **sendBuff, u64_t *sendBuffSize);
    };
    //psi 接收方结构定义如下
    class PsiReceiver
    {
    private:
        //目前w<=512*1024
        int threadNumOmp;
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
        vector<unordered_map<u64, std::vector<std::pair<block, u32_t>>>> HashMapVector;
        IknpOtExtSender iknpOteSender;
        u64 lowL;
        u32_t indexId;
        ThreadPool *psiComputePool;
        vector<vector<u32_t>> psiResults;
        std::vector<std::future<u32_t>> psiResultsIndex;

    public:
        PsiReceiver();
        ~PsiReceiver();
        //初始化psiReceiver
        //commonSeed,localSeed 长度为16字节
        int init(u8_t *commonSeed, u64_t receiverSize, u64_t senderSize, u64_t matrixWidth,
                 u64_t logHeight, int threadNum = 1, u64_t hash2LengthInBytes = 10,
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
#if (defined NOMP) || (defined OMP_ONLY)
        int recvFromSenderAndComputePSIOnce(const u8_t *recvBuff, const u64_t recvBufSize,
                                            vector<u32_t> *psiMsgIndex);
#endif
#ifdef OMP_POOL
        int recvFromSenderAndComputePSIOnce(const u8_t *recvBuff, const u64_t recvBufSize);
        int getPsiResultsForAll(vector<u32_t> *psiResultsOutput);
#endif
    };
    //集成socket的oprf-psi
#ifdef OMP_POOL
    int oprf_psi_receiver_process(u64_t receiverSize, u64_t senderSize, const char *address,
                                  int port, u8_t *commonSeed, u64_t matrixWidth,
                                  u64_t logHeight, u64_t hash2LengthInBytes,
                                  u64_t bucket2ForComputeH2Output, int omp_num,
                                  vector<vector<u8_t>> receiver_set, vector<u32_t> *psiResultsOutput);
    //发送方不需要知道接收方的集合大小，不过这里也将接收方的集合receiverSize作为一个参数，
    //只是为了接口统一的作用
    int oprf_psi_sender_process(u64_t receiverSize, u64_t senderSize, const char *address,
                                int port, u8_t *commonSeed, u64_t matrixWidth,
                                u64_t logHeight, u64_t hash2LengthInBytes,
                                u64_t bucket2ForComputeH2Output, int omp_num,
                                vector<vector<u8_t>> sender_set);
#endif
}