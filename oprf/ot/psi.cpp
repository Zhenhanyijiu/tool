#include "psi.h"
#include <assert.h>
#include <cryptoTools/Crypto/RandomOracle.h>
#include <omp.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <cstdint>
#include <stdlib.h>
#include <channel.h>
namespace osuCrypto
{
  //时间统计ms
  long start_time()
  {
    struct timeval start;
    gettimeofday(&start, nullptr);
    return start.tv_sec * 1000000 + start.tv_usec;
  }
  long get_use_time(long start_time)
  {
    struct timeval end;
    gettimeofday(&end, nullptr);
    long usetime = end.tv_sec * 1000000 + end.tv_usec - start_time;
    return usetime / 1000;
  }

  // psiReceiver
  PsiReceiver::PsiReceiver() {}
  PsiReceiver::~PsiReceiver()
  {
    delete this->psiComputePool;
  }
  int PsiReceiver::init(u8_t *commonSeed, u64_t receiverSize, u64_t senderSize,
                        u64_t matrixWidth, u64_t logHeight, int threadNum,
                        u64_t hash2LengthInBytes, u64_t bucket2ForComputeH2Output)
  {
    this->threadNumOmp = threadNum;
    this->matrixWidth = matrixWidth;
    this->matrixWidthInBytes = (matrixWidth + 7) >> 3;
    this->receiverSize = receiverSize;
    this->senderSize = senderSize;
    this->receiverSizeInBytes = (receiverSize + 7) >> 3;
    this->logHeight = logHeight;
    this->height = 1 << this->logHeight;
    this->heightInBytes = (this->height + 7) >> 3; //除以8
    this->bucket1 = 256;
    this->bucket2ForComputeH2Output = bucket2ForComputeH2Output; // default 256
    this->h1LengthInBytes = 32;
    // todo
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
    this->indexId = 0;
    this->psiComputePool = new ThreadPool(this->threadNumOmp);
    this->psiResults.resize(this->senderSize / this->bucket2ForComputeH2Output + 2);
    printf(">>>hash2LengthInBytes:%ld,bucket2ForComputeH2Output:%ld\n",
           hash2LengthInBytes, bucket2ForComputeH2Output);
    return this->iknpOteSender.init(localRng);
  }
  int PsiReceiver::genPK0FromNpot(u8_t *pubParamBuf, const u64_t pubParamBufByteSize,
                                  u8_t **pk0Buf, u64_t *pk0BufSize)
  {
    return this->iknpOteSender.genPK0FromNpot(pubParamBuf, pubParamBufByteSize,
                                              pk0Buf, pk0BufSize);
  }
  //判断接收方接收数据是否结束
  int PsiReceiver::isRecvEnd() { return this->lowL < this->senderSize ? 0 : 1; }

  //******************PsiSender*********************//
  PsiSender::PsiSender() {}
  PsiSender::~PsiSender()
  {
    delete[] this->sendSet;
    this->sendSet = nullptr;
    delete this->commonPrng;
    this->commonPrng = nullptr;
  }
  //初始化
  int PsiSender::init(u8_t *commonSeedIn, u64_t senderSize, u64_t matrixWidth,
                      u64_t logHeight, int threadNum, u64_t hash2LengthInBytes,
                      u64_t bucket2ForComputeH2Output)
  {
    this->threadNumOmp = threadNum;
    this->matrixWidth = matrixWidth;
    this->matrixWidthInBytes = (matrixWidth + 7) >> 3;
    // this->commonSeed = toBlock((u8 *)commonSeed);
    this->logHeight = logHeight;
    this->height = 1 << logHeight;
    this->heightInBytes = (this->height + 7) >> 3; //除以8
    this->senderSize = senderSize;
    this->senderSizeInBytes = (senderSize + 7) >> 3;
    this->bucket1 = 256;
    this->bucket2ForComputeH2Output = bucket2ForComputeH2Output; // default 256
    this->h1LengthInBytes = 32;
    // todo
    this->hash2LengthInBytes = hash2LengthInBytes;

    block commonSeed = toBlock((u8 *)commonSeedIn);
    PRNG localRng(commonSeed);
    //初始化一个向量r，长度为width
    this->choicesWidthInput.resize(matrixWidth);
    this->choicesWidthInput.randomize(localRng);
    this->hashOutputBuff.resize(this->hash2LengthInBytes *
                                this->bucket2ForComputeH2Output);
    this->hashInputs.resize(this->bucket2ForComputeH2Output);
    for (int i = 0; i < this->bucket2ForComputeH2Output; i++)
    {
      this->hashInputs[i].resize(this->matrixWidthInBytes);
    }
    this->transHashInputs.resize(this->matrixWidth);
    for (int i = 0; i < this->matrixWidth; i++)
    {
      this->transHashInputs[i].resize(this->senderSizeInBytes);
      memset(this->transHashInputs[i].data(), 0, this->senderSizeInBytes);
    }
    this->lowL = (u64)0;
    // printf("===>>this->lowL:%ld\n", this->lowL);
    PRNG comPrng(commonSeed);
    this->commonPrng = new PRNG(commonSeed);
    this->sendSet = new block[this->senderSize];
    return this->iknpOteReceiver.init(localRng);
  }
  //生成公共参数
  int PsiSender::genPublicParamFromNpot(u8_t **pubParamBuf, u64_t *pubParamBufByteSize)
  {
    return this->iknpOteReceiver.genPublicParamFromNpot(pubParamBuf, pubParamBufByteSize);
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
    fg = this->iknpOteReceiver.genRecoverMsg(
        this->choicesWidthInput, this->recoverMsgWidthOutput, this->uBuffOutput);
    if (fg)
    {
      return fg;
    }
    *uBuffOutput = (u8 *)this->uBuffOutput.data();
    *uBuffOutputSize = this->uBuffOutput.size() * sizeof(block);
    return 0;
  }
  //判断结束
  int PsiSender::isSendEnd() { return this->lowL < this->senderSize ? 0 : 1; }

//非并行逻辑
#ifdef NOMP
  // common function
  //将所有输入的数据以相同的方式H1做映射，以dataSetOutput返回
  void transformInputByH1(const AES &commonAes, const u64 h1LengthInBytes,
                          const vector<vector<u8>> &dataSetInput, block *dataSetOutput)
  {
    u64 dataSetInputSize = dataSetInput.size();
    block *aesInput = new block[dataSetInputSize];
    block *aesOutput = new block[dataSetInputSize];
    RandomOracle H1(h1LengthInBytes); // 32bytes
    u8 h1Output[h1LengthInBytes];     // 32bytes
    long start0 = start_time();
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
    printf(">>>计算H1内部for循环用时:%ld ms\n", get_use_time(start0));
    commonAes.ecbEncBlocks(aesInput, dataSetInputSize, aesOutput);
    //生成dataSetOutput数据。文章中H1(y)
    for (auto i = 0; i < dataSetInputSize; ++i)
    {
      dataSetOutput[i] ^= aesOutput[i];
    }
    delete[] aesInput;
    aesInput = nullptr;
    delete[] aesOutput;
    aesOutput = nullptr;
  }

  //psireceiver
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
      transLocations[i] = new u8[this->receiverSize * locationInBytes +
                                 sizeof(u32)];
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
    printf("===>>widthBucket1(16/loc):%d,locationInBytes:%d\n", widthBucket1, locationInBytes);
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
                   (u8 *)(randomLocations + (j - low)) + i * locationInBytes, locationInBytes);
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
          auto location = (*(u32 *)(transLocations[i] + j * locationInBytes)) & shift;
          // shift全1
          // location >> 3(除以8)表示matrixDelta[i]的字节位置
          // location & 0b0111,取出低3位；(location & 7) == 0, 1, 2, 3, 4, 5, 6, 7
          matrixDelta[i][location >> 3] &= ~(1 << (location & 7));
        }
      }
      //////////////// Compute matrix A & sent matrix
      ///////////////////////
      // u8 *sentMatrix[w];
      u64 offset1 = wLeft * this->heightInBytes;
      u64 offset2 = 0;
      for (auto i = 0; i < w; ++i)
      {
        PRNG prng(this->encMsgOutput[i + wLeft][0]);
        prng.get(matrixA[i], this->heightInBytes);
        prng.SetSeed(this->encMsgOutput[i + wLeft][1]);
        // prng.get(sentMatrix[i], this->heightInBytes);
        prng.get(this->sendMatrixADBuff.data() + offset1 + offset2,
                 this->heightInBytes);
        for (auto j = 0; j < this->heightInBytes;
             ++j)
        {
          // sentMatrix[i][j] ^= matrixA[i][j] ^ matrixDelta[i][j];
          (this->sendMatrixADBuff.data() + offset1 + offset2)[j] ^=
              matrixA[i][j] ^ matrixDelta[i][j];
        }
        //发送sM^A^D
        //发送数据U^A^D给对方，
        // ch.asyncSend(sentMatrix[i], heightInBytes);
        //偏移计算
        offset2 += this->heightInBytes;
      }
      ///////////////// Compute hash inputs (transposed)/////////////////////
      for (auto i = 0; i < w; ++i)
      {
        for (auto j = 0; j < receiverSize; ++j)
        {
          auto location =
              (*(u32 *)(transLocations[i] + j * locationInBytes)) &
              shift;
          this->transHashInputs[i + wLeft][j >> 3] |=
              (u8)((bool)(matrixA[i][location >> 3] & (1 << (location &
                                                             7))))
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
      matrixA[i] = nullptr;
      delete[] matrixDelta[i];
      matrixDelta[i] = nullptr;
      delete[] transLocations[i];
      transLocations[i] = nullptr;
    }
    delete[] recvSet;
    recvSet = nullptr;
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
      auto up = low + this->bucket2ForComputeH2Output < this->receiverSize
                    ? low + this->bucket2ForComputeH2Output
                    : this->receiverSize;
      for (auto j = low; j < up; ++j)
      {
        memset(hashInputs[j - low], 0, this->matrixWidthInBytes);
      }
      for (auto i = 0; i < this->matrixWidth; ++i)
      {
        for (auto j = low; j < up; ++j)
        {
          hashInputs[j - low][i >> 3] |=
              (u8)((bool)(this->transHashInputs[i][j >> 3] & (1 << (j & 7)))) << (i & 7);
        }
      }
      for (auto j = low; j < up; ++j)
      {
        H.Reset();
        H.Update(hashInputs[j - low], this->matrixWidthInBytes);
        H.Final(hashOutput);
        //生成一个map并保存
        // this->allHashes[*(u64 *)hashOutput].push_back(std::make_pair(*(block *)hashOutput, j));
        this->allHashes[*(u64 *)hashOutput].emplace_back(std::make_pair(*(block *)hashOutput, j));
      }
    }
    for (auto i = 0; i < this->bucket2ForComputeH2Output; ++i)
    {
      delete[] hashInputs[i];
    }
    return 0;
  }
  //PsiReceiver,接收对方发来的hash输出
  int PsiReceiver::recvFromSenderAndComputePSIOnce(const u8_t *recvBuff,
                                                   const u64_t recvBufSize,
                                                   vector<u32_t> *psiMsgIndex)
  {
    auto up = this->lowL + this->bucket2ForComputeH2Output < this->senderSize
                  ? this->lowL + this->bucket2ForComputeH2Output
                  : this->senderSize;
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
          // psiMsgIndex->push_back(found->second[i].second);
          psiMsgIndex->emplace_back(found->second[i].second);
          break;
        }
      }
    }
    this->lowL += this->bucket2ForComputeH2Output;
    return 0;
  }
  //psisender
  //计算所有id的hash1值，备用
  int PsiSender::computeAllHashOutputByH1(const vector<vector<u8_t>> senderSet)
  {
    if (this->senderSize != senderSet.size())
    {
      return -111;
    }
    /////////// Transform input /////////////////////
    block commonKey;
    AES commonAesHash1, commonAesFkey;
    this->commonPrng->get((u8 *)&commonKey, sizeof(block));
    commonAesHash1.setKey(commonKey);
    // block *sendSet = new block[this->senderSize];
    transformInputByH1(commonAesHash1, this->h1LengthInBytes, senderSet, this->sendSet);
    /////////// Transform input end /////////////////
    return 0;
  }
  int PsiSender::recoverMatrixC(const u8_t *recvMatrixADBuff, const u64_t recvMatixADBuffSize)
  {
    auto locationInBytes = (this->logHeight + 7) / 8;    // logHeight==1
    auto widthBucket1 = sizeof(block) / locationInBytes; // 16/1
    u64 shift = (1 << this->logHeight) - 1;              //全1
    printf("===>>widthBucket1:%ld(16/loc),locationInBytes:%ld\n",
           widthBucket1, locationInBytes);
    ////////////// Initialization //////////////////////
    // PRNG commonPrng(this->commonSeed);
    u8 *transLocations[widthBucket1]; // 16个u8*

    if (recvMatixADBuffSize != this->matrixWidth * this->heightInBytes)
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

    cout << "***********************before cycle" << endl;
    block commonKey;
    // AES commonAesFkey;
    //cycle start
    long start1 = start_time();
    long start22;
    int cycNum = 0; //128/5=25...3
    for (auto wLeft = 0; wLeft < this->matrixWidth; wLeft += widthBucket1)
    {
      cycNum++;
      auto wRight = wLeft + widthBucket1 < this->matrixWidth ? wLeft + widthBucket1
                                                             : this->matrixWidth;
      auto w = wRight - wLeft;
      //////////// Compute random locations (transposed) ////////////////
      this->commonPrng->get((u8 *)&commonKey, sizeof(block));
      AES commonAesFkey;
      commonAesFkey.setKey(commonKey);
      start22 = start_time();
      for (auto low = 0; low < this->senderSize; low += bucket1)
      {
        auto up = low + bucket1 < this->senderSize ? low + bucket1 : this->senderSize;
        commonAesFkey.ecbEncBlocks(this->sendSet + low, up - low, randomLocations);
        for (auto i = 0; i < w; ++i)
        {
          for (auto j = low; j < up; ++j)
          {
            memcpy(transLocations[i] + j * locationInBytes,
                   (u8 *)(randomLocations + (j - low)) + i * locationInBytes, locationInBytes);
          }
        }
      }
      // printf("===>>update aes一次transLocations用时:%ldms\n",get_use_time(start22));
      //////////////// Extend OTs and compute matrix C ///////////////////
      // u8 *recvMatrix;
      u64 offset1 = wLeft * this->heightInBytes;
      u64 offset2 = 0;
      long startOts = start_time();
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
      // printf("===>>>计算compute matrix C用时：%ldms\n", get_use_time(startOts));
      ///////////////// Compute hash inputs (transposed)/////////////////////
      long startInput = start_time();
      for (auto i = 0; i < w; ++i)
      {
        for (auto j = 0; j < senderSize; ++j)
        {
          auto location =
              (*(u32 *)(transLocations[i] + j * locationInBytes)) & shift;
          this->transHashInputs[i + wLeft][j >> 3] |=
              (u8)((bool)(matrixC[i][location >> 3] & (1 << (location & 7)))) << (j & 7);
        }
      }
      // printf("===>>>计算一次hashInput用时：%ldms\n", get_use_time(startInput));
    }
    //cycle end
    //释放内存
    cout << "***********************before cycle end" << endl;
    printf("===>>cycle 用时：%ldms\n", get_use_time(start1));
    printf("===>>cycNum:%d\n", cycNum);
    long start33 = start_time();
    for (auto i = 0; i < widthBucket1; ++i)
    {
      delete[] transLocations[i];
      delete[] matrixC[i];
    }
    printf("===>>delete 释放内存用时:%ldms\n", get_use_time(start33));
    printf(">>>>>>>>>> sendSet,end\n");
    return 0;
  }
  //计算本方的hash输出并发送给对方
  int PsiSender::computeHashOutputToReceiverOnce(u8_t **sendBuff, u64_t *sendBuffSize)
  {
    auto upR = this->lowL + this->bucket2ForComputeH2Output < this->senderSize
                   ? this->lowL + this->bucket2ForComputeH2Output
                   : this->senderSize;
    // H2
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
            (u8)((bool)(this->transHashInputs[i][j >> 3] & (1 << (j & 7)))) << (i & 7);
      }
    }
    u64 offset = 0;
    for (auto j = this->lowL; j < upR; ++j, offset += this->hash2LengthInBytes)
    {
      H.Reset();
      H.Update(this->hashInputs[j - this->lowL].data(), this->matrixWidthInBytes);
      H.Final(hashOutput);
      memcpy(this->hashOutputBuff.data() + offset, hashOutput, this->hash2LengthInBytes);
    }
    *sendBuffSize = (upR - this->lowL) * this->hash2LengthInBytes;
    *sendBuff = this->hashOutputBuff.data();
    this->lowL += this->bucket2ForComputeH2Output;
    return 0;
  }
#endif
//并行逻辑
#if (defined OMP_ONLY) || (defined OMP_POOL)
  //多线程处理，计算Hash1
  typedef struct HashOneInfo
  {
    int threadId;
    int h1LengthInBytes;
    u64 processNum;
    block *aesInputStart;
    block *dataSetOutputStart;
    vector<u8> *dataSetInputStart;
  } HashOneInfo;
  //自己创建线程，未使用这种方式
  void *process_hash1_thread(void *arg)
  {
    HashOneInfo *info = (HashOneInfo *)arg;
    RandomOracle H1(info->h1LengthInBytes); // 32bytes
    u8 h1Output[info->h1LengthInBytes];     // 32bytes
    for (auto i = 0; i < info->processNum; ++i)
    {
      // 256个元素
      H1.Reset();
      //对每一个y属于dataSetInput，映射成一个hash值，32字节（H1,H2）
      H1.Update(info->dataSetInputStart[i].data(), info->dataSetInputStart[i].size());
      H1.Final(h1Output);
      // H1
      info->aesInputStart[i] = *(block *)h1Output; //前16字节，后16字节
      // H2
      info->dataSetOutputStart[i] = *(block *)(h1Output + sizeof(block));
    }
    pthread_exit(NULL);
  }
  //自己创建线程，未使用这种方式
  void transformInputByH1_thread(const AES &commonAes, const u64 h1LengthInBytes,
                                 const vector<vector<u8>> &dataSetInput,
                                 block *dataSetOutput)
  {
    u64 dataSetInputSize = dataSetInput.size();
    block *aesInput = new block[dataSetInputSize];
    block *aesOutput = new block[dataSetInputSize];
    // RandomOracle H1(h1LengthInBytes); // 32bytes
    // u8 h1Output[h1LengthInBytes];     // 32bytes
    long start0 = start_time();
    int numTh = 2;
    pthread_t threads[numTh];
    pthread_attr_t attr;
    void *status;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    HashOneInfo infoArgs[numTh];
    memset(infoArgs, 0, numTh * sizeof(HashOneInfo));
    u64 stepLength = dataSetInputSize / numTh;
    u64 remain = dataSetInputSize % numTh;
    printf(">>>多线程stepLength:%ld\n", stepLength);
    for (int i = 0; i < numTh; i++)
    {
      if (i == numTh - 1)
      {
        infoArgs[i].processNum = stepLength + remain;
      }
      else
      {
        infoArgs[i].processNum = stepLength;
      }
      infoArgs[i].threadId = i;
      infoArgs[i].h1LengthInBytes = 32;
      infoArgs[i].aesInputStart = aesInput + i * stepLength;
      infoArgs[i].dataSetOutputStart = dataSetOutput + i * stepLength;
      infoArgs[i].dataSetInputStart = (vector<u8> *)(dataSetInput.data()) + i * stepLength;
      int fg = pthread_create(&threads[i], nullptr, process_hash1_thread, (void *)(infoArgs + i));
      if (fg)
      {
        printf(">>>create thread error\n");
        exit(-1);
      }
      printf(">>>i in for cycle is:%d\n", i);
    }
    // 删除属性，并等待其他线程
    pthread_attr_destroy(&attr);
    for (int i = 0; i < numTh; i++)
    {
      int fg = pthread_join(threads[i], &status);
      if (fg)
      {
        printf(">>>pthread_join thread error\n");
        exit(-1);
      }
    }
    // pthread_exit(NULL);
    printf(">>>计算H1内部for循环用时:%ld ms\n", get_use_time(start0));
    commonAes.ecbEncBlocks(aesInput, dataSetInputSize, aesOutput);
    //生成dataSetOutput数据。文章中H1(y)
    for (auto i = 0; i < dataSetInputSize; ++i)
    {
      dataSetOutput[i] ^= aesOutput[i];
    }
    delete[] aesInput;
    aesInput = nullptr;
    delete[] aesOutput;
    aesOutput = nullptr;
  }

  //用omp并行指令，加速Hash1的计算，在使用
  void process_hash1_omp(HashOneInfo *info)
  {
    printf(">>>(process_hash1_omp)omp thread id:%d\n", info->threadId);
    RandomOracle H1(info->h1LengthInBytes); // 32bytes
    u8 h1Output[info->h1LengthInBytes];     // 32bytes
    for (auto i = 0; i < info->processNum; ++i)
    {
      // 256个元素
      H1.Reset();
      //对每一个y属于dataSetInput，映射成一个hash值，32字节（H1,H2）
      H1.Update(info->dataSetInputStart[i].data(), info->dataSetInputStart[i].size());
      H1.Final(h1Output);
      // H1
      info->aesInputStart[i] = *(block *)h1Output; //前16字节，后16字节
      // H2
      info->dataSetOutputStart[i] = *(block *)(h1Output + sizeof(block));
    }
  }
  //用omp并行指令，加速Hash1的计算
  void transformInputByH1(const AES &commonAes, const u64 h1LengthInBytes,
                          const vector<vector<u8>> &dataSetInput,
                          block *dataSetOutput, int threadNum)
  {
    u64 dataSetInputSize = dataSetInput.size();
    block *aesInput = new block[dataSetInputSize];
    block *aesOutput = new block[dataSetInputSize];
    long start0 = start_time();
    int numTh = threadNum;
    HashOneInfo infoArgs[numTh];
    memset(infoArgs, 0, sizeof(HashOneInfo) * numTh);
    u64 stepLength = dataSetInputSize / numTh;
    u64 remain = dataSetInputSize % numTh;
    printf(">>>omp 并行计算Hash1,stepLength:%ld,threadNum(%d)\n", stepLength, threadNum);
    for (int i = 0; i < numTh; i++)
    {
      infoArgs[i].threadId = i;
      if (i == numTh - 1)
      {
        infoArgs[i].processNum = stepLength + remain;
      }
      else
      {
        infoArgs[i].processNum = stepLength;
      }
      infoArgs[i].threadId = i;
      infoArgs[i].h1LengthInBytes = 32;
      infoArgs[i].aesInputStart = aesInput + i * stepLength;
      infoArgs[i].dataSetOutputStart = dataSetOutput + i * stepLength;
      infoArgs[i].dataSetInputStart = (vector<u8> *)(dataSetInput.data()) + i * stepLength;
    }
#pragma omp parallel for num_threads(numTh)
    for (int i = 0; i < numTh; i++)
    {
      process_hash1_omp(infoArgs + i);
    }
    // printf(">>>omp 并行计算Hash1 用时:%ld ms\n", get_use_time(start0));
    commonAes.ecbEncBlocks(aesInput, dataSetInputSize, aesOutput);
    //生成dataSetOutput数据。文章中H1(y)
    for (auto i = 0; i < dataSetInputSize; ++i)
    {
      dataSetOutput[i] ^= aesOutput[i];
    }
    delete[] aesInput;
    aesInput = nullptr;
    delete[] aesOutput;
    aesOutput = nullptr;
  }
  //并行计算 MatrixAxorD,参数结构体
  typedef struct ComputeMatrixAxorDInfo
  {
    int threadId;
    // 1
    u64 shift;
    // 2
    u64 wLeftBegin;
    // 3
    u64 processNum;
    // 4
    u64 aesKeyNum;
    // 5
    u64 aesComkeysBegin;
    // 6
    block *aesComKeys;
    // 7
    u64 receiverSize;
    // 8
    u64 heightInBytes;
    // 9
    u64 locationInBytes;
    // 10
    u64 bucket1;
    // 11
    u64 widthBucket1;
    // 12
    block *recvSet;
    // 13
    array<block, 2> *encMsgOutputPtr;
    // 15
    const u8 *recvMatrixADBuffBegin;
    // 16
    vector<u8> *transHashInputsPtr;
    // 17,存储发送给对方的数据
    u8 *sendMatrixADBuffPtr;
  } ComputeMatrixAxorDInfo;
  //并行计算矩阵AxorD，处理函数
  void process_compute_Matrix_AxorD(ComputeMatrixAxorDInfo *infoArg)
  {
    printf(">>>(process_compute_Matrix_AxorD)omp thread id:%d\n", infoArg->threadId);
    int cycTimes = 0;
    u64 bucket1 = infoArg->bucket1;
    u64 widthBucket1 = infoArg->widthBucket1;
    u64 heightInBytes = infoArg->heightInBytes;
    u64 locationInBytes = infoArg->locationInBytes;
    u64 receiverSize = infoArg->receiverSize;
    block randomLocations[bucket1]; // 256block
    u8 *transLocations[widthBucket1];
    for (auto i = 0; i < widthBucket1; ++i)
    {
      transLocations[i] = new u8[receiverSize * locationInBytes + sizeof(u32)];
    }
    u8 *matrixA[widthBucket1];
    u8 *matrixDelta[widthBucket1];
    for (auto i = 0; i < widthBucket1; ++i)
    {
      matrixA[i] = new u8[heightInBytes]; //要释放
      matrixDelta[i] = new u8[heightInBytes];
    }
    for (auto wLeft = 0; wLeft < infoArg->processNum;
         wLeft += widthBucket1, cycTimes++)
    {
      auto wRight = wLeft + widthBucket1 < infoArg->processNum ? wLeft + widthBucket1
                                                               : infoArg->processNum;
      auto w = wRight - wLeft;
      //////////// Compute random locations (transposed) ////////////////
      AES commonAesFkey;
      commonAesFkey.setKey(infoArg->aesComKeys[cycTimes]);
      for (auto low = 0; low < receiverSize; low += bucket1)
      {
        auto up = low + bucket1 < receiverSize ? low + bucket1 : receiverSize;
        //每256个输入处理一次，randomLocations==256blocks,Fk函数，Fk(H1(y))
        commonAesFkey.ecbEncBlocks(infoArg->recvSet + low, up - low, randomLocations);
        //如果w比较宽，这里的计算会增加
        for (auto i = 0; i < w; ++i)
        {
          for (auto j = low; j < up; ++j)
          {
            // randomLocations,256个block
            memcpy(transLocations[i] + j * locationInBytes,
                   (u8 *)(randomLocations + (j - low)) + i * locationInBytes, locationInBytes);
          }
        }
      }
      //////////// Compute matrix Delta /////////////////////////////////
      //对应论文中的P2(接收者)
      for (auto i = 0; i < widthBucket1; ++i)
      {
        memset(matrixDelta[i], 255, heightInBytes);
        // heightInBytes，置为全1矩阵
      }
      //本方拥有的数据y的个数
      for (auto i = 0; i < w; ++i)
      {
        for (auto j = 0; j < receiverSize; ++j)
        {
          auto location = (*(u32 *)(transLocations[i] + j * locationInBytes)) & (infoArg->shift);
          // shift全1
          // location >> 3(除以8)表示matrixDelta[i]的字节位置
          // location & 0b0111,取出低3位；(location & 7)==0,1,2,3,4,5,6,7
          matrixDelta[i][location >> 3] &= ~(1 << (location & 7));
        }
      }
      //////////////// Compute matrix A & sent matrix ///////////////////////
      u64 offset1 = wLeft * heightInBytes;
      u64 offset2 = 0;
      for (auto i = 0; i < w; ++i)
      {
        PRNG prng(infoArg->encMsgOutputPtr[i + wLeft][0]);
        prng.get(matrixA[i], heightInBytes);
        prng.SetSeed(infoArg->encMsgOutputPtr[i + wLeft][1]);
        // prng.get(sentMatrix[i], this->heightInBytes);
        prng.get(infoArg->sendMatrixADBuffPtr + offset1 + offset2, heightInBytes);
        for (auto j = 0; j < heightInBytes; ++j)
        {
          // sentMatrix[i][j] ^= matrixA[i][j] ^ matrixDelta[i][j];
          (infoArg->sendMatrixADBuffPtr + offset1 + offset2)[j] ^= matrixA[i][j] ^ matrixDelta[i][j];
        }
        //发送sM^A^D
        //发送数据U^A^D给对方，
        // ch.asyncSend(sentMatrix[i], heightInBytes);
        //偏移计算
        offset2 += heightInBytes;
      }
      ///////////////// Compute hash inputs (transposed) /////////////////////
      for (auto i = 0; i < w; ++i)
      {
        for (auto j = 0; j < receiverSize; ++j)
        {
          auto location = (*(u32 *)(transLocations[i] + j * locationInBytes)) & (infoArg->shift);
          infoArg->transHashInputsPtr[i + wLeft][j >> 3] |=
              (u8)((bool)(matrixA[i][location >> 3] & (1 << (location & 7)))) << (j & 7);
        }
      }
    }
    // 最后释放空间
    for (auto i = 0; i < widthBucket1; ++i)
    {
      delete[] matrixA[i];
      matrixA[i] = nullptr;
      delete[] matrixDelta[i];
      matrixDelta[i] = nullptr;
      delete[] transLocations[i];
      transLocations[i] = nullptr;
    }
  }
  int PsiReceiver::getSendMatrixADBuff(const u8_t *uBuffInput, const u64_t uBuffInputSize,
                                       const vector<vector<u8_t>> receiverSet,
                                       u8_t **sendMatrixADBuff, u64_t *sendMatixADBuffSize)
  {
    int ublocksize = uBuffInputSize / sizeof(block);
    printf(">>>收到的TxorR矩阵的字节大小:%ld,转化成block大小为:%ld\n", uBuffInputSize, ublocksize);
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
    // u32 locationInBytes = (this->logHeight + 7) / 8;
    u64 locationInBytes = (this->logHeight + 7) >> 3;
    u64 widthBucket1 = sizeof(block) / locationInBytes;
    u64 shift = (1 << this->logHeight) - 1; //全1
    //////////// Initialization ///////////////////
    PRNG commonPrng(this->commonSeed);
    block commonKey;
    AES commonAes;
    if (receiverSet.size() != this->receiverSize)
    {
      return -111;
    }
    /////////// Transform input /////////////////////
    commonPrng.get((u8 *)&commonKey, sizeof(block));
    commonAes.setKey(commonKey);
    block *recvSet = new block[this->receiverSize];
    long start0 = start_time();
    transformInputByH1(commonAes, this->h1LengthInBytes, receiverSet, recvSet,
                       this->threadNumOmp);
    printf(">>>omp 并行计算Hash1用时:%ld ms\n", get_use_time(start0));
    ////////// Transform input end //////////////////
    /*********for cycle start*********/
    printf(">>>(AD)widthBucket1(16/loc):%d,locationInBytes:%d\n", widthBucket1, locationInBytes);
    printf("\n========并行计算AxorD参数准备开始======\n");
    //并行计算矩阵AxorD
    int threadNum = this->threadNumOmp;
    int sumCount = 0;
    int isExit = 0;
    ComputeMatrixAxorDInfo infoArgs[threadNum];
    //将全部内存初始为0
    memset((char *)infoArgs, 0, sizeof(ComputeMatrixAxorDInfo) * threadNum);
    //为每一个线程生成参数数据,主要是处理的矩阵的列数
    for (;;)
    {
      for (int k = 0; k < threadNum; k++)
      {
        int wRight = sumCount + widthBucket1;
        if (wRight <= this->matrixWidth)
        {
          // 3
          infoArgs[k].processNum += widthBucket1;
          // 4
          infoArgs[k].aesKeyNum++;
        }
        else
        {
          isExit = 1;
          break;
        }
        sumCount += widthBucket1;
      }
      if (isExit)
      {
        break;
      }
    }
    printf(">>>debug sumCount:=%d\n", sumCount);
    //不要漏掉余数
    if ((this->matrixWidth - sumCount) != 0)
    {
      infoArgs[threadNum - 1].processNum += this->matrixWidth - sumCount;
      infoArgs[threadNum - 1].aesKeyNum += 1;
    }
    for (int i = 0; i < threadNum; i++)
    {
      printf(">>>procesNum[%2d]:=%2d,", i, infoArgs[i].processNum);
    }
    cout << endl;
    for (int i = 0; i < threadNum; i++)
    {
      printf(">>>aesKeyNum[%2d]:=%2d,", i, infoArgs[i].aesKeyNum);
    }
    //分配处理的矩阵的列数结束
    //最重要的一步，生成Fk函数的keys
    vector<block> commonKeys;
    for (auto wLeft = 0; wLeft < this->matrixWidth; wLeft += widthBucket1)
    {
      block comKey;
      commonPrng.get((u8 *)&comKey, sizeof(block));
      // commonKeys.push_back(comKey);
      commonKeys.emplace_back(comKey);
    }
    printf("\n>>>commonKeys size:%ld\n", commonKeys.size());
    //给参数赋值,重要
    for (int k = 0; k < threadNum; k++)
    {
      infoArgs[k].threadId = k;
      // 1
      infoArgs[k].shift = shift;
      if (k == 0)
      {
        // 2
        infoArgs[k].wLeftBegin = 0;
        // 5
        infoArgs[k].aesComkeysBegin = 0;
      }
      else
      {
        infoArgs[k].wLeftBegin = infoArgs[k - 1].wLeftBegin + infoArgs[k - 1].processNum;
        infoArgs[k].aesComkeysBegin = infoArgs[k - 1].aesComkeysBegin + infoArgs[k - 1].aesKeyNum;
      }
      // 7
      infoArgs[k].receiverSize = this->receiverSize;
      // 8
      infoArgs[k].heightInBytes = this->heightInBytes;
      // 9
      infoArgs[k].locationInBytes = locationInBytes;
      // 10
      infoArgs[k].bucket1 = this->bucket1;
      // 11
      infoArgs[k].widthBucket1 = widthBucket1;
      // 6
      infoArgs[k].aesComKeys = (block *)(commonKeys.data()) + infoArgs[k].aesComkeysBegin;
      // 12
      infoArgs[k].recvSet = recvSet;
      // 13
      infoArgs[k].encMsgOutputPtr =
          (array<block, 2> *)(this->encMsgOutput.data()) + infoArgs[k].wLeftBegin;
      // 16
      infoArgs[k].transHashInputsPtr =
          (vector<u8> *)(this->transHashInputs.data()) + infoArgs[k].wLeftBegin;
      // 17
      infoArgs[k].sendMatrixADBuffPtr = (u8 *)(this->sendMatrixADBuff.data()) +
                                        (infoArgs[k].wLeftBegin) * (this->heightInBytes);
    }
    for (int i = 0; i < threadNum; i++)
    {
      printf("aesBegin[%2d]:%2ld,", i, infoArgs[i].aesComkeysBegin);
    }
    printf("\n");
    for (int j = 0; j < threadNum; j++)
    {
      printf("wlfBegin[%2ld]:%2ld,", j, infoArgs[j].wLeftBegin);
    }
    printf("\n========并行计算AxorD参数准备结束======\n");
    printf("========开始并行计算矩阵AxorD,threadNum(%d)========\n", threadNum);
    // omp process
    start0 = start_time();
#pragma omp parallel for num_threads(threadNum)
    for (int i = 0; i < threadNum; i++)
    {
      process_compute_Matrix_AxorD(infoArgs + i);
    }
    printf("========并行计算矩阵AxorD结束,threadNum(%d)========\n", threadNum);
    printf(">>>计算Hash1之后,并行生成矩阵A,D用时:%ldms\n", get_use_time(start0));
    /*********for cycle end*********/
    //将uBuff输出并发送给对方
    *sendMatrixADBuff = this->sendMatrixADBuff.data();
    *sendMatixADBuffSize = this->heightInBytes * this->matrixWidth;
    /****************************/
    // 最后释放空间
    delete[] recvSet;
    recvSet = nullptr;
    return 0;
  }
  //并行处理，并行生成hashMap,参数结构体类型
  typedef struct HashMapParallelInfo
  {
    int threadId;
    u64 startIndex;
    u64 processNum;
    u64 receiverSize;
    u64 matrixWidth;
    u64 matrixWidthInBytes;
    u64 hash2LengthInBytes;
    u64 bucket2ForComputeH2Output;
    vector<u8> *transHashInputsPtr;
    unordered_map<u64, std::vector<std::pair<block, u32_t>>> *hashMap;
  } HashMapParallelInfo;
  //将sendMatrixADBuff发送给对方之后，接下来生成AllHashMap

  //并行生成hashmap 处理函数
  void process_for_hash_map(HashMapParallelInfo *infoArg)
  {
    printf(">>>(process_for_hash_map)omp thread id:%d\n", infoArg->threadId);
    RandomOracle H(infoArg->hash2LengthInBytes);
    u8 hashOutput[sizeof(block)];
    u8 *hashInputs[infoArg->bucket2ForComputeH2Output];
    for (auto i = 0; i < infoArg->bucket2ForComputeH2Output; ++i)
    {
      hashInputs[i] = new u8[infoArg->matrixWidthInBytes];
    }
    //接收集合中的每个元素
    u64 rightIndex = infoArg->startIndex + infoArg->processNum;
    for (auto low = infoArg->startIndex; low < rightIndex; low += infoArg->bucket2ForComputeH2Output)
    {
      auto up = low + infoArg->bucket2ForComputeH2Output < rightIndex
                    ? low + infoArg->bucket2ForComputeH2Output
                    : rightIndex;
      for (auto j = low; j < up; ++j)
      {
        memset(hashInputs[j - low], 0, infoArg->matrixWidthInBytes);
      }
      for (auto i = 0; i < infoArg->matrixWidth; ++i)
      {
        for (auto j = low; j < up; ++j)
        {
          hashInputs[j - low][i >> 3] |=
              (u8)((bool)(infoArg->transHashInputsPtr[i][j >> 3] & (1 << (j & 7)))) << (i & 7);
        }
      }
      for (auto j = low; j < up; ++j)
      {
        H.Reset();
        H.Update(hashInputs[j - low], infoArg->matrixWidthInBytes);
        H.Final(hashOutput);
        //生成一个map并保存
        // infoArg->hashMap[0][*(u64 *)hashOutput].push_back(
        //     std::make_pair(*(block *)hashOutput, j));
        infoArg->hashMap[0][*(u64 *)hashOutput].emplace_back(std::make_pair(*(block *)hashOutput, j));
      }
    }
    for (auto i = 0; i < infoArg->bucket2ForComputeH2Output; ++i)
    {
      delete[] hashInputs[i];
      hashInputs[i] = nullptr;
    }
  }
  int PsiReceiver::genenateAllHashesMap()
  {
    /////////////////// Compute hash outputs ///////////////////////////
    // H2
    RandomOracle H(this->hash2LengthInBytes);
    u8 hashOutput[sizeof(block)];
    u8 *hashInputs[this->bucket2ForComputeH2Output];
    int threadNum = this->threadNumOmp;
    this->HashMapVector.resize(threadNum);
    u64 processLen = this->receiverSize / threadNum;
    u64 remain = this->receiverSize % threadNum;
    HashMapParallelInfo infoArgs[threadNum];
    //内存初始化为0
    memset(infoArgs, 0, threadNum * sizeof(HashMapParallelInfo));
    //给每个线程的参数赋值
    for (int i = 0; i < threadNum; i++)
    {
      infoArgs[i].threadId = i;
      infoArgs[i].startIndex = i * processLen;
      if (i == threadNum - 1)
      {
        infoArgs[i].processNum = processLen + remain;
      }
      else
      {
        infoArgs[i].processNum = processLen;
      }
      infoArgs[i].receiverSize = this->receiverSize;
      infoArgs[i].matrixWidth = this->matrixWidth;
      infoArgs[i].matrixWidthInBytes = this->matrixWidthInBytes;
      infoArgs[i].hash2LengthInBytes = this->hash2LengthInBytes;
      infoArgs[i].bucket2ForComputeH2Output = this->bucket2ForComputeH2Output;
      infoArgs[i].transHashInputsPtr = (vector<u8> *)(this->transHashInputs.data());
      infoArgs[i].hashMap =
          (unordered_map<u64, std::vector<std::pair<block, u32_t>>> *)(this->HashMapVector.data() + i);
    }
    //给每个线程的参数赋值结束
    printf(">>>并行计算hashmap 开始，threadNum(%d)\n", threadNum);
#pragma omp parallel for num_threads(threadNum)
    for (int i = 0; i < threadNum; i++)
    {
      process_for_hash_map(infoArgs + i);
    }
    return 0;
  }
  /********************
  //并行处理 匹配逻辑,采用在外部进行omp指令，性能比在外部还要慢5倍以上
  typedef struct ComputePsiInfo
  {
    //1
    int threadId;
    //2
    const u8_t *recvBuffStart;
    //3
    u64 processNum;
    //4
    u64 hash2LengthInBytes;
    //5
    u64 hashMapSize;
    //6
    unordered_map<u64, std::vector<std::pair<block, u32_t>>> *hashMapPtr;
    //7
    vector<u32_t> *psiIndexVecTempPtr;
  } ComputePsiInfo;
  void process_compute_psi_omp(ComputePsiInfo *infoArg)
  {
    // printf(">>>(process_compute_psi_omp)omp thread id:%d\n", infoArg->threadId);
    for (auto idx = 0; idx < infoArg->processNum; ++idx)
    {
      u64 mapIdx = *(u64 *)(infoArg->recvBuffStart + idx * infoArg->hash2LengthInBytes);
      for (int i = 0; i < infoArg->hashMapSize; i++)
      {
        auto found = infoArg->hashMapPtr[i].find(mapIdx);
        if (found == infoArg->hashMapPtr[i].end())
          continue;
        //可能找到好几个
        for (auto i = 0; i < found->second.size(); ++i)
        {
          if (memcmp(&(found->second[i].first),
                     infoArg->recvBuffStart + idx * infoArg->hash2LengthInBytes,
                     infoArg->hash2LengthInBytes) == 0)
          {
            // (infoArg->psiIndexVecTempPtr)[0].push_back(found->second[i].second);
            (infoArg->psiIndexVecTempPtr)[0].emplace_back(found->second[i].second);
            break;
          }
        }
      }
    }
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
    u64 threadNum = this->threadNumOmp;
    vector<vector<u32_t>> psiIndexVecTemp(threadNum);
    ComputePsiInfo infoArgs[threadNum];
    memset((char *)infoArgs, 0, sizeof(ComputePsiInfo) * threadNum);
    u64 processNumAll = up - this->lowL;
    u64 processNum = up / threadNum;
    //取余
    // u64 remain = up & (threadNumOmp - 1);
    u64 remain = up % threadNum;
    for (int i = 0; i < threadNum; i++)
    {
      //1
      infoArgs[i].threadId = i;
      //3
      if (i < threadNum - 1)
      {
        infoArgs[i].processNum = processNum;
      }
      else
      {
        infoArgs[i].processNum = processNum + remain;
      }
      //2
      infoArgs[i].recvBuffStart = recvBuff + processNum * i;
      //4
      infoArgs[i].hash2LengthInBytes = this->hash2LengthInBytes;
      //5
      infoArgs[i].hashMapSize = threadNum;
      //6
      infoArgs[i].hashMapPtr = (unordered_map<u64, std::vector<std::pair<block, u32_t>>> *)(this->HashMapVector.data());
      //7
      infoArgs[i].psiIndexVecTempPtr = (vector<u32_t> *)(psiIndexVecTemp.data()) + i;
    }

#pragma omp parallel for num_threads(threadNum)
    for (int i = 0; i < threadNum; i++)
    {
      process_compute_psi_omp(infoArgs + i);
    }
    this->lowL += this->bucket2ForComputeH2Output;
    u64_t sum_len = 0;
    for (int i = 0; i < threadNum; i++)
    {
      // psiMsgIndex->insert(psiMsgIndex->end(), psiIndexVecTemp[i].begin(), psiIndexVecTemp[i].end());
      sum_len += psiIndexVecTemp[i].size();
    }
    psiMsgIndex->resize(sum_len);
    u64_t offset = 0;
    for (int i = 0; i < threadNum; i++)
    {
      memcpy(psiMsgIndex->data() + offset, psiIndexVecTemp[i].data(),
             sizeof(u32_t) * psiIndexVecTemp[i].size());
      offset += psiIndexVecTemp[i].size();
    }
    return 0;
  }

*******************/

  //*******************psi-sender*****************//
  //计算所有id的hash1值，备用
  int PsiSender::computeAllHashOutputByH1(const vector<vector<u8_t>> senderSet)
  {
    if (this->senderSize != senderSet.size())
    {
      return -111;
    }
    /////////// Transform input /////////////////////
    block commonKey;
    AES commonAesHash1, commonAesFkey;
    this->commonPrng->get((u8 *)&commonKey, sizeof(block));
    commonAesHash1.setKey(commonKey);
    // block *sendSet = new block[this->senderSize];
    transformInputByH1(commonAesHash1, this->h1LengthInBytes, senderSet,
                       this->sendSet, this->threadNumOmp);
    /////////// Transform input end /////////////////
    return 0;
  }
  //并行恢复 recoverMatrixC，参数结构体类型
  typedef struct RecoverMatrixCInfo
  {
    int threadId;
    // 1
    u64 shift;
    // 2
    u64 wLeftBegin;
    // 3
    u64 processNum;
    // 4
    u64 aesKeyNum;
    // 5
    u64 aesComkeysBegin;
    // 6
    block *aesComKeys;
    // AES *currThreadAesFkey;
    // 7
    u64 senderSize;
    // 8
    u64 heightInBytes;
    // 9
    u64 locationInBytes;
    // 10
    u64 bucket1;
    // 11
    u64 widthBucket1;
    // 12
    block *sendset;
    // 13
    block *recoverMsgWidthOutputPtr;
    // 14
    int *choicesWidthInputPtr;
    // 15
    const u8 *recvMatrixADBuffBegin;
    // 16
    vector<u8> *transHashInputsPtr;
  } RecoverMatrixCInfo;

  void process_recover_matrix_C(RecoverMatrixCInfo *infoArg)
  {
    printf(">>>(process_recover_matrix_C)omp thread id:%d\n", infoArg->threadId);
    int cycTimes = 0;
    u64 widthBucket1 = infoArg->widthBucket1;
    u64 bucket1 = infoArg->bucket1;
    u64 senderSize = infoArg->senderSize;
    u64 heightInBytes = infoArg->heightInBytes;
    u64 locationInBytes = infoArg->locationInBytes;
    block randomLocations[bucket1]; // 256个block
    u8 *matrixC[widthBucket1];      // 16
    u8 *transLocations[widthBucket1];
    for (auto i = 0; i < widthBucket1; ++i)
    {
      // 256*1+4，每个元素为u8*，下面语句为创建一个u8*,260个u8
      transLocations[i] = new u8[senderSize * locationInBytes + sizeof(u32)];
      matrixC[i] = new u8[heightInBytes]; // 32
    }
    for (auto wLeft = 0; wLeft < infoArg->processNum; wLeft += widthBucket1, cycTimes++)
    {
      auto wRight = wLeft + widthBucket1 < infoArg->processNum
                        ? wLeft + widthBucket1
                        : infoArg->processNum;
      auto w = wRight - wLeft;
      //////////// Compute random locations (transposed) ////////////////
      AES commonAesFkey;
      commonAesFkey.setKey(infoArg->aesComKeys[cycTimes]);
      for (auto low = 0; low < senderSize; low += bucket1)
      {
        auto up = low + bucket1 < senderSize ? low + bucket1 : senderSize;
        commonAesFkey.ecbEncBlocks(infoArg->sendset + low, up - low, randomLocations);
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
      //////////////// Extend OTs and compute matrix C ///////////////////
      // u8 *recvMatrix;
      u64 offset1 = wLeft * heightInBytes;
      u64 offset2 = 0;
      for (auto i = 0; i < w; ++i)
      {
        PRNG prng(infoArg->recoverMsgWidthOutputPtr[i + wLeft]);
        prng.get(matrixC[i], heightInBytes);
        if (infoArg->choicesWidthInputPtr[i + wLeft])
        {
          for (auto j = 0; j < heightInBytes; ++j)
          {
            matrixC[i][j] ^= (infoArg->recvMatrixADBuffBegin + offset1 + offset2)[j];
          }
        }
        offset2 += heightInBytes;
      }
      ///////////////// Compute hash inputs (transposed) /////////////////////
      for (auto i = 0; i < w; ++i)
      {
        for (auto j = 0; j < senderSize; ++j)
        {
          auto location = (*(u32 *)(transLocations[i] + j * locationInBytes)) & (infoArg->shift);
          infoArg->transHashInputsPtr[i + wLeft][j >> 3] |=
              (u8)((bool)(matrixC[i][location >> 3] & (1 << (location & 7)))) << (j & 7);
        }
      }
    }
    /****cycle end ****/
    //**************释放内存*************//
    for (auto i = 0; i < widthBucket1; ++i)
    {
      delete[] transLocations[i];
      transLocations[i] = nullptr;
      delete[] matrixC[i];
      matrixC[i] = nullptr;
    }
  }

  int PsiSender::recoverMatrixC(const u8_t *recvMatrixADBuff, const u64_t recvMatixADBuffSize)
  {
    auto locationInBytes = (this->logHeight + 7) / 8;    // logHeight==1
    auto widthBucket1 = sizeof(block) / locationInBytes; // 16/1
    u64 shift = (1 << this->logHeight) - 1;              //全1
    printf(">>>recover matrix C,widthBucket1:%ld(16/loc),locationInBytes:%ld\n",
           widthBucket1, locationInBytes);
    ////////////// Initialization //////////////////////
    if (recvMatixADBuffSize != this->matrixWidth * this->heightInBytes)
    {
      return -111;
    }
    // u8 *transHashInputs[this->matrixWidth]; // width==60，矩阵宽度
    cout << "***********************before cycle" << endl;
    /****cycle start***/
    long start1 = start_time();
    int threadNum = this->threadNumOmp;
    u64 sumCount = 0;
    int isExit = 0;
    printf("========参数准备开始======\n");
    RecoverMatrixCInfo infoArgs[threadNum];
    //内存初始化为0
    memset((char *)infoArgs, 0, threadNum * sizeof(RecoverMatrixCInfo));
    //对每个线程进行分配处理的矩阵的列数
    for (;;)
    {
      for (int k = 0; k < threadNum; k++)
      {
        int wRight = sumCount + widthBucket1;
        if (wRight <= this->matrixWidth)
        {
          // 3
          infoArgs[k].processNum += widthBucket1;
          // 4
          infoArgs[k].aesKeyNum++;
        }
        else
        {
          isExit = 1;
          break;
        }
        sumCount += widthBucket1;
      }
      if (isExit)
      {
        break;
      }
    }
    printf(">>>omp 处理,sumCount:=%d\n", sumCount);
    //不要漏掉余数
    if ((this->matrixWidth - sumCount) != 0)
    {
      infoArgs[threadNum - 1].processNum += this->matrixWidth - sumCount;
      infoArgs[threadNum - 1].aesKeyNum += 1;
    }
    for (int i = 0; i < threadNum; i++)
    {
      printf("procesNum[%2d]=:%2d,", i, infoArgs[i].processNum);
    }
    cout << endl;
    for (int i = 0; i < threadNum; i++)
    {
      printf("aesKeyNum[%2d]=:%2d,", i, infoArgs[i].aesKeyNum);
    }
    //分配处理的矩阵的列数结束
    //最重要的一步，生成Fk函数的keys
    vector<block> commonKeys;
    for (auto wLeft = 0; wLeft < this->matrixWidth; wLeft += widthBucket1)
    {
      block comKey;
      this->commonPrng->get((u8 *)&comKey, sizeof(block));
      // commonKeys.push_back(comKey);
      commonKeys.emplace_back(comKey);
    }
    printf("\n>>>commonKeys size:%ld\n", commonKeys.size());
    //最重要的一步，取出 choicesWidthInput
    u64 choiceSize = this->choicesWidthInput.size();
    vector<int> choiceVector;
    for (int i = 0; i < choiceSize; i++)
    {
      // choiceVector.push_back(this->choicesWidthInput[i]);
      choiceVector.emplace_back(this->choicesWidthInput[i]);
    }
    //给参数赋值
    for (int k = 0; k < threadNum; k++)
    {
      infoArgs[k].threadId = k;
      // 1
      infoArgs[k].shift = shift;
      if (k == 0)
      {
        // 2
        infoArgs[k].wLeftBegin = 0;
        // 5
        infoArgs[k].aesComkeysBegin = 0;
      }
      else
      {
        infoArgs[k].wLeftBegin = infoArgs[k - 1].wLeftBegin + infoArgs[k - 1].processNum;
        infoArgs[k].aesComkeysBegin = infoArgs[k - 1].aesComkeysBegin + infoArgs[k - 1].aesKeyNum;
      }
      // 7
      infoArgs[k].senderSize = this->senderSize;
      // 8
      infoArgs[k].heightInBytes = this->heightInBytes;
      // 9
      infoArgs[k].locationInBytes = locationInBytes;
      // 10
      infoArgs[k].bucket1 = this->bucket1;
      // 11
      infoArgs[k].widthBucket1 = widthBucket1;
      // 6
      infoArgs[k].aesComKeys =
          (block *)(commonKeys.data()) + infoArgs[k].aesComkeysBegin;
      // 12
      infoArgs[k].sendset = this->sendSet;
      // 13
      infoArgs[k].recoverMsgWidthOutputPtr =
          this->recoverMsgWidthOutput.data() + infoArgs[k].wLeftBegin;
      // 14
      infoArgs[k].choicesWidthInputPtr =
          choiceVector.data() + infoArgs[k].wLeftBegin; // todo
      // 15
      infoArgs[k].recvMatrixADBuffBegin =
          recvMatrixADBuff + infoArgs[k].wLeftBegin * this->heightInBytes;
      // 16
      infoArgs[k].transHashInputsPtr =
          (vector<u8> *)(this->transHashInputs.data()) + infoArgs[k].wLeftBegin;
    }

    for (int i = 0; i < threadNum; i++)
    {
      printf("aesBegin[%2d]:%2ld,", i, infoArgs[i].aesComkeysBegin);
    }
    printf("\n");
    for (int j = 0; j < threadNum; j++)
    {
      printf("wlfBegin[%2ld]:%2ld,", j, infoArgs[j].wLeftBegin);
    }
    printf("\n========参数准备结束======\n");
    printf("========并行处理恢复矩阵C开始,threadNum(%d)========\n", threadNum);

#pragma omp parallel for num_threads(threadNum)
    for (int i = 0; i < threadNum; i++)
    {
      process_recover_matrix_C(infoArgs + i);
    }
    printf("========并行处理恢复矩阵C结束,threadNum(%d)========\n", threadNum);
    /****cycle end ****/
    //**************释放内存*************//
    cout << "***********************before cycle end" << endl;
    printf(">>>cycle 用时：%ldms\n", get_use_time(start1));
    printf(">>> this->sendSet,end\n");
    return 0;
  }
  //计算本方的hash输出并发送给对方
  int PsiSender::computeHashOutputToReceiverOnce(u8_t **sendBuff, u64_t *sendBuffSize)
  {
    auto upR = this->lowL + this->bucket2ForComputeH2Output < this->senderSize
                   ? this->lowL + this->bucket2ForComputeH2Output
                   : this->senderSize;
    // H2
    RandomOracle H(this->hash2LengthInBytes);
    u8 hashOutput[sizeof(block)];
    for (auto j = this->lowL; j < upR; ++j)
    {
      memset(this->hashInputs[j - this->lowL].data(), 0,
             this->matrixWidthInBytes);
    }
    for (auto i = 0; i < this->matrixWidth; ++i)
    {
      for (auto j = this->lowL; j < upR; ++j)
      {
        this->hashInputs[j - this->lowL][i >> 3] |=
            (u8)((bool)(this->transHashInputs[i][j >> 3] & (1 << (j & 7)))) << (i & 7);
      }
    }
    u64 offset = 0;
    for (auto j = this->lowL; j < upR; ++j, offset += this->hash2LengthInBytes)
    {
      H.Reset();
      H.Update(this->hashInputs[j - this->lowL].data(), this->matrixWidthInBytes);
      H.Final(hashOutput);
      memcpy(this->hashOutputBuff.data() + offset, hashOutput, this->hash2LengthInBytes);
    }
    *sendBuffSize = (upR - this->lowL) * this->hash2LengthInBytes;
    *sendBuff = this->hashOutputBuff.data();
    this->lowL += this->bucket2ForComputeH2Output;
    return 0;
  }
#endif

// PsiReceiver,接收对方发来的hash输出
#ifdef OMP_ONLY
  //匹配不使用线程池
  int PsiReceiver::recvFromSenderAndComputePSIOnce(const u8_t *recvBuff,
                                                   const u64_t recvBufSize,
                                                   vector<u32_t> *psiMsgIndex)
  {
    auto up = this->lowL + this->bucket2ForComputeH2Output < this->senderSize
                  ? this->lowL + this->bucket2ForComputeH2Output
                  : this->senderSize;
    if (recvBufSize != (up - this->lowL) * this->hash2LengthInBytes)
    {
      return -122;
    }
    //
    u64 offset = 0;
    for (auto idx = 0; idx < up - this->lowL; ++idx, offset += this->hash2LengthInBytes)
    {
      u64 mapIdx = *(u64 *)(recvBuff + offset);
      // #pragma omp parallel for num_threads(this->threadNumOmp)
      //这里加omp指令并不能提高性能，反而下降一倍多
      for (int i = 0; i < this->HashMapVector.size(); i++)
      {
        auto found = this->HashMapVector[i].find(mapIdx);
        if (found == this->HashMapVector[i].end())
          continue;
        //可能找到好几个
        for (auto j = 0; j < found->second.size(); ++j)
        {
          if (memcmp(&(found->second[j].first), recvBuff + offset,
                     this->hash2LengthInBytes) == 0)
          {
            // psiMsgIndex->push_back(found->second[j].second);
            psiMsgIndex->emplace_back(found->second[j].second);
            break;
          }
        }
      }
    }
    this->lowL += this->bucket2ForComputeH2Output;
    return 0;
  }
#endif

#ifdef OMP_POOL
  //匹配用线程池
  typedef struct ThreadPoolInfo
  {
    u8_t *recvBuff;
    u64_t recvBufSize;
    u64_t hash2LengthInBytes;
    u64_t lowL;
    u64_t up;
    unordered_map<u64, std::vector<std::pair<block, u32_t>>> *HashMapVectorPtr;
    int hashMapSize;
    u32_t psiResultId;
    vector<u32_t> *psiResult;
  } ThreadPoolInfo;
  // typedef ThreadPoolInfo *ThreadPoolInfoPtr;
  // 任务function
  u32_t process_compute_psi_by_threadpool(ThreadPoolInfo *infoArg)
  {
    u64_t recvBufSize = infoArg->recvBufSize;
    // printf("------------------->>>>psiResultId:%d\n", psiResultId);
    // vector<u32_t> *psiResult = infoArg->psiResult;
    u64 offset = 0;
    for (auto idx = 0; idx < infoArg->up - infoArg->lowL; ++idx,
              offset += infoArg->hash2LengthInBytes)
    {
      u64 mapIdx = *(u64 *)(infoArg->recvBuff + offset);
      //这里加omp指令并不能提高性能，反而下降一倍多
      for (int i = 0; i < infoArg->hashMapSize; i++)
      {
        auto found = infoArg->HashMapVectorPtr[i].find(mapIdx);
        if (found == infoArg->HashMapVectorPtr[i].end())
          continue;
        //可能找到好几个
        for (auto j = 0; j < found->second.size(); ++j)
        {
          if (memcmp(&(found->second[j].first), infoArg->recvBuff + offset,
                     infoArg->hash2LengthInBytes) == 0)
          {
            // psiMsgIndex->push_back(found->second[j].second);
            infoArg->psiResult[0].emplace_back(found->second[j].second);
            // printf("###psiResultId:%d\n", psiResultId);
            break;
          }
        }
      }
    }
    u32_t psiResultId = infoArg->psiResultId;
    //free内存
    free(infoArg->recvBuff);
    infoArg->recvBuff = nullptr;
    free(infoArg);
    infoArg = nullptr;
    return psiResultId;
  }
  int PsiReceiver::recvFromSenderAndComputePSIOnce(const u8_t *recvBuff,
                                                   const u64_t recvBufSize)
  {
    auto up = this->lowL + this->bucket2ForComputeH2Output < this->senderSize
                  ? this->lowL + this->bucket2ForComputeH2Output
                  : this->senderSize;
    if (recvBufSize != (up - this->lowL) * this->hash2LengthInBytes)
    {
      return -122;
    }
    ThreadPoolInfo *infoArg = (ThreadPoolInfo *)malloc(sizeof(ThreadPoolInfo));
    memset(infoArg, 0, sizeof(ThreadPoolInfo));
    //赋值
    infoArg->recvBuff = (u8_t *)malloc(recvBufSize);
    if (infoArg->recvBuff == nullptr)
    {
      return -123;
    }
    memcpy(infoArg->recvBuff, recvBuff, recvBufSize);
    infoArg->recvBufSize = recvBufSize;
    infoArg->hash2LengthInBytes = this->hash2LengthInBytes;
    infoArg->lowL = this->lowL;
    infoArg->up = up;
    infoArg->HashMapVectorPtr = this->HashMapVector.data();
    infoArg->hashMapSize = this->HashMapVector.size();
    infoArg->psiResultId = this->indexId;
    infoArg->psiResult = (vector<u32_t> *)(this->psiResults.data()) + this->indexId;
    //赋值end
    this->psiResultsIndex.emplace_back(this->psiComputePool->enqueue(
        process_compute_psi_by_threadpool, infoArg));
    this->lowL += this->bucket2ForComputeH2Output;
    this->indexId++;
    return 0;
  }
  //最后获取psi结果
  int PsiReceiver::getPsiResultsForAll(vector<u32_t> *psiResultsOutput)
  {
    u64_t psiResultsSize = this->psiResultsIndex.size();
    printf("===>>psiResultsIndex size:%ld\n", psiResultsSize);
    for (u64_t i = 0; i < psiResultsSize; i++)
    {
      u32_t resultIndex = (this->psiResultsIndex)[i].get();
      // printf("===>>ind:%ld,resultIndex:%d\n", i, resultIndex);
      for (int j = 0; j < this->psiResults[resultIndex].size(); j++)
      {
        (*psiResultsOutput).emplace_back(this->psiResults[resultIndex][j]);
      }
    }
    return 0;
  }
#endif
//集成socket的oprf-psi接口，单独提供出来
#ifdef OMP_POOL
  int oprf_psi_receiver_process(u64_t receiverSize, u64_t senderSize, const char *address,
                                int port, u8_t *commonSeed, u64_t matrixWidth,
                                u64_t logHeight, u64_t hash2LengthInBytes,
                                u64_t bucket2ForComputeH2Output, int omp_num,
                                vector<vector<u8_t>> receiver_set,
                                vector<u32_t> *psiResultsOutput)
  {
    //生成recvSet
    // long start0 = start_time();
    PsiReceiver psiRecv;
    //初始化一个socket连接
    void *server = initChannel(SERVER, address, port);
    if (server == nullptr)
    {
      return -11;
    }
    long start1 = start_time();
    //初始化psiRecv
    int fg = psiRecv.init(commonSeed, receiverSize, senderSize, matrixWidth, logHeight,
                          omp_num, hash2LengthInBytes, bucket2ForComputeH2Output);
    if (fg)
    {
      return fg;
    }
    // assert(fg == 0);
    //接收对方的公共参数,并生成 pk0sBuf
    char *pubParamBuf = nullptr;
    char *pk0sBuf = nullptr;
    u64 pk0sBufSize = 0;
    int n = recv_data(server, &pubParamBuf);
    if (n <= 0)
    {
      return -12;
    }

    fg = psiRecv.genPK0FromNpot((u8 *)pubParamBuf, n, (u8 **)&pk0sBuf, &pk0sBufSize);
    if (fg)
    {
      return fg;
    }

    n = send_data(server, pk0sBuf, pk0sBufSize);
    //接收 uBuffInput,并生成matrixAD,并发送给对方
    char *uBuffInput = nullptr;
    char *matrixADBuff = nullptr;
    u64 matrixADBuffSize = 0;
    n = recv_data(server, &uBuffInput);
    if (n <= 0)
    {
      return -12;
    }
    // assert(n > 0);
    long start2 = start_time();
    fg = psiRecv.getSendMatrixADBuff((u8 *)uBuffInput, n, receiver_set,
                                     (u8 **)&matrixADBuff, &matrixADBuffSize);
    n = send_data(server, matrixADBuff, matrixADBuffSize);
    printf("===>>recv:生成MatrixAD所需时间:%ld ms\n", get_use_time(start2));
    printf("===>>recv:OT所需时间:%ld ms\n", get_use_time(start1));
    //本方生成hashMap
    long start3 = start_time();
    fg = psiRecv.genenateAllHashesMap();
    if (fg)
    {
      return fg;
    }
    printf("===>>recv:生成hashMap表所需时间:%ld ms\n", get_use_time(start3));
    //循环接收对方发来的hashOutput
    long start4 = start_time();
    // char *hashOutput = nullptr;
    // vector<oc::u32> psiMsgIndexs;
    int totalCyc = senderSize / bucket2ForComputeH2Output;
    printf("===>>sendersize:%ld,bucket2ForComputeH2Output:%d,totalCyc:%d\n",
           senderSize, bucket2ForComputeH2Output, totalCyc);
    int count = 0, countTmp = 10;
    long int start00;
    for (; psiRecv.isRecvEnd() == 0;)
    {
      char *hashOutput = nullptr;
#ifdef SOCKET_TEST
      if (count < countTmp)
      {
        start00 = start_time();
      }
#endif
      n = recv_data(server, &hashOutput);
      if (n <= 0)
      {
        printf("===>>(recv_data error)count:%d\n", count);
      }
#ifdef SOCKET_TEST
      if (count < countTmp)
      {
        printf("count:%d,recv:接收一次H2Ouput所需时间:%ldms\n", count, get_use_time(start00));
      }
#endif
      fg = psiRecv.recvFromSenderAndComputePSIOnce((u8 *)hashOutput, n);
      if (fg)
      {
        return fg;
      }
#ifdef SOCKET_TEST
      if (count < countTmp)
      {
        printf("count:%d,pool recv:接收一次H2Ouput并匹配所需时间:%ldms\n", count, get_use_time(start00));
      }
#endif
      count++;
    }
    //获取psi结果
    fg = psiRecv.getPsiResultsForAll(psiResultsOutput);
    printf("===>>getPsiResultsForAll end fg:%d\n", fg);
    if (fg)
    {
      return fg;
    }
    printf("===>>count:%d\n", count);
    printf("===>>recv: 匹配find用时：%ldms,totalCyc:%d,count:%d\n",
           get_use_time(start4), totalCyc, count);
    printf("===>>recv:总用时：%ldms\n", get_use_time(start1));

    //释放mem
    releaseChannel(server);
    printf("===>>oprf psi receiver end...\n");
    return 0;
  }
  //psi sender方，接口单独提供出来
  int oprf_psi_sender_process(u64_t receiverSize, u64_t senderSize, const char *address,
                              int port, u8_t *commonSeed, u64_t matrixWidth,
                              u64_t logHeight, u64_t hash2LengthInBytes,
                              u64_t bucket2ForComputeH2Output, int omp_num,
                              vector<vector<u8_t>> sender_set)
  {

    //初始化一个socket连接
    void *client = initChannel(CLIENT, address, port);
    if (client == nullptr)
    {
      return -11;
    }
    // assert(client);
    // psi 发送方
    long start2 = start_time();
    PsiSender psiSender;
    int fg = psiSender.init(commonSeed, senderSize, matrixWidth, logHeight, omp_num,
                            hash2LengthInBytes, bucket2ForComputeH2Output);
    if (fg)
    {
      return fg;
    }
    // assert(fg == 0);
    //[1]生成基本ot协议的公共参数，并发送给对方
    u8 *pubParamBuf = nullptr;
    u64 pubParamBufSize = 0;
    fg = psiSender.genPublicParamFromNpot(&pubParamBuf, &pubParamBufSize);
    if (fg)
    {
      return fg;
    }
    // assert(fg == 0);
    int n = send_data(client, (char *)pubParamBuf, pubParamBufSize);
    // assert((u64)n == pubParamBufSize);
    //[2]接收对方发过来的pk0sBuff
    char *pk0sBuf = nullptr;
    n = recv_data(client, &pk0sBuf);
    if (n <= 0)
    {
      return -12;
    }
    // assert(n > 0);
    u8 *uBuffOutput = nullptr;
    u64 uBuffOutputSize = 0;
    //[3]输入pk0s，生成uBuffOutput,并发送给对方
    long start_TxorR = start_time();
    fg = psiSender.genMatrixTxorRBuff((u8 *)pk0sBuf, n, &uBuffOutput, &uBuffOutputSize);
    if (fg)
    {
      return fg;
    }
    printf("===>>发送方生成TxorR矩阵用时:%ld ms\n", get_use_time(start_TxorR));
    // assert(fg == 0);
    n = send_data(client, (char *)uBuffOutput, uBuffOutputSize);
    // assert((u64)n == uBuffOutputSize);
    //计算H1 start（避免等待）
    printf("===>>发送方避免等待，开始计算hash1...\n");
    fg = psiSender.computeAllHashOutputByH1(sender_set);
    if (fg)
    {
      return fg;
    }
    // assert(fg == 0);
    //计算H1 end
    //[4]接收MatrixAxorD,生成矩阵C
    char *matrixAxorD = nullptr;
    long start_recv_AD = start_time();
    n = recv_data(client, &matrixAxorD);
    if (n <= 0)
    {
      return -12;
    }
    // assert(n > 0);
    printf("===>>接收收到matrixAxorD用时:%ld ms\n", get_use_time(start_recv_AD));
    //恢复矩阵C
    long start3 = start_time();
    fg = psiSender.recoverMatrixC((u8 *)matrixAxorD, n);
    if (fg)
    {
      return fg;
    }
    // assert(fg == 0);
    printf("===>>recoverMatrixC所需时间:%ldms\n", get_use_time(start3));
    printf("===>>OT所需时间:%ldms\n", get_use_time(start2));
    //循环发送hash输出给对方
    u64 hashOutputOnceBuffSize = 0;
    u8 *hashOutputOnceBuff = nullptr;
    int totalCyc = senderSize / bucket2ForComputeH2Output;
    int count = 0, countTmp = 10;
    int ret = psiSender.isSendEnd();
    printf("===>>isSendEnd:%d\n", ret);
    long start4 = start_time();
    long int start00;
    for (; psiSender.isSendEnd() == 0;)
    {
#ifdef SOCKET_TEST
      if (count < countTmp)
      {
        start00 = start_time();
      }
#endif
      fg = psiSender.computeHashOutputToReceiverOnce(&hashOutputOnceBuff,
                                                     &hashOutputOnceBuffSize);
      if (fg)
      {
        return fg;
      }
#ifdef SOCKET_TEST
      if (count < countTmp)
      {
        printf("count:%d,send:计算一次H2Output所需时间:%ldms\n", count, get_use_time(start00));
      }
#endif
      n = send_data(client, (char *)hashOutputOnceBuff, hashOutputOnceBuffSize);
      assert((u64)n == hashOutputOnceBuffSize);
#if SOCKET_TEST
      if (count < countTmp)
      {
        printf("count:%d,send:计算一次H2Output并发送所需时间:%ldms\n", count, get_use_time(start00));
      }
#endif
      count++;
    }
    printf("===>>count:%d\n", count);
    printf("sender: 匹配find用时：%ldms,totalCyc:%d\n", get_use_time(start4), totalCyc);
    printf("sender: 总用时:%ldms\n", get_use_time(start2));
    //释放mem, sender
    releaseChannel(client);
    printf("===>>oprf psi sender end...\n");
    return 0;
  }
#endif
} // namespace osuCrypto
