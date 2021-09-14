#include "naorpinkas.h"

#include <cryptoTools/Common/Log.h>
#include <cryptoTools/Common/Timer.h>
// #include <cryptoTools/Common/BitVector.h>
#include <cryptoTools/Crypto/RandomOracle.h>

#define PARALLEL

#ifdef NAOR_PINKAS

// #include <memory>
void print_error(int n)
{
    std::cout << "========error!!! error:" << n << endl;
}
namespace osuCrypto
{

    //static const  u64 minMsgPerThread(16);
    /***********************
    naor-pinkas ot protocal for Sender
    ************************/
    NaorPinkasSender::NaorPinkasSender()
    {
    }
    NaorPinkasSender::~NaorPinkasSender()
    {
        delete this->alphaPtr;
    }
    int NaorPinkasSender::init(PRNG &rng, const u32 otMsgPairSize,
                               const u32 otPerMsgBitSize)
    {
        this->otMsgPairSize = otMsgPairSize;
        this->otPerMsgBitSize = otPerMsgBitSize;
        // cout << "====otMsgPairSize  :" << this->otMsgPairSize << endl;
        // cout << "====otPerMsgBitSize:" << this->otPerMsgBitSize << endl;
        this->numThreads = 1, this->nSndVals = 2;
        block seed = rng.get<block>();
        this->prng.SetSeed(seed);
        //alpha也可以定义为长度为1的vector，此时就不需要指针了
        this->alphaPtr = new Number(this->curve, this->prng);
        this->R = this->prng.get<block>();
        this->pC.reserve(nSndVals);
        return 0;
    }

    int NaorPinkasSender::genPublicParam(u8 **pubParamBuf, u64 *pubParamBufByteSize)
    {
        const Point g = this->curve.getGenerator();
        u64 fieldElementSize = g.sizeBytes();
        u64 byteSize = fieldElementSize * this->nSndVals;
        pubPCParamBuf.resize(byteSize);
        this->pC.emplace_back(this->curve);
        this->pC[0] = g * (*this->alphaPtr); // A=alpha*G
        this->pC[0].toBytes(pubPCParamBuf.data());
        Number tmp(this->curve);
        for (u64 u = 1; u < this->nSndVals; u++)
        {
            this->pC.emplace_back(this->curve);
            tmp.randomize(this->prng);
            this->pC[u] = g * tmp; // C=tmp*G
            this->pC[u].toBytes(pubPCParamBuf.data() + u * fieldElementSize);
        }
        *pubParamBuf = pubPCParamBuf.data();
        if (*pubParamBuf == nullptr)
        {
            print_error(-1);
            return -1;
        }
        *pubParamBufByteSize = byteSize;
        // vector<u8> sendBuff(nSndVals * fieldElementSize);
        // Number alpha2(curve, *alphaPtr);
        // std::cout << "alphaPtr:" << *alphaPtr << endl;
        // std::cout << "alpha2  :" << alpha2 << endl;
        return 0;
    }
    int NaorPinkasSender::getEncKey(u8 *pk0Buf, const u64 pk0BufSize,
                                    vector<array<block, 2>> &encKeys)
    {
        const Point g = this->curve.getGenerator();
        u64 fieldElementSize = g.sizeBytes();
        // u64 encKeySize = encKeys.size();
        u64 encKeySize = this->otMsgPairSize;
        if (pk0Buf == nullptr || pk0BufSize != fieldElementSize * encKeySize)
        {
            print_error(-2);
            return -2;
        }
        encKeys.resize(encKeySize);
        // u64 pk0Num = pk0BufSize / fieldElementSize;
        Point pPK0(curve), PK0a(curve), fetmp(curve);
        // Number alpha2(curve, alpha);
        std::vector<u8> hashInBuff(fieldElementSize); // 33字节
        //todo:这里的randomoracle的输出长度是否固定？？？
        RandomOracle sha(sizeof(block)); // 16字节
        for (int i = 0; i < encKeySize; i++)
        {
            //对于第i对消息，获取pPK0==k*G
            pPK0.fromBytes(pk0Buf + i * fieldElementSize);
            //计算a*pPK0
            PK0a = pPK0 * (*(this->alphaPtr));
            //将点转化成压缩形式
            PK0a.toBytes(hashInBuff.data()); // PK0a=(x,y),取x
            //计算hash，text=i||hashInBuff||R做hash运算,并将结果存到messages[i][0]中
            sha.Reset();
            sha.Update((u8 *)&i, sizeof(i));
            sha.Update(hashInBuff.data(), hashInBuff.size());
            // sha.Update(R);
            //结果存到messages[i][0]中
            sha.Final(encKeys[i][0]);
            //处理另一个Ca/PK0a
            fetmp = this->pC[1] * (*(this->alphaPtr)) - PK0a;
            fetmp.toBytes(hashInBuff.data());
            sha.Reset();
            sha.Update((u8 *)&i, sizeof(i));
            sha.Update(hashInBuff.data(), hashInBuff.size());
            // sha.Update(R);
            //结果存到 encKeys[i][1]
            sha.Final(encKeys[i][1]);
        }
        ////
        return 0;
    }
    int NaorPinkasSender::genOTCipher(const vector<array<block, 2>> &encKey,
                                      const vector<array<block, 2>> &otMessages,
                                      vector<array<block, 2>> &otMsgCiphers)
    {
        u64 encKeySize = encKey.size();
        u64 otMessagesSize = otMessages.size();
        // u64 otMsgCiphersSize = otMsgCiphers.size();
        if (otMsgPairSize != encKeySize || otMsgPairSize != otMessagesSize)
        {
            print_error(-3);
            return -3;
        }
        otMsgCiphers.resize(otMsgPairSize);
        for (int i = 0; i < otMessagesSize; i++)
        {
            otMsgCiphers[i][0] = encKey[i][0] ^ otMessages[i][0];
            otMsgCiphers[i][1] = encKey[i][1] ^ otMessages[i][1];
        }
        ////
        return 0;
    }
    /*
    naor-pinkas ot protocal for Receiver
    */
    NaorPinkasReceiver::NaorPinkasReceiver() {}
    NaorPinkasReceiver::~NaorPinkasReceiver() {}
    int NaorPinkasReceiver::init(PRNG &rng, const u32 otMsgPairSize,
                                 const u32 otPerMsgBitSize)
    {
        this->otMsgPairSize = otMsgPairSize;
        this->otPerMsgBitSize = otPerMsgBitSize;
        this->numThreads = 1;
        this->nSndVals = 2;
        block seed = rng.get<block>();
        this->prng.SetSeed(seed);
        this->pC.reserve(this->nSndVals);
        return 0;
    }
    int NaorPinkasReceiver::genPK0(u8 *pubParamBuf, const u64 pubParamBufByteSize,
                                   const BitVector &choices, u8 **pk0Buf, u64 *pk0BufSize)
    {
        u64 rNum = choices.size();
        cout << "rNum:" << rNum << endl;
        cout << "otMsgPairSize:" << this->otMsgPairSize << endl;
        if (pubParamBuf == nullptr || pk0Buf == nullptr || rNum != otMsgPairSize)
        {
            print_error(-4);
            return -4;
        }
        const Point g = this->curve.getGenerator();
        u64 fieldElementSize = g.sizeBytes();
        //恢复对方发来的A,C两个公钥点
        for (auto u = 0; u < this->nSndVals; u++)
        {
            pC.emplace_back(curve);
            pC[u].fromBytes(pubParamBuf + u * fieldElementSize);
        }
        this->sks.reserve(otMsgPairSize);
        vector<Point> pks_sk;
        pks_sk.reserve(otMsgPairSize);
        *pk0BufSize = fieldElementSize * otMsgPairSize;
        this->pk0sBuf.resize(*pk0BufSize);
        *pk0Buf = this->pk0sBuf.data();
        Brick bg(g);      //基点G
        Point pk0(curve); //Ar
        u64 offset = 0;
        for (int i = 0; i < otMsgPairSize; i++)
        {
            this->sks.emplace_back(curve);
            // k1,k2,...,kk
            this->sks[i].randomize(prng);
            pks_sk.emplace_back(curve);    //每次初始化一个ECCPoint
            pks_sk[i] = bg * this->sks[i]; //生成对应的公钥g^k
            //根据选择0 or 1，计算A0
            u8 r = choices[i];
            if (r != 0)
            {
                pk0 = this->pC[1] - pks_sk[i];
            }
            else
            {
                pk0 = pks_sk[i];
            }
            pk0.toBytes(pk0sBuf.data() + offset);
            offset += fieldElementSize;
        }
        return 0;
    }
    int NaorPinkasReceiver::getDecKey(vector<block> &decKey)
    {
        // if (otMsgPairSize != decKey.size())
        // {
        //     print_error(-5);
        //     return -5;
        // }
        decKey.resize(this->otMsgPairSize);
        // resuse this space, not the data of PK0...
        Point PK0(curve);
        Point &gka = PK0;
        u64 fieldElementSize = PK0.sizeBytes();
        cout << "fieldElementSize:" << fieldElementSize << endl;
        RandomOracle sha(sizeof(block)); //otPerMsgBitSize
        vector<u8> buff(fieldElementSize);
        Brick ga(pC[0]); // bc==A==g^a

        for (int i = 0; i < this->otMsgPairSize; i++)
        {
            // now compute g ^(a * k) = (g^a)^k=A^k
            gka = ga * this->sks[i]; //计算恢复消息的
            gka.toBytes(buff.data());
            //计算hash，text=i||gka||R作为hash输入
            sha.Reset();
            sha.Update((u8 *)&i, sizeof(i));
            sha.Update(buff.data(), buff.size());
            // sha.Update(R);
            //将密钥存到messges中
            sha.Final(decKey[i]);
        }
        return 0;
    }
    int NaorPinkasReceiver::genOTRecover(const vector<block> &decKey, const BitVector &choices,
                                         const vector<array<block, 2>> &otMsgCiphers,
                                         vector<block> &otMsgRecover)
    {
        if (decKey.size() != otMsgPairSize || choices.size() != otMsgPairSize ||
            otMsgCiphers.size() != otMsgPairSize)
        {
            print_error(-6);
            return -6;
        }
        otMsgRecover.resize(otMsgPairSize);
        for (int i = 0; i < otMsgPairSize; i++)
        {
            u8 r = choices[i];
            otMsgRecover[i] = otMsgCiphers[i][r] ^ decKey[i];
        }
        return 0;
    }
}

#endif