#include "psi.h"
#include <cryptoTools/Crypto/RandomOracle.h>
namespace osuCrypto
{
    //common function
    //将所有输入的数据以相同的方式H1做映射，以dataSetOutput返回
    void transformInputByH1(const AES &commonAes, const u64 h1LengthInBytes,
                            const vector<block> &dataSetInput,
                            block *&dataSetOutput)
    {
        u64 dataSetInputSize = dataSetInput.size();
        block *aesInput = new block[dataSetInputSize];
        block *aesOutput = new block[dataSetInputSize];
        RandomOracle H1(h1LengthInBytes); // 32bytes
        u8 h1Output[h1LengthInBytes];     // 32bytes
        for (auto i = 0; i < dataSetInputSize; ++i)
        { // 256个元素
            H1.Reset();
            //对每一个y属于dataSetInput，映射成一个hash值，32字节（H1,H2）
            H1.Update((u8 *)(dataSetInput.data() + i), sizeof(block));
            H1.Final(h1Output);
            // H1
            aesInput[i] = *(block *)h1Output; //前16字节，后16字节
            // H2
            dataSetOutput[i] = *(block *)(h1Output + sizeof(block));
        }
        commonAes.ecbEncBlocks(aesInput, dataSetInputSize, aesOutput);
        //生成dataSetOutput数据。文章中H1(y)
        for (auto i = 0; i < dataSetInputSize; ++i)
        {
            dataSetOutput[i] ^= aesOutput[i];
        }
        delete[] aesInput;
        delete[] aesOutput;
    }
    //psiReceiver
    PsiReceiver::PsiReceiver() {}
    PsiReceiver::~PsiReceiver() {}
    int PsiReceiver::init(const block &commonSeed, const block &localSeed,
                          u64 matrixWidth, u64 logHeight, u64 receiverSize)
    {
        this->matrixWidth = matrixWidth;
        this->matrixWidthInBytes = (matrixWidth + 7) >> 3;
        this->receiverSize = receiverSize;
        this->receiverSizeInBytes = (receiverSize + 7) >> 3;
        this->logHeight = logHeight;
        this->height = 1 << this->logHeight;
        this->heightInBytes = (this->height + 7) >> 3; //除以8
        this->bucket1 = 256;
        this->bucket2 = 256;
        this->h1LengthInBytes = 32;
        //todo
        this->hash2LengthInBytes = 16;
        this->sendMatrixADBuff = (u8 *)malloc(this->heightInBytes * this->matrixWidth);
        if (this->sendMatrixADBuff == NULL)
        {
            return -1;
        }
        this->transHashInputs.resize(this->matrixWidth);
        for (auto i = 0; i < this->matrixWidth; ++i)
        {
            this->transHashInputs[i] = new u8[this->receiverSizeInBytes];
            memset(this->transHashInputs[i], 0, this->receiverSizeInBytes);
        }
        this->encMsgOutput.resize(this->matrixWidth);
        cout << "===>commonSeed before:" << this->commonSeed << endl;
        this->commonSeed = commonSeed;
        cout << "===>commonSeed after :" << this->commonSeed << endl;
        PRNG localRng(localSeed);
        return this->iknpOteSender.init(localRng);
    }
    //内存释放
    void PsiReceiver::releasePsiReceiver()
    {
        if (this->sendMatrixADBuff)
        {
            free(this->sendMatrixADBuff);
            this->sendMatrixADBuff = NULL;
        }
        for (auto i = 0; i < this->matrixWidth; ++i)
        {
            delete[] this->transHashInputs[i];
        }
    }
    int PsiReceiver::genPK0FromNpot(u8 *pubParamBuf, const u64 pubParamBufByteSize,
                                    u8 **pk0Buf, u64 &pk0BufSize)
    {
        return this->iknpOteSender.genPK0FromNpot(pubParamBuf, pubParamBufByteSize,
                                                  pk0Buf, pk0BufSize);
    }
    int PsiReceiver::getSendMatrixADBuff(const u8 *uBuffInput, const int uBuffInputSize,
                                         const vector<block> &receiverSet,
                                         u8 **sendMatrixADBuff, u64 &sendMatixADBuffSize)
    {
        int ublocksize = uBuffInputSize / sizeof(block);
        vector<block> uBuffInputBlock(ublocksize);
        block *begin = (block *)uBuffInput;
        for (int i = 0; i < ublocksize; i++)
        {
            uBuffInputBlock[i] = *(begin + i);
        }
        //生成encmsgoutput
        int fg = this->iknpOteSender.getEncMsg(uBuffInputBlock, this->encMsgOutput);
        if (fg != 0)
        {
            return fg;
        }
        //psi begin
        // u32 locationInBytes = (this->logHeight + 7) / 8;
        u64 locationInBytes = (this->logHeight + 7) >> 3;
        u64 widthBucket1 = sizeof(block) / locationInBytes;
        u64 shift = (1 << this->logHeight) - 1; //全1
        //////////// Initialization ///////////////////
        PRNG commonPrng(this->commonSeed);
        block commonKey;
        AES commonAes;
        u8 *matrixA[widthBucket1];
        u8 *matrixDelta[widthBucket1];
        for (auto i = 0; i < widthBucket1; ++i)
        {
            matrixA[i] = new u8[this->heightInBytes]; //要释放
            matrixDelta[i] = new u8[this->heightInBytes];
        }
        // u64 receiverSize = receiverSet.size();
        // u64 receiverSizeInBytes = (receiverSize + 7) / 8;
        // u64 receiverSizeInBytes = (receiverSize + 7) >> 3;
        if (receiverSet.size() != this->receiverSize)
        {
            return -111;
        }
        u8 *transLocations[widthBucket1];
        for (auto i = 0; i < widthBucket1; ++i)
        {
            transLocations[i] = new u8[this->receiverSize * locationInBytes + sizeof(u32)];
        }
        block randomLocations[this->bucket1]; // 256block
        /////////// Transform input /////////////////////
        commonPrng.get((u8 *)&commonKey, sizeof(block));
        commonAes.setKey(commonKey);
        block *recvSet = new block[receiverSize];
        transformInputByH1(commonAes, this->h1LengthInBytes, receiverSet, recvSet);
        ////////// Transform input end //////////////////
        /*********for cycle start*********/
        for (auto wLeft = 0; wLeft < this->matrixWidth; wLeft += widthBucket1)
        {
            auto wRight = wLeft + widthBucket1 < this->matrixWidth ? wLeft + widthBucket1 : this->matrixWidth;
            auto w = wRight - wLeft;
            //////////// Compute random locations (transposed) ////////////////
            commonPrng.get((u8 *)&commonKey, sizeof(block));
            commonAes.setKey(commonKey);
            for (auto low = 0; low < receiverSize; low += this->bucket1)
            {
                auto up = low + this->bucket1 < receiverSize ? low + this->bucket1 : receiverSize;
                //每256个输入处理一次，randomLocations==256blocks,Fk函数，Fk(H1(y))
                commonAes.ecbEncBlocks(recvSet + low, up - low, randomLocations);
                //如果w比较宽，这里的计算会增加
                for (auto i = 0; i < w; ++i)
                {
                    for (auto j = low; j < up; ++j)
                    {
                        // randomLocations,256个block
                        memcpy(transLocations[i] + j * locationInBytes,
                               (u8 *)(randomLocations + (j - low)) + i * locationInBytes,
                               locationInBytes);
                    }
                }
            }
            //////////// Compute matrix Delta /////////////////////////////////
            //对应论文中的P2(接收者)
            for (auto i = 0; i < widthBucket1; ++i)
            {
                memset(matrixDelta[i], 255, this->heightInBytes);
                // heightInBytes，置为全1矩阵
            }
            //本方拥有的数据y的个数
            for (auto i = 0; i < w; ++i)
            {
                for (auto j = 0; j < receiverSize; ++j)
                {
                    auto location =
                        (*(u32 *)(transLocations[i] + j * locationInBytes)) & shift;
                    // shift全1
                    // location >> 3(除以8)表示matrixDelta[i]的字节位置
                    // location & 0b0111,取出低3位；(location & 7)==0,1,2,3,4,5,6,7
                    matrixDelta[i][location >> 3] &= ~(1 << (location & 7));
                }
            }
            //////////////// Compute matrix A & sent matrix ///////////////////////
            // u8 *sentMatrix[w];
            u64 offset1 = wLeft * this->heightInBytes;
            u64 offset2 = 0;
            for (auto i = 0; i < w; ++i)
            {
                PRNG prng(this->encMsgOutput[i + wLeft][0]);
                prng.get(matrixA[i], this->heightInBytes);
                // sentMatrix[i] = new u8[this->heightInBytes];
                prng.SetSeed(this->encMsgOutput[i + wLeft][1]);
                // prng.get(sentMatrix[i], this->heightInBytes);
                prng.get(this->sendMatrixADBuff + offset1 + offset2, this->heightInBytes);
                for (auto j = 0; j < this->heightInBytes; ++j)
                {
                    // sentMatrix[i][j] ^= matrixA[i][j] ^ matrixDelta[i][j];
                    (this->sendMatrixADBuff + offset1 + offset2)[j] ^= matrixA[i][j] ^ matrixDelta[i][j];
                }
                //发送sM^A^D
                //发送数据U^A^D给对方，
                // ch.asyncSend(sentMatrix[i], heightInBytes);
                //偏移计算
                offset2 += this->heightInBytes;
            }
            ///////////////// Compute hash inputs (transposed) /////////////////////
            for (auto i = 0; i < w; ++i)
            {
                for (auto j = 0; j < receiverSize; ++j)
                {
                    auto location =
                        (*(u32 *)(transLocations[i] + j * locationInBytes)) & shift;
                    this->transHashInputs[i + wLeft][j >> 3] |=
                        (u8)((bool)(matrixA[i][location >> 3] & (1 << (location & 7))))
                        << (j & 7);
                }
            }
        }
        /*********for cycle end*********/
        //将uBuff输出并发送给对方
        *sendMatrixADBuff = this->sendMatrixADBuff;
        sendMatixADBuffSize = this->heightInBytes * this->matrixWidth;
        /****************************/
        // 最后释放空间
        for (auto i = 0; i < widthBucket1; ++i)
        {
            delete[] matrixA[i];
            delete[] matrixDelta[i];
            delete[] transLocations[i];
        }
        delete[] recvSet;
        return 0;
    }
    //将sendMatrixADBuff发送给对方之后，接下来生成AllHashMap
    int PsiReceiver::genenateAllHashesMap()
    {
        /////////////////// Compute hash outputs ///////////////////////////
        //H2
        RandomOracle H(this->hash2LengthInBytes);
        u8 hashOutput[sizeof(block)];
        u8 *hashInputs[this->bucket2];
        for (auto i = 0; i < this->bucket2; ++i)
        {
            hashInputs[i] = new u8[this->matrixWidthInBytes];
        }
        //接收集合中的每个元素
        for (auto low = 0; low < receiverSize; low += bucket2)
        {
            auto up = low + bucket2 < receiverSize ? low + bucket2 : receiverSize;
            for (auto j = low; j < up; ++j)
            {
                memset(hashInputs[j - low], 0, this->matrixWidthInBytes);
            }
            for (auto i = 0; i < this->matrixWidth; ++i)
            {
                for (auto j = low; j < up; ++j)
                {
                    hashInputs[j - low][i >> 3] |=
                        (u8)((bool)(this->transHashInputs[i][j >> 3] & (1 << (j & 7))))
                        << (i & 7);
                }
            }
            for (auto j = low; j < up; ++j)
            {
                H.Reset();
                H.Update(hashInputs[j - low], this->matrixWidthInBytes);
                H.Final(hashOutput);
                //生成一个map并保存
                this->allHashes[*(u64 *)hashOutput].push_back(
                    std::make_pair(*(block *)hashOutput, j));
            }
        }
        for (auto i = 0; i < this->bucket2; ++i)
        {
            delete[] hashInputs[i];
        }
        return 0;
    }
    //psiSender
}