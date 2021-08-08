#include "psi.h"
#include <assert.h>
#include <sys/time.h>
#include <cryptoTools/Crypto/RandomOracle.h>
#include <cstdint>
namespace osuCrypto
{
    //时间统计us
    long start_time()
    {
        struct timeval start;
        gettimeofday(&start, NULL);
        return start.tv_sec * 1000000 + start.tv_usec;
    }
    long get_use_time(long start_time)
    {
        struct timeval end;
        gettimeofday(&end, NULL);
        long usetime = end.tv_sec * 1000000 + end.tv_usec - start_time;
        return usetime / 1000;
    }
    //common function
    //将所有输入的数据以相同的方式H1做映射，以dataSetOutput返回
    void transformInputByH1(const AES &commonAes, const u64 h1LengthInBytes,
                            const vector<vector<u8>> &dataSetInput,
                            block *&dataSetOutput)
    {
        u64 dataSetInputSize = dataSetInput.size();
        block *aesInput = new block[dataSetInputSize];
        block *aesOutput = new block[dataSetInputSize];
        RandomOracle H1(h1LengthInBytes); // 32bytes
        u8 h1Output[h1LengthInBytes];     // 32bytes
        for (auto i = 0; i < dataSetInputSize; ++i)
        {
            // 256个元素
            H1.Reset();
            //对每一个y属于dataSetInput，映射成一个hash值，32字节（H1,H2）
            H1.Update(dataSetInput[i].data(), dataSetInput[i].size());
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
    // PsiReceiver::~PsiReceiver() {}
    int PsiReceiver::init(u8_t *commonSeed, u64_t receiverSize, u64_t senderSize,
                          u64_t matrixWidth, u64_t logHeight, u64_t hash2LengthInBytes,
                          u64_t bucket2ForComputeH2Output)
    {
        this->matrixWidth = matrixWidth;
        this->matrixWidthInBytes = (matrixWidth + 7) >> 3;
        this->receiverSize = receiverSize;
        this->senderSize = senderSize;
        this->receiverSizeInBytes = (receiverSize + 7) >> 3;
        this->logHeight = logHeight;
        this->height = 1 << this->logHeight;
        this->heightInBytes = (this->height + 7) >> 3; //除以8
        this->bucket1 = 256;
        this->bucket2ForComputeH2Output = bucket2ForComputeH2Output; //default 256
        this->h1LengthInBytes = 32;
        //todo
        this->hash2LengthInBytes = hash2LengthInBytes;
        this->sendMatrixADBuff.resize(this->heightInBytes * this->matrixWidth);
        this->transHashInputs.resize(this->matrixWidth);
        for (auto i = 0; i < this->matrixWidth; ++i)
        {
            this->transHashInputs[i].resize(this->receiverSizeInBytes);
            memset(this->transHashInputs[i].data(), 0, this->receiverSizeInBytes);
        }
        this->encMsgOutput.resize(this->matrixWidth);
        // cout << "===>commonSeed before:" << this->commonSeed << endl;
        this->commonSeed = toBlock((u8 *)commonSeed);

        // cout << "===>commonSeed after :" << this->commonSeed << endl;
        // block localSeedBlock = toBlock((u8 *)localSeed);
        PRNG localRng(this->commonSeed);
        this->lowL = (u64)0;
        printf("===>>hash2LengthInBytes:%ld,bucket2ForComputeH2Output:%ld\n", hash2LengthInBytes, bucket2ForComputeH2Output);
        return this->iknpOteSender.init(localRng);
    }
    int PsiReceiver::genPK0FromNpot(u8_t *pubParamBuf, const u64_t pubParamBufByteSize,
                                    u8_t **pk0Buf, u64_t *pk0BufSize)
    {
        return this->iknpOteSender.genPK0FromNpot(pubParamBuf, pubParamBufByteSize,
                                                  pk0Buf, *pk0BufSize);
    }
    int PsiReceiver::getSendMatrixADBuff(const u8_t *uBuffInput, const u64_t uBuffInputSize,
                                         const vector<vector<u8_t>> receiverSet,
                                         u8_t **sendMatrixADBuff, u64_t *sendMatixADBuffSize)
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
        // void *timeCompute = newTimeCompute();
        // startTime(timeCompute);
        long start0 = start_time();
        transformInputByH1(commonAes, this->h1LengthInBytes, receiverSet, recvSet);
        // long useTimeH1 = getEndTime(timeCompute);
        printf("===>>计算H1用时:%ldms\n", get_use_time(start0));
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
                prng.SetSeed(this->encMsgOutput[i + wLeft][1]);
                // prng.get(sentMatrix[i], this->heightInBytes);
                prng.get(this->sendMatrixADBuff.data() + offset1 + offset2, this->heightInBytes);
                for (auto j = 0; j < this->heightInBytes; ++j)
                {
                    // sentMatrix[i][j] ^= matrixA[i][j] ^ matrixDelta[i][j];
                    (this->sendMatrixADBuff.data() + offset1 + offset2)[j] ^= matrixA[i][j] ^ matrixDelta[i][j];
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
        // long useTimeCycle = getEndTime(timeCompute);
        printf("===>>计算H1之后,生成矩阵A,D用时:%ldms\n", get_use_time(start0));
        /*********for cycle end*********/
        //将uBuff输出并发送给对方
        *sendMatrixADBuff = this->sendMatrixADBuff.data();
        *sendMatixADBuffSize = this->heightInBytes * this->matrixWidth;
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
        u8 *hashInputs[this->bucket2ForComputeH2Output];
        for (auto i = 0; i < this->bucket2ForComputeH2Output; ++i)
        {
            hashInputs[i] = new u8[this->matrixWidthInBytes];
        }
        //接收集合中的每个元素
        for (auto low = 0; low < this->receiverSize; low += this->bucket2ForComputeH2Output)
        {
            auto up = low + this->bucket2ForComputeH2Output < this->receiverSize ? low + this->bucket2ForComputeH2Output : this->receiverSize;
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
        for (auto i = 0; i < this->bucket2ForComputeH2Output; ++i)
        {
            delete[] hashInputs[i];
        }
        return 0;
    }
    //判断接收方接收数据是否结束
    int PsiReceiver::isRecvEnd()
    {
        return this->lowL < this->senderSize ? 0 : 1;
    }
    //PsiReceiver,接收对方发来的hash输出
    int PsiReceiver::recvFromSenderAndComputePSIOnce(const u8_t *recvBuff, const u64_t recvBufSize,
                                                     vector<u32_t> *psiMsgIndex)
    {
        auto up = this->lowL + this->bucket2ForComputeH2Output < this->senderSize ? this->lowL + this->bucket2ForComputeH2Output : this->senderSize;
        if (recvBufSize != (up - this->lowL) * this->hash2LengthInBytes)
        {
            return -122;
        }
        for (auto idx = 0; idx < up - this->lowL; ++idx)
        {
            u64 mapIdx = *(u64 *)(recvBuff + idx * this->hash2LengthInBytes);
            auto found = this->allHashes.find(mapIdx);
            if (found == this->allHashes.end())
                continue;
            //可能找到好几个
            for (auto i = 0; i < found->second.size(); ++i)
            {
                if (memcmp(&(found->second[i].first),
                           recvBuff + idx * this->hash2LengthInBytes,
                           this->hash2LengthInBytes) == 0)
                {
                    psiMsgIndex->push_back(found->second[i].second);
                    break;
                }
            }
        }
        this->lowL += this->bucket2ForComputeH2Output;
        return 0;
    }
    //******************PsiSender*********************//
    PsiSender::PsiSender() {}
    // PsiSender::~PsiSender() {}
    //初始化
    int PsiSender::init(u8_t *commonSeed, u64_t senderSize, u64_t matrixWidth, u64_t logHeight,
                        u64_t hash2LengthInBytes, u64_t bucket2ForComputeH2Output)
    {
        this->matrixWidth = matrixWidth;
        this->matrixWidthInBytes = (matrixWidth + 7) >> 3;
        this->commonSeed = toBlock((u8 *)commonSeed);
        this->logHeight = logHeight;
        this->height = 1 << logHeight;
        this->heightInBytes = (this->height + 7) >> 3; //除以8
        this->senderSize = senderSize;
        this->senderSizeInBytes = (senderSize + 7) >> 3;
        this->bucket1 = 256;
        this->bucket2ForComputeH2Output = bucket2ForComputeH2Output; //default 256
        this->h1LengthInBytes = 32;
        //todo
        this->hash2LengthInBytes = hash2LengthInBytes;
        // block localSeedBlock = toBlock((u8 *)localSeed);
        PRNG localRng(this->commonSeed);
        //初始化一个向量r，长度为width
        this->choicesWidthInput.resize(matrixWidth);
        this->choicesWidthInput.randomize(localRng);
        // this->hashOutputBuff.resize(this->hash2LengthInBytes * this->bucket2ForComputeH2Output);
        this->hashInputs.resize(this->bucket2ForComputeH2Output);
        for (int i = 0; i < this->bucket2ForComputeH2Output; i++)
        {
            this->hashInputs[i].resize(this->matrixWidthInBytes);
        }
        this->transHashInputs.resize(this->matrixWidth);
        for (int i = 0; i < this->matrixWidth; i++)
        {
            this->transHashInputs[i].resize(this->senderSizeInBytes);
            // memset(this->transHashInputs[i].data(), 0, this->senderSizeInBytes);
        }
        this->lowL = (u64)0;
        // this->upR = (u64)0;
        printf("===>>this->lowL:%ld\n", this->lowL);
        return this->iknpOteReceiver.init(localRng);
    }
    //生成公共参数
    int PsiSender::genPublicParamFromNpot(u8_t **pubParamBuf, u64_t *pubParamBufByteSize)
    {
        return this->iknpOteReceiver.genPublicParamFromNpot(pubParamBuf, *pubParamBufByteSize);
    }
    //接收到pk0s,并生成T异或R 将uBuffOutput发送给对方
    int PsiSender::genMatrixTxorRBuff(u8_t *pk0Buf, const u64_t pk0BufSize,
                                      u8_t **uBuffOutput, u64_t *uBuffOutputSize)
    {
        int fg = this->iknpOteReceiver.getEncKeyFromNpot(pk0Buf, pk0BufSize);
        if (fg)
        {
            return fg;
        }
        //这里生成ubuff以及恢复密钥recoverMsgWidthOutput
        fg = this->iknpOteReceiver.genRecoverMsg(this->choicesWidthInput,
                                                 this->recoverMsgWidthOutput, this->uBuffOutput);
        if (fg)
        {
            return fg;
        }
        *uBuffOutput = (u8 *)this->uBuffOutput.data();
        *uBuffOutputSize = this->uBuffOutput.size() * sizeof(block);
        return 0;
    }
    //
    int PsiSender::recoverMatrixC(const u8_t *recvMatrixADBuff, const u64_t recvMatixADBuffSize,
                                  const vector<vector<u8_t>> senderSet)
    {
        auto locationInBytes = (this->logHeight + 7) / 8;    // logHeight==1
        auto widthBucket1 = sizeof(block) / locationInBytes; // 16/1
        u64 shift = (1 << this->logHeight) - 1;              //全1
        ////////////// Initialization //////////////////////
        PRNG commonPrng(this->commonSeed);
        block commonKey;
        AES commonAes;
        u8 *transLocations[widthBucket1]; // 16个u8*
        if (this->senderSize != senderSet.size() || recvMatixADBuffSize != this->matrixWidth * this->heightInBytes)
        {
            return -111;
        }
        for (auto i = 0; i < widthBucket1; ++i)
        {
            // 256*1+4，每个元素为u8*，下面语句为创建一个u8*,260个u8
            transLocations[i] = new u8[this->senderSize * locationInBytes + sizeof(u32)];
        }
        // auto tn = senderSize * locationInBytes + sizeof(u32);
        // cout << "####transLocations中每一列的字节数：" << tn << "\n";
        // bucket1 = bucket2 = 1 << 8;  // 256
        block randomLocations[this->bucket1]; // 256个block
        u8 *matrixC[widthBucket1];            // 16
        for (auto i = 0; i < widthBucket1; ++i)
        {
            matrixC[i] = new u8[this->heightInBytes]; // 32
        }
        // u8 *transHashInputs[this->matrixWidth]; // width==60，矩阵宽度
        for (auto i = 0; i < this->matrixWidth; ++i)
        { // senderSizeInBytes==32
            memset(this->transHashInputs[i].data(), 0, this->senderSizeInBytes);
        }
        /////////// Transform input /////////////////////
        commonPrng.get((u8 *)&commonKey, sizeof(block));
        commonAes.setKey(commonKey);
        block *sendSet = new block[this->senderSize];
        transformInputByH1(commonAes, this->h1LengthInBytes, senderSet, sendSet);
        /////////// Transform input end /////////////////
        cout << "***********************before cycle" << endl;
        /****cycle start***/
        for (auto wLeft = 0; wLeft < this->matrixWidth; wLeft += widthBucket1)
        {
            auto wRight = wLeft + widthBucket1 < this->matrixWidth ? wLeft + widthBucket1 : this->matrixWidth;
            auto w = wRight - wLeft;
            //////////// Compute random locations (transposed) ////////////////
            commonPrng.get((u8 *)&commonKey, sizeof(block));
            commonAes.setKey(commonKey);
            for (auto low = 0; low < this->senderSize; low += bucket1)
            {
                auto up = low + bucket1 < this->senderSize ? low + bucket1 : this->senderSize;
                commonAes.ecbEncBlocks(sendSet + low, up - low, randomLocations);
                for (auto i = 0; i < w; ++i)
                {
                    for (auto j = low; j < up; ++j)
                    {
                        memcpy(transLocations[i] + j * locationInBytes,
                               (u8 *)(randomLocations + (j - low)) + i * locationInBytes,
                               locationInBytes);
                    }
                }
            }
            // cout << "***********************11......" << endl;
            //////////////// Extend OTs and compute matrix C ///////////////////
            // u8 *recvMatrix;
            u64 offset1 = wLeft * this->heightInBytes;
            u64 offset2 = 0;
            for (auto i = 0; i < w; ++i)
            {
                PRNG prng(this->recoverMsgWidthOutput[i + wLeft]);
                prng.get(matrixC[i], this->heightInBytes);
                if (this->choicesWidthInput[i + wLeft])
                {
                    for (auto j = 0; j < heightInBytes; ++j)
                    {
                        matrixC[i][j] ^= (recvMatrixADBuff + offset1 + offset2)[j];
                    }
                }
                offset2 += this->heightInBytes;
            }
            // cout << "***********************22......" << endl;
            ///////////////// Compute hash inputs (transposed) /////////////////////
            for (auto i = 0; i < w; ++i)
            {
                // cout << "***********************33......" << endl;
                for (auto j = 0; j < senderSize; ++j)
                {
                    auto location =
                        (*(u32 *)(transLocations[i] + j * locationInBytes)) & shift;
                    this->transHashInputs[i + wLeft][j >> 3] |=
                        (u8)((bool)(matrixC[i][location >> 3] & (1 << (location & 7))))
                        << (j & 7);
                }
            }
        }
        /****cycle end ****/
        //**************释放内存*************//
        cout << "***********************before cycle end" << endl;
        for (auto i = 0; i < widthBucket1; ++i)
        {
            delete[] transLocations[i];
            delete[] matrixC[i];
            // printf(">>>>>>>>>i:%d,widthBucket1:%d\n", i, widthBucket1);
        }
        delete[] sendSet;
        printf(">>>>>>>>>> sendSet,end\n");
        return 0;
    }
    //
    int PsiSender::isSendEnd()
    {
        // printf("===>>in issend,lowL:%ld\n", this->lowL);
        return this->lowL < this->senderSize ? 0 : 1;
    }
    //计算本方的hash输出并发送给对方
    int PsiSender::computeHashOutputToReceiverOnce(u8_t *hashOutputBuff, u64_t *sendBuffSize)
    {
        auto upR = this->lowL + this->bucket2ForComputeH2Output < this->senderSize ? this->lowL + this->bucket2ForComputeH2Output : this->senderSize;
        //H2
        RandomOracle H(this->hash2LengthInBytes);
        u8 hashOutput[sizeof(block)];
        for (auto j = this->lowL; j < upR; ++j)
        {
            memset(this->hashInputs[j - this->lowL].data(), 0, this->matrixWidthInBytes);
        }
        for (auto i = 0; i < this->matrixWidth; ++i)
        {
            for (auto j = this->lowL; j < upR; ++j)
            {
                this->hashInputs[j - this->lowL][i >> 3] |=
                    (u8)((bool)(this->transHashInputs[i][j >> 3] & (1 << (j & 7))))
                    << (i & 7);
            }
        }
        for (auto j = this->lowL; j < upR; ++j)
        {
            H.Reset();
            H.Update(this->hashInputs[j - this->lowL].data(), this->matrixWidthInBytes);
            H.Final(hashOutput);
            memcpy(hashOutputBuff + (j - this->lowL) * this->hash2LengthInBytes,
                   hashOutput, this->hash2LengthInBytes);
        }
        *sendBuffSize = (upR - this->lowL) * this->hash2LengthInBytes;
        // *sendBuff = this->hashOutputBuff.data();
        this->lowL += this->bucket2ForComputeH2Output;
        return 0;
    }
}
