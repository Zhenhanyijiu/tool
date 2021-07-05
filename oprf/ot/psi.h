#include "iknpote.h"
#include <unordered_map>
namespace osuCrypto
{
    //psi 发送方结构定义如下
    class PsiSender
    {
    private:
    public:
        PsiSender();
        ~PsiSender();
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
        u64 bucket2;
        u64 h1LengthInBytes;
        u64 hash2LengthInBytes;
        vector<array<block, 2>> encMsgOutput; //可能用不着
        block commonSeed;
        u8 *sendMatrixADBuff;
        vector<u8 *> transHashInputs;
        std::unordered_map<u64, std::vector<std::pair<block, u32>>> allHashes;
        IknpOtExtSender iknpOteSender;

    public:
        PsiReceiver();
        ~PsiReceiver();
        //初始化psiReceiver
        int init(const block &commonSeed, const block &localSeed,
                 u64 matrixWidth, u64 logHeight, u64 receiverSizeInBytes);
        void releasePsiReceiver();
        //接收对方发送过来的公共参数，并根据baseot choices输入生成pk0s,并发送过对方
        int genPK0FromNpot(u8 *pubParamBuf, const u64 pubParamBufByteSize,
                           u8 **pk0Buf, u64 &pk0BufSize);
        //uBuffInput对方发送过来的，作为输入，encMsgOutput为输出，大小在外部初始化
        // int getEncMsg(const vector<block> &uBuffInput, vector<array<block, 2>> &encMsgOutput);
        int getSendMatrixADBuff(const u8 *uBuffInput, const int uBuffInputSize,
                                const vector<block> &receiverSet,
                                u8 **sendMatrixADBuff, u64 &sendMatixADBuffSize);
        int genenateAllHashesMap();
    };
}