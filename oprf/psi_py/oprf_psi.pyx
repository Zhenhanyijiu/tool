from libcpp.vector cimport vector
from libc cimport string
from libc.stdio cimport printf
cdef extern from "psi.h" namespace "osuCrypto":
    ctypedef unsigned long int u64_t
    ctypedef unsigned char u8_t
    ctypedef unsigned int u32_t
    cdef cppclass PsiSender:
        PsiSender()except+
        # ~PsiSender()
        int init(u8_t *commonSeed, u64_t senderSize, u64_t matrixWidth, u64_t logHeight,
                 u64_t hash2LengthInBytes = 10, u64_t bucket2ForComputeH2Output = 256)
        int genPublicParamFromNpot(u8_t **pubParamBuf, u64_t *pubParamBufByteSize)
        int genMatrixTxorRBuff(u8_t *pk0Buf, const u64_t pk0BufSize,
                               u8_t **uBuffOutputTxorR, u64_t *uBuffOutputSize)
        int recoverMatrixC(const u8_t *recvMatrixADBuff, const u64_t recvMatixADBuffSize,
                           const vector[vector[u8_t]] senderSet)
        int isSendEnd()
        int computeHashOutputToReceiverOnce(u8_t **sendBuff, u64_t *sendBuffSize)

    cdef cppclass PsiReceiver:
        PsiReceiver()except+
        int init(u8_t *commonSeed, u64_t receiverSize, u64_t senderSize,
                 u64_t matrixWidth, u64_t logHeight, u64_t hash2LengthInBytes = 10,
                 u64_t bucket2ForComputeH2Output = 256)
        int genPK0FromNpot(u8_t *pubParamBuf, const u64_t pubParamBufByteSize,
                           u8_t **pk0Buf, u64_t *pk0BufSize)
        int getSendMatrixADBuff(const u8_t *uBuffInputTxorR, const u64_t uBuffInputSize,
                                const vector[vector[u8_t]] receiverSet,
                                u8_t **sendMatrixADBuff, u64_t *sendMatixADBuffSize)
        int genenateAllHashesMap()
        int isRecvEnd()
        int recvFromSenderAndComputePSIOnce(const u8_t *recvBuff, const u64_t recvBufSize,
                                            vector[u32_t] *psiMsgIndex)

cdef class OprfPsiSender:
    cdef PsiSender psi_sender        
    def __init__(self):
        # self.psi_sender.init()
        pass
    